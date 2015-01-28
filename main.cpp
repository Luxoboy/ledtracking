#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <signal.h>
#include <chrono>
#include <thread>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <cstring>      // Needed for memset
#include <sys/socket.h> // Needed for the socket functions
#include <netdb.h>
#include <string>

#define CAPTURE_PATH "/home/pi/ram/capture.jpg"
#define SLEEP_MILLI 250

#define CAPTURE_WIDTH_DEFAULT 800
#define CAPTURE_HEIGHT_DEFAULT 800

using namespace cv;
using namespace std;

/**
 * @brief Fork process and execute raspistill.
 * This method forks the process. If successfully forked, the child process
 * will execute raspistill.
 */
void forkRaspistill();

/**
 * @brief Capture new picture.
 * @return True if the signal was successfully sent.
 * This method simply sends a SIGUSR1 signal to the raspitill process, triggering
 * the capture of a new frame.
 */
bool capture();

/**
 * @brief Init the network configurations.
 * @return False if an error occured.
 * Set the structures.
 */
bool initNetwork();

/**
 * @brief Creates the socket to the server.
 * @return True if successfully created.
 */
bool createSocket();

/**
 * @brief Connects to the server.
 * @return True is successfully connected.
 */
bool connect();

/**
 * @brief Send a message to the server.
 * @param msg The message to send to the server.
 * @return True if successfully sent.
 */
bool send(std::string msg);

/**
 * @brief Processes the image and extracts the coordinates.
 * @param img
 */
void processImage(Mat &img);

int CAPTURE_WIDTH = CAPTURE_WIDTH_DEFAULT,
        CAPTURE_HEIGHT = CAPTURE_HEIGHT_DEFAULT;

int raspiStillPID = -1; //The PID of the child process exectuting raspistill.

int status;
struct addrinfo host_info; // The struct that getaddrinfo() fills up with data.
struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.
std::string server_IP, socket_port;
int socket_d; // Socket descriptor

int main(int argc, char** argv)
{

    //The IP address is the first argument passed to the program.
    if (argc > 1)
        server_IP = argv[1];
    else
    {
        cout << "No address IP passed, setting default value." << endl;
        server_IP = "10.42.0.1";
    }
    

    //The port is the second argument passed.
    if (argc > 2)
        socket_port = argv[2];
    else
    {
        socket_port = "3000";
        cout << "No socket port passed, setting default value." << endl;
    }
    
    //The captured image width is the third argument passed.
    if(argc > 3)
    {
        CAPTURE_WIDTH = atoi(argv[3]);
    }
    
    //The captured image height is the fourth argument passed.
    if(argc > 4)
    {
        CAPTURE_HEIGHT = atoi(argv[4]);
    }


    cout << "Params :\n"
            "Capture path: " << CAPTURE_PATH << endl <<
            "Captured image: width=" << CAPTURE_WIDTH << "px, height=" <<
            CAPTURE_HEIGHT << endl <<
            "Sleep time between captures (ms):" << SLEEP_MILLI << endl <<
            "Server IP: " << server_IP << endl <<
            "Socket port used: " << socket_port << endl;

    if (!initNetwork())
    {
        cerr << "Error occured while initializing network configuration." << endl
                << "Exiting..." << endl;
        exit(1);
    }

    if (!createSocket())
    {
        cerr << "Error occured while creating the socket.\nExiting..." << endl;
        exit(1);
    }

    if (!connect())
    {
        exit(1);
    }

    send("Je suis Antho le petit robot !");


    forkRaspistill();
    int capturedFrames = 0;
    while (true)
    {

        if (!capture())
        {
            continue;
        }

        capturedFrames++;

        //Waiting for the image to be saved.
        std::this_thread::sleep_for(std::chrono::milliseconds(250));


        cout << "Trying to open " << CAPTURE_PATH << ", frame number " << capturedFrames << "\n";
        Mat imgOriginal;

        int failures = 0;
        do
        {
            imgOriginal = imread(CAPTURE_PATH);
            if (imgOriginal.data == NULL)
            {
                cerr << "Error while reading " << CAPTURE_PATH << ".\n" << endl;
                failures++;
                if (failures > 1000)
                {
                    cout << "More than 10 failures. Exiting...\n";
                    exit(1);
                }
                cout << "Trying again.\n";
            } else
            {
                //Image opend successfully
                cout << "Frame number " << capturedFrames << " opened successfully.\n";
                break;
            }

        } while (true);
        
        processImage(imgOriginal);

        /*

        std::string str = to_string(capturedFrames) + ".jpg";

        cout << "Trying to copy " << CAPTURE_PATH << " to " << str << endl;
        std::string cmd = "cp /home/pi/ram/capture.jpg /home/pi/ram/" + str;
        system(cmd.c_str());
         */

        //std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MILLI));
    }

    return 0;

}

