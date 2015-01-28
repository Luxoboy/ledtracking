#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <signal.h>
#include <chrono>
#include <thread>
#include <pthread.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "network.h"

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
 * @brief Processes the image and extracts the coordinates.
 * @param img
 */
void processImage(Mat &img);

/**
 * Sets variables from arguments passed.
 * @param argc
 * @param argv
 */
void argSetting(int argc, char* argv[]);

int CAPTURE_WIDTH = CAPTURE_WIDTH_DEFAULT,
        CAPTURE_HEIGHT = CAPTURE_HEIGHT_DEFAULT;

int raspiStillPID = -1; //The PID of the child process exectuting raspistill.

int main(int argc, char** argv)
{
    argSetting(argc, argv);
    
    mutex_buf = PTHREAD_MUTEX_INITIALIZER;
    messageReceived = false;

    if (pthread_mutex_init(&mutex_buf, NULL) != 0)
    {
        cerr << "Error while intializing buffer mutex.\nExiting..." << endl;
        return 0;
    }


    cout << "- - - - - - - - - -" << endl <<
            "Params :\n"
            "Capture path: " << CAPTURE_PATH << endl <<
            "Captured image: width=" << CAPTURE_WIDTH << "px, height=" <<
            CAPTURE_HEIGHT << endl <<
            "Sleep time between captures (ms):" << SLEEP_MILLI << endl <<
            "Server IP: " << server_IP << endl <<
            "Socket port used: " << socket_port <<
            "- - - - - - - -" << endl << endl;

    if (!initNetwork())
    {
        cerr << "Error occured while initializing network configuration." << endl
                << "Exiting..." << endl;
        return 0;
    }

    if (pthread_create(&receivingThread, NULL, recvThread, receivingBuffer) != 0)
    {
        cerr << "Error while creating receiving thread.\nExiting..." << endl;
        return 0;
    }

    sleep(10);
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

        //std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MILLI));
    }

    return 0;

}

void argSetting(int argc, char* argv[])
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
    if (argc > 3)
    {
        CAPTURE_WIDTH = atoi(argv[3]);
    }

    //The captured image height is the fourth argument passed.
    if (argc > 4)
    {
        CAPTURE_HEIGHT = atoi(argv[4]);
    }
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

void processImage(Mat &imgOriginal)
{
    Mat imgHSV;

    cvtColor(imgOriginal, imgHSV, COLOR_RGB2HSV); //Convert the captured frame from BGR to HSV

    Mat imgThresholded;

    inRange(imgHSV, Scalar(0, 0, 240), Scalar(179, 20, 255), imgThresholded); //Threshold the image
    imwrite("/home/pi/ram/thresh.jpg", imgThresholded);

    vector<vector <Point> > contours;

    findContours(imgThresholded, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    cout << "Contours foud: " << contours.size() << endl;

    for (vector<Point> vec : contours)
    {

        Moments omoments = moments(vec);

        double dM01 = omoments.m01;
        double dM10 = omoments.m10;
        double dArea = omoments.m00;
        cout << "dArea" << dArea << endl;
        int posX, posY;
        if (dArea != 0)
        {
            //calculate the position of the ball
            posX = dM10 / dArea;
            posY = dM01 / dArea;
        } else
        {
            posX = vec.at(0).x;
            posY = vec.at(0).y;
        }

        cout << "x: " << posX << ", y: " << posY << endl;
    }

    drawContours(imgOriginal, contours, -1, Scalar(255, 0, 0));
    imwrite("/home/pi/ram/contours.jpg", imgOriginal);
}

