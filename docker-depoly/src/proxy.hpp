#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "request.hpp"
class Proxy {
 public:
  long uid;
  int socket;
  std::string clientIP;
  // Request * request;
  // Response * response;

 public:
  Proxy(long uid, int socket, std::string clientIP) {
    this->uid = uid;
    this->socket = socket;
    this->clientIP = clientIP;
  }
  //If we have destructor, we may need to follow rule of 5 as it is c++11
  //   ~Proxy() {}
};

void * runProxy(void * myProxy);