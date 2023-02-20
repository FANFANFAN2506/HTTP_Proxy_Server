#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

class Proxy{
public:
    long uid;
    int socket;
    std::string clientIP;
    // Request * request;
    // Response * response;
    
public:
    Proxy(long uid, int socket, std::string clientIP){
        this->uid = uid;
        this->socket = socket;
        this->clientIP = clientIP;
    }

    ~Proxy(){}
};

void * runProxy(void * myProxy);