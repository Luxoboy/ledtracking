#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <signal.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"

using namespace cv;
using namespace std;

void forkRaspistill();

int raspiStillPID = -1;
int main(int argc, char** argv) {
    forkRaspistill();
    VideoCapture cap("/dev/stdin"); //capture the video from web cam
    //cap.set(CV_CAP_PROP_POS_AVI_RATIO, 1);
    //cap.set(CV_CAP_PROP_POS_FRAMES, 9999999999999999);
    while (!cap.isOpened()) // if not success, exit program
    {
        cout << "Cannot open the web cam" << endl;
        cap.open("stream");
        //return -1;
    }
    cout<<"Flux ouvert"<<endl;

    //namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

    int iLowH = 0;
    int iHighH = 179;

    int iLowS = 0;
    int iHighS = 255;

    int iLowV = 0;
    int iHighV = 255;

    //Create trackbars in "Control" window
    cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
    cvCreateTrackbar("HighH", "Control", &iHighH, 179);

    cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", "Control", &iHighS, 255);

    cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
    cvCreateTrackbar("HighV", "Control", &iHighV, 255);


    while (true) {
        Mat imgOriginal;

        bool bSuccess = cap.read(imgOriginal); // read a new frame from video

        while (!bSuccess) //if not success, break loop
        {
            cout << "Cannot read a frame from video stream" << endl;
            cap.read(imgOriginal);
            //break;
        }
        cout<<"Frame lue."<<endl;
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

        //imshow("Thresholded Image", imgThresholded); //show the thresholded image
        */
        imshow("Original", imgOriginal); //show the original image

        if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed by user" << endl;
            break;
        } 
    }

    return 0;

}


void forkRaspistill()
{
    int raspiStillPID = fork();
    if(raspiStillPID < 0)
    {
        cerr<<"Erreur fork raspistill.\n";
        exit(1);
    }
    if(raspiStillPID == 0)
    {
        execl("/usr/bin/raspistill", "raspistill", "-s -t 0 -o /home/pi/ram/capture.jpg", NULL);
    }
    sleep(6);
    exit(0);
}