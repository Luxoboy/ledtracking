#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <signal.h>
#include <chrono>

#include <pthread.h>

#include "network.h"
#include "Robot.h"
#include "global.h"
#include "processing.h"

using namespace std;

int CAPTURE_WIDTH = CAPTURE_WIDTH_DEFAULT,
        CAPTURE_HEIGHT = CAPTURE_HEIGHT_DEFAULT;


int raspiStillPID = -1; //The PID of the child process exectuting raspistill.

int main(int argc, char** argv)
{
    argSetting(argc, argv);
    
    mutex_buf = PTHREAD_MUTEX_INITIALIZER;
    messageReceived = false;

    if (pthread_mutex_init(&mutex_buf, NULL) != 0)
    {
        cerr << "Error while intializing buffer mutex.\nExiting..." << endl;
        return 0;
    }


    cout << "- - - - - - - - - -" << endl <<
            "Params :\n"
            "Capture path: " << CAPTURE_PATH << endl <<
            "Captured image: width=" << CAPTURE_WIDTH << "px, height=" <<
            CAPTURE_HEIGHT << endl <<
            "Sleep time between captures (ms):" << SLEEP_MILLI << endl <<
            "Server IP: " << server_IP << endl <<
            "Socket port used: " << socket_port << endl <<
            "- - - - - - - -" << endl << endl;

    if (!initNetwork())
    {
        cerr << "Error occured while initializing network configuration." << endl
                << "Exiting..." << endl;
        return 0;
    }

    if (pthread_create(&receivingThread, NULL, recvThread, receivingBuffer) != 0)
    {
        cerr << "Error while creating receiving thread.\nExiting..." << endl;
        return 0;
    }

    forkRaspistill();
    Robot::setRatio(500);
    Robot r(0,0), r2(10,10);
    
    captureLoop();

    return 0;

}

void argSetting(int argc, char* argv[])
{
    //The IP address is the first argument passed to the program.
    if (argc > 1)
        server_IP = argv[1];
    else
    {
        cout << "No address IP passed, setting default value." << endl;
        server_IP = "192.168.1.3";
    }


    //The port is the second argument passed.
    if (argc > 2)
        socket_port = argv[2];
    else
    {
        socket_port = "3000";
        cout << "No socket port passed, setting default value." << endl;
    }

    //The captured image width is the third argument passed.
    if (argc > 3)
    {
        CAPTURE_WIDTH = atoi(argv[3]);
    }

    //The captured image height is the fourth argument passed.
    if (argc > 4)
    {
        CAPTURE_HEIGHT = atoi(argv[4]);
    }
}

void forkRaspistill()
{
    cout << "Trying to fork.\n";
    raspiStillPID = fork();
    if (raspiStillPID < 0)
    {
        cerr << "Error forking.\n";
        exit(1);
    }
    if (raspiStillPID == 0)
    {
        cout << "Fork successful. Executing raspistill.\n";
        if (execl("/usr/bin/raspistill", "raspistill", "-w", to_string(CAPTURE_WIDTH).c_str(),
                "-h", to_string(CAPTURE_HEIGHT).c_str(), "-t", "0", "-s", "-o",
                CAPTURE_PATH, "-q", "100", NULL) < 0)
        {
            cerr << "Error executing raspistill. Terminating child process...\n";
            exit(1);
        }
    }
    sleep(1);
}

bool capture()
{
    bool ret = true;
    cout << "Sending SIGUSR1 signal to child process.\n";
    if (kill(raspiStillPID, SIGUSR1) < 0)
    {
        cerr << "Error while sending signal.\n";
        ret = false;
    }
    return ret;
}