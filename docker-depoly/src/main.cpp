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

#include "log.cpp"
#include "server.hpp"

#define PORT 12345
#define MAXPENDING 10
long requestID = 0;

int main(void){
    Server listenServer(PORT);
    listenServer.Bind();
    listenServer.Listen(MAXPENDING);
    struct sockaddr_in clitAddr;
    socklen_t clitLen = sizeof(clitAddr);

    while(1){
        int connfd = accept(listenServer.sock,(struct sockaddr*)&clitAddr, &clitLen);
        if(connfd == -1){
            log(std::string("NULL: Failed to estblish connection with client"));
        }else{
            requestID++;
            pthread_t thread;
            //pthread_create(&thread, NULL, proxy, );
        }
    }
}