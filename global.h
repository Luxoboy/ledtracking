#ifndef GLOBAL_H
#define	GLOBAL_H

#define CAPTURE_PATH "/home/pi/ram/capture.jpg"

#define CAPTURE_WIDTH_DEFAULT 800
#define CAPTURE_HEIGHT_DEFAULT 800

extern int CAPTURE_WIDTH, CAPTURE_HEIGHT;


extern int raspiStillPID; //The PID of the child process exectuting raspistill.

/**
 * @brief Fork process and execute raspistill.
 * This method forks the process. If successfully forked, the child process
 * will execute raspistill.
 */
void forkRaspistill();

/**
 * @brief Capture new picture.
 * @return True if the signal was successfully sent.
 * This method simply sends a SIGUSR1 signal to the raspitill process, triggering
 * the capture of a new frame.
 */
bool captureSignal();

/**
 * Sets variables from arguments passed.
 * @param argc
 * @param argv
 */
void argSetting(int argc, char* argv[]);

/**
 * Parse JSON incoming message and call approtiate methods.
 * @param msg
 */
void readMessage(std::string msg);

Json::Value initValue();



#endif	/* GLOBAL_H */