void forkRaspistill()
{
    cout << "Trying to fork.\n";
    raspiStillPID = fork();
    if (raspiStillPID < 0)
    {
        cerr << "Error forking.\n";
        exit(1);
    }
    if (raspiStillPID == 0)
    {
        cout << "Fork successful. Executing raspistill.\n";
        if (execl("/usr/bin/raspistill", "raspistill", "-w", to_string(CAPTURE_WIDTH).c_str(), 
                "-h", to_string(CAPTURE_HEIGHT).c_str(), "-t", "0", "-s", "-o", 
                CAPTURE_PATH, "-q", "100", NULL) < 0)
        {
            cerr << "Error executing raspistill. Terminating child process...\n";
            exit(1);
        }
    }
    sleep(1);
}

bool capture()
{
    bool ret = true;
    cout << "Sending SIGUSR1 signal to child process.\n";
    if (kill(raspiStillPID, SIGUSR1) < 0)
    {
        cerr << "Error while sending signal.\n";
        ret = false;
    }
    return ret;
}

bool initNetwork()
{
    host_info.ai_family = AF_INET; // IP version not specified. Can be both.
    host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.

    memset(&host_info, 0, sizeof host_info);

    status = getaddrinfo(server_IP.c_str(), socket_port.c_str(), &host_info, &host_info_list);

    if (status != 0)
    {
        cout << "Error setting the network structures with getadrinfo." << endl;
        return false;
    }

    return true;
}

bool createSocket()
{
    cout << "Trying to create a socket..." << endl;
    socket_d = socket(host_info_list->ai_family, host_info_list->ai_socktype,
            host_info_list->ai_protocol);

    if (socket_d == -1)
    {
        cout << "Failed to create the socket." << endl;
        return false;
    }

    cout << "Socket created successfully." << endl;
    return true;
}

bool connect()
{
    cout << "Trying to connect to server." << endl;
    status = connect(socket_d, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1)
    {
        cout << "Failed to connect to server." << endl;
        return false;
    }

    cout << "Successfully connected to server." << endl;
    return true;
}

bool send(string msg)
{
    if (status == -1)
    {
        cout << "Not connected to server, cannot send message." << endl;
        return false;
    }

    int res = send(socket_d, msg.c_str(), msg.size(), 0);

    if (res != msg.size())
    {
        cerr << "Error occured when sending message to server." << endl;
        return false;
    }

    cout << "Message sucessfully sent to server." << endl;
    return true;
}

void processImage(Mat &imgOriginal)
{
    Mat imgHSV;

    cvtColor(imgOriginal, imgHSV, COLOR_RGB2HSV); //Convert the captured frame from BGR to HSV
    
    Mat imgThresholded;

    inRange(imgHSV, Scalar(0, 0, 255), Scalar(179, 255, 255), imgThresholded); //Threshold the image
    imwrite("/home/pi/ram/thresh.jpg", imgThresholded);
    
    vector<vector <Point> > contours;
    
    findContours(imgThresholded, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    cout << "Contours foud: " << contours.size() << endl;

    for (vector<Point> vec : contours)
    {

        Moments omoments = moments(vec);

        double dM01 = omoments.m01;
        double dM10 = omoments.m10;
        double dArea = omoments.m00;
        cout << "dArea" << dArea << endl;
        int posX, posY;
        // if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero 
        if (dArea != 0)
        {
            //calculate the position of the ball
            posX = dM10 / dArea;
            posY = dM01 / dArea;
        }
        else
        {
            posX = vec.at(0).x;
            posY = vec.at(0).y;
        }

            cout << "x: " << posX << ", y: " << posY << endl;
    }
}