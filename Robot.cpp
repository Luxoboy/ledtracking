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

Robot::~Robot()
{
}

void Robot::setPosition(int x_current, int y_current)
{
    if (x_current > 0)
    {
        this->x_last = this->x_current;
        this->x_current = x_current;
    }

    if (y_current > 0)
    {
        this->y_last = this->y_current;
        this->y_current = y_current;
    }

    lastTime = steady_clock::now();
}

string Robot::toJSON()
{
    string ret = "{\"id\":";
    ret += to_string(id);
    ret += ",\"x\":";
    ret += to_string((double) (this->x_current) / ratio) + ",";
    ret += "\"y\":"+to_string((double) (this->y_current) / ratio) + "}";
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
    ret += "}";
    return ret;
}

bool Robot::tryPosition(double x, double y)
{
    double travelledDistance = sqrt(pow(x - x_current, 2) + pow(y - y_current, 2));
    duration<double> moveDuration = duration_cast<duration<double>>(steady_clock::now() - lastTime);

    double durationSeconds = moveDuration.count();

    if (travelledDistance > MAX_TRAVEL_DISTANCE * durationSeconds)
    {
        return false;
    }
    setPosition(x, y);
    return true;
}

Robot& Robot::getRobot(uint id)
{
    if(id >= 0 && id < robots.size())
    {
        return robots.at(id);
    }
}

