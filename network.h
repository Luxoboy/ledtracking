#ifndef NETWORK_H
#define	NETWORK_H

#include <cstring>      // Needed for memset
#include <sys/socket.h> // Needed for the socket functions
#include <netdb.h>
#include <string>
#include <unistd.h>



/**
 * @brief Init the network configuration.
 * @return False if an error occured.
 * Set the structures.
 */
bool initNetwork();

/**
 * @brief Creates the socket to the server.
 * @return True if successfully created.
 */
bool createSocket();

/**
 * Closes socket.
 * @return True if successfull, false if an error occured.
 */
bool closeSocket();

/**
 * @brief Connects to the server.
 * @return True is successfully connected.
 */
bool connect();

/**
 * @brief Send a message to the server.
 * @param msg The message to send to the server.
 * @return True if successfully sent.
 */
bool send(std::string msg);

/**
 * Thread method that listens to incoming messages.
 * @param arg
 * @return 
 */
void *recvThread(void* arg);

extern int status;
extern struct addrinfo host_info; // The struct that getaddrinfo() fills up with data.
extern struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.
extern std::string server_IP, socket_port;
extern int socket_d; // Socket descriptor
extern bool messageReceived;
extern char receivingBuffer[1000];
extern pthread_t receivingThread;

#endif	/* NETWORK_H */

