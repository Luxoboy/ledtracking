/* 
 * File:   Robot.cpp
 * Author: Anthony Correia
 * 
 * Created on 27 janvier 2015, 14:22
 */

#include "Robot.h"

int Robot::numberRobots = 0;

Robot::Robot()
{
    id = numberRobots++;
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



