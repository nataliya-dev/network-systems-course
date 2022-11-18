## Objective

The purpose of this assignment was to create a local web proxy that can relay HTTP request from clients to HTTP servers.

## Completed parts
- The basics - running the webproxy mean running an executable with two arguments: port number and timeout
- Listening and accepting connections from HTTP clients - only GET methods are supported and each request is handled in a separate thread
- Parsing requests from HTTP clients - each request can connect to a specified port, host, and retrieve the required file
- Forwarding these requests to the HTTP server and relaying data - a new connection is created and data is retrieved and forwarded
- Multi-threaded Proxy - using pthread to create different threads so they can run in parallel with efficiency
- Caching - the timeout, page cache, expiration, hostname IP addesss, and blacklist all have been implemented
- Link Prefets - NOT implemented

## How to run the code
In order to run the code:
```
cd [path to folder]
make
./webproxy [port number] [timeout]
```

## Design decisions
-


## Testing
- Set the appropriate proxy setting on your browser
- Use www.example.com or netsys.cs.colorado.edu as starting points to test the execution. You will see the folder becomeing populated with cached files. After a timeout, if you request another webpage then you will see old files get deleted and new files becoming populated.