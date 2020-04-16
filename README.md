# Network-Layer-Verification
## General description
Our main goal is to provide a verified abstract interface for communication in
the TCP/IP network.
TCP is one of the main communication protocols of the internet, it provides reliable,
ordered and error-checked delivery of a stream of bytes between applications running on hosts communicating via an IP network.
On a TCP/IP network every device must have an IP address.the IP address identifies the computer. However an IP address alone is not sufficient for running network applications, as a computer can run multiple applications/services.
Just as the IP address identifies the computer, The network port identifies the application or service running on the computer.The use of ports allow computers/devices to run multiple services/applications.An IP address and a port number equals to a socket.
a network socket is an internal endpoint for sending or receiving data within
a node on a computer network.Concretely, it is a representation of this endpoint in networking software, such as an entry in a table.
In our project we will use the TCP protocol and sockets to establish the connection between a
client and our server and will provide a simple implementation for this type of connections.


## Implementation description
Our system is built on a central communication component(connectionManager)
that is not exposed to the users or any other computer receiving the messages,
whether they are intended for the computer or not.
The central component has a data structure that contains all the open sockets
on the computer.

When our system receives a message the connectionManager extracts the IP header
from it and checks if the destination address is the same as the computer it
works on. if it is equal we check if the protocol written in the IP header is
TCP.if the message is not intended to the computer our system works on we pass
the message to the next computer in the route to the destination address.

Once the TCP message is intended to us, we first, save the number of the message
written in the TCP header which represent the number of message in the current  
TCP connection in a buffer. second, we check the port number the message was
sent to, then the connectionManager will map it to the open socket that
listens to that port, if we don't find we will throw the message or send an
error message back to the client. then our system adds the message to the TCP
window that the socket contains, if we have the whole message we can return
a message according to the request  to the client.

## Features
1. Our system will manage the operations related to the sockets such as: creating
new communication endpoint("socket"), attaching a local address to a socket
("bind"), announcing willingness to accept connections("listen"), blocking caller until a connection request arrives("accept"), actively attempt to establish a connection("connection"),
sending data over the connection("send"), receive data from connection("receive"),
and finally, closing the connection("close").
2. Our system will manage the "congestion control" mechanism that deals with a
case when more data is sent to us than we can handle without losing any message.
3. Our system will manage the TCP window scale option, which will handle the operation
to increase the receive window size allowed in transmission control protocol above
the maximum value.
