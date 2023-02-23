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
  Cache * cache;

 public:
  //Constructor
  Proxy() : uid(0), socket_des(0), clientIP(), request(NULL) /*, response(NULL)*/ {}
  Proxy(long id, int sd, std::string ip, Cache * cache) :
      uid(id),
      socket_des(sd),
      clientIP(ip),
      request(NULL),
      cache(cache) /*,response(NULL)*/ {}
  //Get the private field
  long return_UID() const { return uid; }
  int return_socket_des() const { return socket_des; }
  http_Request * return_request() const { return request; }
  http_Response * return_response() const { return response; }
  //Initialize Request
  void setRequest(std::string Line);
  int connectServer();
  void judgeRequest();
  void connectTunnel(int socket_server);
  void handleConnect(int server_fd);
  ~Proxy() {}
};

void * runProxy(void * myProxy);
