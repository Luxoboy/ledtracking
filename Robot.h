/* 
 * File:   Robot.h
 * Author: Anthony Correia
 *
 * Created on 27 janvier 2015, 14:22
 */

#ifndef ROBOT_H
#define	ROBOT_H

class Robot {
public:
    Robot();
    Robot(const Robot& orig);
    virtual ~Robot();

    int GetId() const {
        return id;
    }

    int GetX() const {
        return x_current;
    }

    void SetX(int x_current) {
        this->x_last = this->x_current;
        this->x_current = x_current;
    }

    int GetY() const {
        return y_current;
    }

    void SetY(int y_current) {
        this->y_last = this->y_current;
        this->y_current = y_current;
    }
    
    

    static double ratio;
    static int numberRobots;
private:
    int id;
    int x_current, y_current; // The current positions of the robot.
    int x_last, y_last; //The previous positions (previous capture).

};

#endif	/* ROBOT_H */

