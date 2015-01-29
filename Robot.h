/* 
 * File:   Robot.h
 * Author: Anthony Correia
 *
 * Created on 27 janvier 2015, 14:22
 */

#ifndef ROBOT_H
#define	ROBOT_H

#define MAX_TRAVEL_DISTANCE 0.5 // Maximum distance a robot can travel in 1 second in m.

#include <string>
#include <vector>
#include <chrono>

class Robot
{
public:
    /**
     * Constructs a Robot with current position.
     * @param x_current
     * @param y_current
     */
    Robot(int x_current, int y_current);
    Robot(const Robot& orig);
    virtual ~Robot();
    
    static Robot* getRobot(uint id);
    
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
    
    static uint numberOfRobots();
    
    /**
     * Sets the ratio between pixels and meter.
     * @param ration
     */
    static void setRatio(double ratio);

    
    /**
     * Checks if position is correct according to ratio. If so, it sets the 
     * current position.
     * @param x
     * @param y
     * @return True if position was accepted.
     */
    bool tryPosition(double x, double y);
    
    std::string toJSON();
    
    static std::string robotsToJSON();


private:
    int id;
    int x_current, y_current; // The current positions of the robot.
    int x_last, y_last; //The previous positions (previous capture).
    static double ratio; //The number of pixels it takes to make a meter.
    static std::vector<Robot*> robots; //Vector of robots
    
    std::chrono::steady_clock::time_point lastTime;
    
    void setPosition(int x, int y); 

};

#endif	/* ROBOT_H */

