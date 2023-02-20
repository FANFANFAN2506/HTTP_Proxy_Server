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

class Server{
public:
    int sock;
    struct sockaddr_in servIP;
    
public:
    Server(int port){
        servIP.sin_family = AF_INET;
        servIP.sin_port = htons(port);
        servIP.sin_addr.s_addr = htonl(INADDR_ANY);
        this->sock = socket(AF_INET,SOCK_STREAM,0);
        if (this->sock == -1){
            std::cerr << "Socket Create Fail!" << std::endl;
        }
    }

    ~Server(){}

    int Bind(){
        int ans = bind(sock,(struct sockaddr*)&servIP,sizeof(servIP));
        if (ans == -1){
            std::cerr << "Socket Bind Fail!" << std::endl;
        }
        return 0;
    }

    int Listen(int maxbacklog){
        int ans = listen(sock,maxbacklog);
        if (ans == -1){
            std::cerr << "Socket Listen Fail!" << std::endl;
        }
        return 0;
    }
};