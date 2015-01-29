#include "processing.h"
#include "global.h"
#include "network.h"
#include "Robot.h"

#include <iostream>
#include <thread>

using namespace std;
using namespace cv;

void processImage(Mat &img)
{
    Mat imgOriginal(img);
    
    prepareImage(img);
    
    vector<Moments> vec_moments;
    extractMoments(vec_moments, img);

    uint ind_robot = 0;

    vector<Moments>::iterator it = vec_moments.begin();

    while (!vec_moments.empty())
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
            if (!r->tryPosition(posX, posY))
            {
                it++;
            } else
            {
                vec_moments.erase(it);
                if (++ind_robot == Robot::numberOfRobots())
                    break;
            }
        } else
        {
            vec_moments.erase(it);
        }
    }
    send(Robot::robotsToJSON());
}

void prepareImage(Mat& imgOriginal)
{
    Mat imgHSV;

    cvtColor(imgOriginal, imgHSV, COLOR_RGB2HSV); //Convert the captured frame from BGR to HSV

    Mat imgThresholded;

    inRange(imgHSV, Scalar(0, 0, 240), Scalar(179, 20, 255), imgThresholded); //Threshold the image
    imwrite("/home/pi/ram/thresh.jpg", imgThresholded);
    
    imgOriginal = imgThresholded;
}

void captureLoop()
{
    int capturedFrames = 0;
    Mat imgOriginal;
    while (true)
    {
        captureFrame(imgOriginal);
        capturedFrames++;
        processImage(imgOriginal);
    }
}

bool captureFrame(Mat& imgOriginal)
{
    if (!captureSignal())
    {
        return false;
    }

    //Waiting for the image to be saved.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int failures = 0;
    do
    {
        imgOriginal = imread(CAPTURE_PATH);
        if (imgOriginal.data == NULL)
        {
            cerr << "Error while reading " << CAPTURE_PATH << ".\n" << endl;
            failures++;
            if (failures > MAX_FAILURES)
            {
                cout << "More than " << MAX_FAILURES << "  failures. Exiting...\n";
                exit(1);
            }
            cout << "Trying again.\n";
        } else
        {
            //Image opend successfully
            cout << "Frame opened successfully.\n";
            break;
        }

    } while (true);
    return true;
}

void extractMoments(vector<Moments>& vec_moments, Mat& img)
{
    vector<vector <Point> > contours;

    findContours(img, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    
    drawContours(img, contours, -1, Scalar(255, 0, 0));
    imwrite("/home/pi/ram/contours.jpg", img);

    cout << "Contours foud: " << contours.size() << endl;
    
    vec_moments.clear();
    vec_moments.reserve(contours.size());

    computeMoments(vec_moments, contours);
}

vector<Moments>& computeMoments(vector<Moments>& vec_moments, vector<vector<Point>> vectors)
{
    for (vector<Point> points : vectors)
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
