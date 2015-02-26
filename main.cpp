#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <signal.h>
#include <chrono>

#include <pthread.h>
#include <bits/signum.h>

#include "network.h"
#include "Robot.h"
#include "global.h"
#include "processing.h"

using namespace std;

int CAPTURE_WIDTH = CAPTURE_WIDTH_DEFAULT,
        CAPTURE_HEIGHT = CAPTURE_HEIGHT_DEFAULT;


int raspiStillPID = -1; //The PID of the child process exectuting raspistill.

bool TRACKING = false;

pthread_mutex_t MODE;

int main(int argc, char** argv)
{
    if(signal(SIGINT, SIGINT_handler) != SIG_ERR)
    {
        cout << "Signal handling correctly set.\n";
    }
    else
    {
        perror("SIGIN handling");
        cerr << "Failed trying to handle SIGINT. Exiting.\n";
        return 0;
    }
    argSetting(argc, argv);
    
    MODE = PTHREAD_MUTEX_INITIALIZER;
    messageReceived = false;

    if (pthread_mutex_init(&MODE, NULL) != 0)
    {
        cerr << "Error while intializing mode mutex.\nExiting..." << endl;
        return 0;
    }


    cout << "- - - - - - - - - -" << endl <<
            "Params :\n"
            "Capture path: " << CAPTURE_PATH << endl <<
            "Captured image: width=" << CAPTURE_WIDTH << "px, height=" <<
            CAPTURE_HEIGHT << endl <<
            "Server IP: " << server_IP << endl <<
            "Socket port used: " << socket_port << endl <<
            "- - - - - - - - - -" << endl << endl;

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
    Robot::setRatio(427);
    Robot* r = new Robot(0,0), *r2 = new Robot(10,10);
    
    captureLoop();
    
    killRaspistill();
    closeSocket();

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

bool captureSignal()
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

void readMessage(string msg)
{
    Json::Reader reader;
    Json::Value root;
    
    if(!reader.parse(msg, root, false))
    {
        cout << "Received message could not be parsed as JSON!" << endl;
    }
    
    Json::Value action = root["action"];
    if(not action.isNull() && action.isString())
    {
        pthread_mutex_lock(&MODE);
        string actionString = action.asString();
        if(actionString == "calibrage")
        {
            Json::Value answer = initValue();
            answer["action"] = "calibrage";
            Json::FastWriter writer;
            Json::Value value = root["valeur"];
            
            if(!value.isNull() && value.isDouble())
            {
                TRACKING = false;
                string ret = calibrate(value.asDouble());
                if(ret != "")
                {
                    answer["statut"] = "NOK";
                    answer["message"] = ret;
                }
                else
                {
                    answer["statut"] = "OK";
                }
                send(writer.write(answer));
            }
            else
            {
                cout << "[JSON] Incorrect value for value field in calibrate action" <<
                        endl;
            }
        }
        else if(actionString == "start")
        {
            TRACKING = true;
        }
        else if(actionString == "stop")
        {
            TRACKING = false;
        }
        pthread_mutex_unlock(&MODE);
    }
}

Json::Value initValue()
{
    Json::Value val;
    val["idModule"] = "localisation";
    return val;
}

void killRaspistill()
{
    if(kill(raspiStillPID, SIGKILL) < 0)
    {
        cerr << "Error killing raspistill process. Please kill manually." << endl;
    }
    else
    {
        cout << "Killed raspistill process successfully." << endl;
    }
}

void SIGINT_handler(int sig)
{
    pthread_mutex_lock(&MODE);
    status = -1;
    pthread_mutex_unlock(&MODE);
}