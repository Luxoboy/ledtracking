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

int raspiStillPID = -1; //The PID of the child process exectuting raspistill.


int main(int argc, char** argv) {
    
    
    cout<<"Params :\n"
            "Capture path: "<<CAPTURE_PATH<<endl<<
            "Sleep time between captures (ms):"<<SLEEP_MILLI<<endl;
    

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
    while (true) {
        
        if(!capture())
        {
            continue;
        }
        
        capturedFrames++;
        
        //Waiting for the image to be saved.
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        
        cout<<"Trying to open "<<CAPTURE_PATH<<", frame number "<<capturedFrames<<"\n";
        Mat imgOriginal;
        
        int failures = 0;
        do
        {
            imgOriginal = imread(CAPTURE_PATH, CV_LOAD_IMAGE_COLOR);
            if(imgOriginal.data == NULL)
            {
                cerr<<"Error while reading "<<CAPTURE_PATH<<".\n"<<endl;
                failures++;
                if(failures > 100)
                {
                    cout<<"More than 10 failures. Exiting...\n";
                    exit(1);
                }
                cout<<"Trying again.\n";
            }
            else
            {
                //Image opend successfully
                break;
            }
            
        }
        while(true);
        
        std::string str = to_string(capturedFrames)+".jpg";
        
        cout<<"Trying to rename "<<CAPTURE_PATH<<" to "<<str<<endl;
        rename(CAPTURE_PATH, str.c_str());
        
        

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
    cout<<"Trying to fork.\n";
    raspiStillPID = fork();
    if(raspiStillPID < 0)
    {
        cerr<<"Error forking.\n";
        exit(1);
    }
    if(raspiStillPID == 0)
    {
        cout<<"Fork successful. Executing raspistill.\n";
        if(execl("/usr/bin/raspistill", "raspistill", "-t", "0", "-s", "-o", CAPTURE_PATH, NULL) < 0)
        {
            cerr<<"Error executing raspistill. Terminating child process...\n";
            exit(1);
        }
    }
    sleep(1);
}

bool capture()
{
    bool ret = true;
    cout<<"Sending SIGUSR1 signal to child process.\n";
    if(kill(raspiStillPID, SIGUSR1) < 0)
    {
        cerr<<"Error while sending signal.\n";
        ret = false;
    }
    return ret;
}