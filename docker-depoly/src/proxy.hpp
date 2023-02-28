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
#include <memory>

#include "cache.hpp"
#include "request.hpp"

class Proxy {
 public:
  long uid;
  int socket_des;
  int server_des;
  std::string clientIP;
  // http_Request * request;
  std::unique_ptr<http_Request> request;

 public:
  //Constructor
  Proxy() : uid(0), socket_des(0), server_des(-1), clientIP(), request(nullptr) {}
  Proxy(long id, int sd, std::string ip) :
      uid(id), socket_des(sd), clientIP(ip), request(nullptr) {}
  //Get the private field
  long return_UID() const { return uid; }
  int return_socket_des() const { return socket_des; }
  // http_Request * return_request() const { return request; }
  // std::unique_ptr<http_Request> return_request() const { return request; }
  //Initialize Request
  void setRequest(std::string Line, std::vector<char> & line_send);
  int connectServer();
  void judgeRequest();
  void proxyCONNECT();
  void proxyPOST();
  void proxyGET();
  http_Response * proxyFetch(int socket_server, int socket_client);
  void proxyERROR(int code);
  http_Response * chunkHandle(vector<char> & input, int server_fd);
  bool check502(vector<char> & input);
  std::vector<char> ConstructValidation(http_Response * response_instance);
  void HandleValidation(http_Response * response_instance, std::string request_url);
  int sendall(int s, char * buf, int * len);
  void receiveLog(http_Response * resp);
  ~Proxy() {
    close(socket_des);
    if (server_des) {
      close(server_des);
    }
    // if (request) {
    //   delete request;
    // }
  }
};

void * runProxy(void * myProxy);
void * runProxy1(std::unique_ptr<Proxy> myProxy);
void proxyListen();
