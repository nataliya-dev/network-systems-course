# UDP Socket Programming
## Programming Assignment 1

## Background
UDP stands for Use Datagram Protocol and is a connectionless transmission model with a minimum protocol mechanism. ACKs and handshaking are not part of its design and implementation. Although traditionally packets are not guaranteed to be received, in this assignment we implement just that -- reliability. To ensure that packets get transmitted, the receiver waits for frame acknowledgment based on the sequence number of the packet received. A timeout feature allows the receiver to resend a packet in case an acknowledgment has been dropped. This is a minimum working model and is meant to demonstrate the basic functionality of reliable UDP transfer.

## Compile
```
cd [socket_folder]
make
```

## Usage
Open the first terminal:
```
cd client
./client [hostname] [port]
```
Open the second terminal:
```
cd server
./server [port]
```

On the clinet side, you will be prompted to type out one of the following commands:
```
get [filename]
put [filename]
delete [filename]
ls
exit
```
Below is a brief explanation of the commands:

- The "get" command asks the server to transmit a file.
- The "put" command transmits a file to the server.
- The "delete" command asks the server to delete a file in its directory.
- The "ls" command asks the server to send a list of files in its local directory.
- And finally, the "exit" command asks the server to exit gracefully.

## Folder structure
- client - stores files and executables used for the client.
- server - stores files and executable used for the server.
- utils - a collection of header files with shared functions used by the server and the client.

## Author
Nataliya Nechyporenko

