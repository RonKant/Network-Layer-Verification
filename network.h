#ifndef __NETWORK_H__
#define __NETWORK_H__

/* 
 *	This file will contain signatures and additional definitions
 *	for the final interface of our project.
 */

// Unique identifier of a socket object.
typedef struct {
	int id; // positive unique, or -1
	int src_port, dst_port;
	char* src_ip, *dst_ip;
} * SocketID;

#define ILLEGAL_SOCKET_ID -1

// All possible error codes.
typedef enum {
	SUCCESS,
	INVALID_ARGUMENT,
	MEMORY_ERROR,
	REQUEST_DENIED
} Status;

/*
 * 	An (ip, port) tuple for representing destinations accross the network.
 */
typedef struct {
	char* addr;
	int port;
}* Address;

/**
 * Allocates a new Address object.
 * Returns NULL on failure.
 */
Address AddressCreate(const char* ip, int port);

/**
 * Frees all memory related to address.
 */
void AddressDestroy(Address address);


/*
 * Creates a new socket object.
 * On success - returns the new socket's identifier.
 * On failure - returns ILLEGAL_SOCKET_ID.
 */
SocketID SocketCreate();

/**
 * Frees all memory related to SocketID object (does not close socket!).
 */
void SocketDestroy(SocketID sockid);

/*
 * Closes a given socket and it's current ongoing connection.
 * Also destroys all memroy related to socket.
 */
Status SocketClose(SocketID sockid);

/*
 * Binds a socket to a given address.
 */
Status SocketBind(SocketID sockid, Address addrport);

/*
 * Instructs TCP protocol to listen for connections on given socket.
 * Parameter queueLimit - the maximum number of listenable connections.
 */
Status SocketListen(SocketID sockid, int queueLimit);

/*
 * Establishes a connection with a remote socket (blocking).
 * Parameter foreignAddr - the remote (ip, port) to connect to. 
 */
Status SocketConnect(SocketID sockid, Address foreignAddr); // TODO: in our implementation one has to bind before connecting (so we don't randomize client port)
															// distinct (by enum) a socket bound+LISTEN and a socket bound+nothing.

/*
 * Gets a new socket for an incoming client connection.
 * Parameter Address - after returning, will contain remote (ip, port) of the new connection. 
 * On success - returns a new registered socket for communication with client.
 * On failre - returns ILLEGAL_SOCKET_ID.
 */
SocketID SocketAccept(SocketID sockid, Address clientAddr);

/*
 * Sends a message to the socket at the other end of an existing connection.
 * Returns the number of bytes transmitted (-1 on failure).
 */
int SocketSend(SocketID sockid, char* message);

/*
 * Waits for receiving data from the other end of an existing connection.
 * Parameter bufferLen - the size of recvBuffer, into which the received message will be written. 
 */
int SocketRecv(SocketID sockid, char* recvBuffer, int bufferLen);

#endif