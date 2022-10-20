## Summary
The following code is a basic implementation of an HTTP-based server which can handle the following:
- GET and POST commands
- Default page handling
- Handling multiple connections
- Error handling
- Pipelining

## How to run the code
In order to run the code: 
```
cd [path to folder]
make
./server [port number]
```

## How to test the code
First, run the code as described in the instructions above with port number 9999:
```
./server 9999
```
Then, to test that the server can correctly display a webpage, you can load http://localhost:9999/index.html on your browser of choice (FireFox). 

To test a GET request for a single file, you can use: 
```
curl http://localhost:9999/index.html
```

To test pipelining, you can use: 
```
curl http://localhost:9999/index.html -H "Connection: Keep-alive"
```
Note that the thread shuld not exit immediately as it did with the previus request where we did not specify the connection type. 

To test error handling you can use a flawed input file name as such and you will see a 500 error in return:
```
curl http://localhost:9999/indexxx.html --verbose
```

To test the POST command you will keep the server open in one terminal and type the following command in another terminal: 
```
curl -X POST http://localhost:9999/index.html -H "Content-Type: text/html" -d "My name is Nataliya"
```