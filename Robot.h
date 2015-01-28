/* 
 * File:   Robot.h
 * Author: Anthony Correia
 *
 * Created on 27 janvier 2015, 14:22
 */

#ifndef ROBOT_H
#define	ROBOT_H

#define MAX_TRAVEL_DISTANCE 0.1 // Maximum distance a robot can travel in m/s.

#include <string>
#include <vector>
#include <chrono>

class Robot
{
public:
    Robot(int x_current, int y_current);
    Robot(const Robot& orig);
    virtual ~Robot();
    
    int getId() const
    {
        return id;
    }

    int getX() const
    {
        return x_current;
    }

    int getY() const
    {
        return y_current;
    }
    
    static double getRatio()
    {
        return ratio;
    }
    
    static void setRatio(double ration);

    void setPosition(int x_current, int y_current);
    
    std::string toJSON();
    
    static std::string robotsToJSON();


private:
    int id;
    int x_current, y_current; // The current positions of the robot.
    int x_last, y_last; //The previous positions (previous capture).
    static double ratio; //The number of pixels it takes to make a meter.
    static std::vector<Robot*> robots; //Vector of robots
    
    std::chrono::steady_clock::time_point lastTime;

};

#endif	/* ROBOT_H */

