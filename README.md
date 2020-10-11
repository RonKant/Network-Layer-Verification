# Network-Layer-Verification
## General Description
Our main goal is to provide a verified abstract interface for communication in
the TCP/IP layer.
TCP is one of the main communication protocols of the internet, and it provides reliable,
ordered and error-checked delivery of a stream of bytes between applications running on hosts communicating via IP protocol.
On a TCP/IP network every device must have an IP address which is the device's unique identifier. However, an IP address alone is not sufficient for running network applications, as a computer may run multiple applications/services.
Just as the IP address identifies the computer, the TCP protocol provides the network port, which identifies the application or service running on a computer.The use of ports allows computers/devices to run multiple services/applications. A pair of an IP address and a port number is attributed to a socket.
A network socket is an internal endpoint for sending or receiving data within a node on a computer network. More specifically, it is a representation of this endpoint in networking software, such as an entry in a table.
In our project we will use the TCP protocol and sockets to establish the connection between a clients and servers and will provide a simple interface for using this type of connections.


## Implementation Description
Our system is built on a central communication component (NetworkManager)
which is hidden from the users or any other computer receiving the messages,
whether they are intended for that computer or not.
The central component has a data structure that contains and manages all the open sockets
on the computer (each identified by an IP address+port pair).

When our system receives a message, the NetworkManager extracts its IP header
and checks whether the destination address is the same as the computer it
runs on. In the case where the message's destination is a different device, the communication component passes it onward in the network.

In the case where the message is indeed directed to the current device, it is parsed for a TCP header, which is further extracted to identify the message's destination port in our system. The connectionManager will then either forward the message it to the open matching socket that
listens on that port, or otherwise will throw the message away. 
The message is then placed in the TCP window that the socket contains, to be parsed further and joined together with other TCP packets to construct their joined message and return that to the software that is using our socket.

## Features
Our system will manage the operations related to the sockets such as: creating
new communication endpoints ("socket"), attaching a local address to a socket ("bind"), announcing willingness to accept connections ("listen"), blocking callers until a connection request arrives ("accept"), actively attempting to establish a connection ("connect"),
sending data over a connection ("send"), receive data from a connection ("receive"),
and finally, closing a connection("close").
