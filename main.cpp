#include "Robot.h"

#include <iostream>

using namespace std;
int main(int argc, char* argv[])
{
    Robot r1(1,1), r2(2,2);
    
    cout<< Robot::robotsToJSON();
    
}