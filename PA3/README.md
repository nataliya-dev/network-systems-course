## Objective

The purpose of this assignment was to create a local web proxy that can relay HTTP request from clients to HTTP servers.

## Completed parts
- The basics - running the webproxy mean running an executable with two arguments: port number and timeout
- Listening and accepting connections from HTTP clients - only GET methods are supported and each request is handled in a separate thread
- Parsing requests from HTTP clients - each request can connect to a specified port, host, and retrieve the required file
- Forwarding these requests to the HTTP server and relaying data - a new connection is created and data is retrieved and forwarded
- Multi-threaded Proxy - using pthread to create different threads so they can run in parallel with efficiency
- Caching - the timeout, page cache, expiration, hostname IP addesss, and blacklist all have been implemented
- Error handling - forbidden and bad requests
- Link Prefets - NOT implemented

## How to run the code
In order to run the code:
```
cd [path to folder]
make
./webproxy [port number] [timeout]
```

## Design decisions
- webproxy.c - starts a listening port and spins a thread for incoming connections
- connection.c - creates an approrpiate thread for each request and starts the exchange_data() function which then handles the GET requests. This is the multithreading section of the assignment.
- parser.c - the part of the code dealing with the request. It handles parsing, caching, and the exchange between servers and clients.
- Blacklist was implemented using a text file. The program parses this file to check whether or not the hostname matches one of the elements in the file. If so then an error is returned. otherwise the code continues.
- There are two parts to caching. One is maintaing a list of available files on RAM. This part is implemented using an array of structs. This array consists of a key, this is the hostname and a time. The time is when the file was written to the disk. When checking for a cached file, the time is compared to current time. If there is a timeout then the program connects to host otherwise the cached file is retrieved locally. The second part is storing the file received from host. This file name is hostname/file except that the '/' charachter are replaced with '?' this way, there is no confusion with directories.
- Removing files from the cache happens after any one thread sends over a file. This was done for program simplicity, but would be better done every 10 or so seconds in a separate thread.
- An implementation of mutexes is also present to protect the RAM files which are used by multiple threads such as arrays, blacklist file, etc.



## Testing
- Set the appropriate proxy setting on your browser
- Use www.example.com or netsys.cs.colorado.edu as starting points to test the execution. You will see the folder becomeing populated with cached files. After a timeout, if you request another webpage then you will see old files get deleted and new files becoming populated.