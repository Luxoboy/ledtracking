/* 
 * File:   Robot.cpp
 * Author: Anthony Correia
 * 
 * Created on 27 janvier 2015, 14:22
 */

#include "Robot.h"
#include <math.h>
#include <iostream>

using namespace std;
using namespace std::chrono;

double Robot::ratio = -1.0;
vector<Robot*> Robot::robots = vector<Robot*>();

Robot::Robot(int x_current, int y_current)
: x_current(0), y_current(0)
{
    setPosition(x_current, y_current);
    id = robots.size();
    robots.push_back(this);
}

Robot::Robot(const Robot& orig)
{
    id = orig.id;

    x_current = orig.x_current;
    y_current = orig.y_current;

    x_last = orig.x_last;
    y_last = orig.y_last;
}

Robot::Robot()
{
    cout << "[ROBOT] Added new robot, with no position." << endl;
    setPosition(-1, -1);
    robots.push_back(this);
}

Robot::~Robot()
{
}

void Robot::clearRobots()
{
    robots.clear();
    cout << "[ROBOT] Cleared robot list." << endl;
}


void Robot::setRatio(double ratio)
{
    cout << "[ROBOT] Ratio changed to " << ratio << endl;
    Robot::ratio = ratio;
}

void Robot::setPosition(int x, int y)
{
    x_last = x_current;
    x_current = x;

    y_last = y_current;
    y_current = y;
    cout << "[ROBOT] " << id << ": coordinates successfully updated to " <<
            x_current << ", " << y_current << endl;
    lastTime = steady_clock::now();
}

string Robot::toJSON()
{
    cout << "[ROBOT] " << id << ": exporting to JSON.\nCurrent coordinates : " <<
            x_current << ", " << y_current << endl;
    string ret = "{\"id\":";
    ret += to_string(id);
    ret += ",\"x\":";
    ret += to_string(((double) (x_current) / ratio)*100.0) + ",";
    ret += "\"y\":" + to_string(((double) (y_current) / ratio)*100) + "}";
    cout << "JSON: " << ret << endl;
    return ret;
}

string Robot::robotsToJSON()
{
    string ret = "{\"id\":\"localisation\",\"robots\":[";
    for (Robot* r : robots)
    {
        ret += r->toJSON();
        ret += ",";
    }
    ret.back() = ']';
    ret += "}\n";
    return ret;
}

bool Robot::tryPosition(double x, double y)
{
    if (x_current != -1 && y_current != -1)
    {
        double travelledDistance = sqrt(pow(x - x_current, 2) + pow(y - y_current, 2));
        duration<double> moveDuration = duration_cast<duration<double>>(steady_clock::now() - lastTime);

        double durationSeconds = moveDuration.count();

        if (travelledDistance > MAX_TRAVEL_DISTANCE * durationSeconds)
        {
            cout << "[ROBOT] " << id << ": failed to update coordinates.\n";
            return false;
        }
    }
    setPosition(x, y);
    return true;
}

Robot* Robot::getRobot(uint id)
{
    if (id < robots.size())
    {
        return robots.at(id);
    }
    return NULL;
}

uint Robot::numberOfRobots()
{
    return robots.size();
}

