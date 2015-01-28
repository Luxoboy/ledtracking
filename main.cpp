#include "Robot.h"

#include <iostream>
#include <unistd.h>
#include <math.h>

using namespace std;
int main(int argc, char* argv[])
{
    Robot r1(1,1), r2(2,2);
    
    cout<< Robot::robotsToJSON() << endl;
    sleep(1);
    r1.tryPosition(1,1.1);
    
    cout << sqrt(pow(1.1,2)) << endl;
    
    return 0;
    
}