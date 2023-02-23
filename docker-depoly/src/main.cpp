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

#define PORT "12345"
#define MAXPENDING 10
#define MAXCachingCapacity 100
long requestID = 0;

using namespace std;

int main(int argc, char * argv[]) {
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list, *a;
  // const char * hostname = NULL;
  const char * hostname = "0.0.0.0";
  const char * port = PORT;

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  // host_info.ai_flags    = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }
  for (a = host_info_list; a != NULL; a = a->ai_next) {
    socket_fd = socket(a->ai_family, a->ai_socktype, a->ai_protocol);
    if (socket_fd == -1) {
      cerr << "Error: cannot create socket" << endl;
      cerr << "  (" << hostname << "," << port << ")" << endl;
      continue;
    }

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, a->ai_addr, a->ai_addrlen);
    if (status == -1) {
      close(socket_fd);
      cerr << "Error: cannot bind socket" << endl;
      cerr << "  (" << hostname << "," << port << ")" << endl;
      continue;
    }
    break;
  }
  if (a == NULL) {
    cerr << "selectserver failed to bind" << endl;
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(host_info_list);
  status = listen(socket_fd, MAXPENDING);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }

  cout << "Waiting for connection on port " << port << endl;
  struct sockaddr_in socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd;
  Cache * cache = new Cache(MAXCachingCapacity);
  while (1) {
    client_connection_fd =
        accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
      log("NULL: Failed to estblish connection with client");
    }
    else {
      string ip = inet_ntoa(socket_addr.sin_addr);
      pthread_t thread;
      //This pointer may need to be considered for RAII
      Proxy * myProxy = new Proxy(requestID, client_connection_fd, ip, cache);
      pthread_create(&thread, NULL, runProxy, myProxy);
      //pthread_join(thread,NULL);
      requestID++;
    }
  }
  return 0;
}