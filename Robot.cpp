/* 
 * File:   Robot.cpp
 * Author: Anthony Correia
 * 
 * Created on 27 janvier 2015, 14:22
 */

#include "Robot.h"

#include <string>

using namespace std;

double Robot::ratio = -1.0;
vector<Robot*> Robot::robots = vector<Robot*>();

Robot::Robot(int x_current, int y_current)
    : x_current(0), y_current(0)
{
    setX(x_current);
    setY(y_current);
    
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

void Robot::setY(int y_current)
{
    if (y_current > 0)
    {
        this->y_last = this->y_current;
        this->y_current = y_current;
    }
}

void Robot::setX(int x_current)
{
    if (x_current > 0)
    {
        this->x_last = this->x_current;
        this->x_current = x_current;
    }
}

string Robot::toJSON()
{
    string ret ="[";
    ret += to_string((double)(this->x_current)/ratio) + ",";
    ret += to_string((double)(this->y_current)/ratio) + "]";
    return ret;
}

string Robot::robotsToJSON()
{
    string ret = "{\"id\":\"localisation\",\"robots\":[";
    for(Robot* r : robots)
    {
        ret += r->toJSON();
        ret += ",";
    }
    ret.back() = ']';
    ret += "}";
    return ret;
}

