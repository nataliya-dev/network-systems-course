
// Request
struct Request {
  string source;
  string destination;
  int processTime;
  int startTime;
  string header;
}

// Webserver
string servername;
queue<Request> requestQueue;

Webserver(char c) { servername = c; };
getName(){return servername};
addRequest(Request r) {
  requestQueue.enqueue(r);
  r.startTime = curTime;
};
getNumRequests() { return requestQueue.size(); }
isRequestQueueEmpty() { return requestQueue.empty(); };
