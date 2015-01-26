#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <signal.h>
#include <chrono>
#include <thread>

#include <cstring>      // Needed for memset
#include <sys/socket.h> // Needed for the socket functions
#include <netdb.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#define CAPTURE_PATH "/home/pi/ram/capture.jpg"
#define SLEEP_MILLI 250
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
bool send(string msg);

int raspiStillPID = -1; //The PID of the child process exectuting raspistill.

int status;
struct addrinfo host_info; // The struct that getaddrinfo() fills up with data.
struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.
string server_IP, socket_port;
int socket_d; // Socket descriptor

int main(int argc, char** argv)
{

    //The IP address is the first argument passed to the program.
    if (argc < 2)
        server_IP = argv[1];
    else
    {
        cout << "No address IP passed, setting default value." << endl;
        server_IP = "10.42.0.1";
    }

    //The port is the second argument passed.
    if (argc < 3)
        socket_port = argv[2];
    else
    {
        socket_port = "3000";
        cout << "No socket port passed, setting default value." << endl;
    }


    cout << "Params :\n"
            "Capture path: " << CAPTURE_PATH << endl <<
            "Sleep time between captures (ms):" << SLEEP_MILLI << endl <<
            "Server IP: " << server_IP << endl <<
            "Socket port used: " << socket_port << endl;
    
    if(!initNetwork())
    {
        cerr << "Error occured while initializing network configuration." << endl
                << "Exiting..." << endl;
        exit(1);
    }
    
    if(!createSocket())
    {
        cerr << "Error occured while creating the socket.\nExiting..." << endl;
        exit(1);
    }
    
    if(!connect())
    {
        exit(1);
    }
    
    send("Je suis Antho le petit robot !");


    int iLowH = 0;
    int iHighH = 179;

    int iLowS = 0;
    int iHighS = 255;

    int iLowV = 0;
    int iHighV = 255;

    /*
    namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
    //Create trackbars in "Control" window
    cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
    cvCreateTrackbar("HighH", "Control", &iHighH, 179);

    cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", "Control", &iHighS, 255);

    cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
    cvCreateTrackbar("HighV", "Control", &iHighV, 255);
     */

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
            imgOriginal = imread(CAPTURE_PATH, CV_LOAD_IMAGE_COLOR);
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

        std::string str = to_string(capturedFrames) + ".jpg";

        cout << "Trying to copy " << CAPTURE_PATH << " to " << str << endl;
        std::string cmd = "cp /home/pi/ram/capture.jpg /home/pi/ram/" + str;
        system(cmd.c_str());



        /*
        vector<KeyPoint> points;
        //FASTX(imgThresholded, points, 200, false, FastFeatureDetector::TYPE_9_16);

        SimpleBlobDetector::Params par;
        par.filterByCircularity = false;
        par.filterByColor = true;
        par.blobColor = 0;
        //par.thresholdStep = 200;
        //par.minThreshold = 200;
        //par.maxThreshold = 255;
        SimpleBlobDetector blobD(par);
        //blobD.create("SimpleBlobDetector");
        blobD.detect(imgOriginal, points);
        for (int i = 0; i < points.size(); i++) {
            KeyPoint p = points.at(i);
            printf("x=%f, y=%f, size=%d\n", p.pt.x, p.pt.y, p.size);
            break;
        }

        cv::Mat blobImg;
        cv::drawKeypoints(imgOriginal, points, blobImg);
        //cv::imshow("Blobs", blobImg);

         */
        Mat imgHSV;
        Mat gray;
        cvtColor(imgOriginal, gray, CV_RGB2GRAY);


        //imshow("N&B", gray);
        cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

        Mat imgThresholded;

        inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image



        //morphological opening (remove small objects from the foreground)
        erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

        //morphological closing (fill small holes in the foreground)
        dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

        imshow("Thresholded Image", imgThresholded); //show the thresholded image
        imshow("Original", imgOriginal); //show the original image

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MILLI));
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
        if (execl("/usr/bin/raspistill", "raspistill", "-t", "0", "-s", "-o", CAPTURE_PATH, NULL) < 0)
        {
            cerr << "Error executing raspistill. Terminating child process...\n";
            exit(1);
        }
    }
    sleep(1);
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
    if(status == -1)
    {
        cout << "Not connected to server, cannot send message." << endl;
        return false;
    }
    
    int res = send(socket_d, msg.c_str(), msg.size(), 0);
    
    if(res != msg.size())
    {
        cerr << "Error occured when sending message to server." << endl;
        return false;
    }
    
    cout << "Message sucessfully sent to server." << endl;
    return true;
}

