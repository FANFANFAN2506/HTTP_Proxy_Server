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

#include "log.cpp"
#include "proxy.hpp"
#include "server.hpp"

#define PORT 12345
#define MAXPENDING 10
long requestID = 0;


int main(void) {
  Server listenServer(PORT);
  listenServer.Bind();
  listenServer.Listen(MAXPENDING);
  struct sockaddr_in clitAddr;
  socklen_t clitLen = sizeof(clitAddr);

  while (1) {
    int connfd = accept(listenServer.sock, (struct sockaddr *)&clitAddr, &clitLen);
    if (connfd < 0) {
      log(std::string("NULL: Failed to estblish connection with client"));
    }
    else {
      std::string ip = inet_ntoa(clitAddr.sin_addr);
      pthread_t thread;
      //THis pointer may need to be considered for RAII
      Proxy * myProxy = new Proxy(requestID, connfd, ip);
      pthread_create(&thread, NULL, runProxy, myProxy);
      requestID++;
    }
  }
}
