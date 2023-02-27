#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "cache.hpp"
#include "request.hpp"

class Proxy {
 public:
  long uid;
  int socket_des;
  std::string clientIP;
  http_Request * request;
  http_Response * response;

 public:
  //Constructor
  Proxy() :
      uid(0), socket_des(0), clientIP(), request(NULL), response(NULL) {}
  Proxy(long id, int sd, std::string ip) :
      uid(id),
      socket_des(sd),
      clientIP(ip),
      request(NULL),
      response(NULL)
      {}
  //Get the private field
  long return_UID() const { return uid; }
  int return_socket_des() const { return socket_des; }
  http_Request * return_request() const { return request; }
  http_Response * return_response() const { return response; }
  //Initialize Request
  void setRequest(std::string Line, std::vector<char> & line_send);
  int connectServer();
  void judgeRequest();
  void proxyCONNECT();
  void proxyPOST();
  void proxyGET();
  http_Response * proxyFetch(int socket_server, int socket_client);
  void proxyERROR(int code);
  bool chunkHandle(vector<char> & input, int server_fd);
  bool check502(vector<char> & input);
  std::vector<char> ConstructValidation(http_Response * response_instance);
  void HandleValidation(http_Response * response_instance, std::string request_url);
  int sendall(int s, char * buf, int * len);
  void receiveLog(http_Response * resp);
  void destructProxy() {
    if (request) {
      delete request;
    }
  }
  ~Proxy() {
    if (request) {
      delete request;
    }
  }
};

void * runProxy(void * myProxy);
void proxyListen();
