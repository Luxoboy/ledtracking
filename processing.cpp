#include "processing.h"
#include "global.h"

#include <iostream>
#include <thread>

using namespace std;
using namespace cv;

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

void captureLoop()
{
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
}

vector<Moments>& computeMoments(vector<Moments>& vec_moments, vector<vector<Point>> vectors)
{
    for(vector<Point> points : vectors)
    {
        vec_moments.push_back(moments(points));
    }
    sort(vec_moments.begin(), vec_moments.end(), sortMoments);
    
    return vec_moments;
}

bool sortMoments(const Moments& m1, const Moments m2)
{
    return m1.m00 > m2.m00;
}
