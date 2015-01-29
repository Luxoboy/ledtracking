#include "processing.h"
#include "global.h"
#include "network.h"
#include "Robot.h"

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
    
    vector<Moments> vec_moments(contours.size());
    
    computeMoments(vec_moments, contours);
    
    uint ind_robot = 0;
    
    vector<Moments>::iterator it = vec_moments.begin();

    while(!vec_moments.empty())
    {
        Moments m = *it;
        double dM01 = m.m01;
        double dM10 = m.m10;
        double dArea = m.m00;
        cout << "dArea" << dArea << endl;
        int posX, posY;
        if (dArea != 0)
        {
            posX = dM10 / dArea;
            posY = dM01 / dArea;
            Robot* r = Robot::getRobot(ind_robot);
            if(!r->tryPosition(posX, posY))
            {
                it++;
            }
            else
            {
                vec_moments.erase(it);
                if(++ind_robot == Robot::numberOfRobots())
                    break;
            }
        }
        else
        {
            vec_moments.erase(it);
        }
    }
    send(Robot::robotsToJSON());
    drawContours(imgOriginal, contours, -1, Scalar(255, 0, 0));
    imwrite("/home/pi/ram/contours.jpg", imgOriginal);
    int a;
    //cin >> a;
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
