#include "proxy.hpp"

void * runProxy(void * myProxy) {
  Proxy * Proxy_instance = (Proxy *)myProxy;
  std::string Line = recvAll(Proxy_instance->socket_des);
  Proxy_instance->setRequest(Line);
}

//Proxy memeber functions:

void Proxy::setRequest(std::string Line) {
  try {
    std::string Received_time = get_current_Time();
    request = new http_Request(this->socket_des, Line, this->clientIP, Received_time);
  }
  catch (std::exception & e) {
    std::cerr << "Request construction failed" << std::endl;
  }
}

void Proxy::judgeRequest() {
  if (request->return_method().c_str() == "CONNECT") {
    /* If the method is CONNECT: 
    1. Setup the connection with target server
    2. The request line should be ignored, send the header & data to server
    3. Reply a HTTP 200 OK 
    */
    int socket_server = connectServer();
    const char * message_server = request->return_header().c_str();
    send(socket_server, &message_server, strlen(message_server), 0);
    const char * message_client = "HTTP/1.1 200 OK\r\n";
    send(request->return_socket_des(), &message_client, strlen(message_client), 0);
    connectTunnel(socket_server);
    //Finish connect
    pthread_exit(NULL);
  }
  else if (request->return_method().c_str() == "POST") {
    //The request is POST
  }
  else {
    //Thre request is GET
  }
}

int Proxy::connectServer() {
  int error;
  int socket_server;
  const char * server_ip = request->return_Host().c_str();
  const char * server_port = request->return_port().c_str();
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  error = getaddrinfo(server_ip, server_port, &hints, &res);
  if (error != 0) {
    std::cerr << "In connection: ";
    std::cerr << "Error: cannot get the address info from the host " << clientIP
              << " on port: " << server_port << std::endl;
    pthread_exit(NULL);
  }
  socket_server = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (socket_server == -1) {
    std::cerr << "Error: cannot create socket for " << clientIP
              << " on port: " << server_port << std::endl;
    pthread_exit(NULL);
  }
  error = connect(socket_server, res->ai_addr, res->ai_addrlen);
  if (error == -1) {
    std::cerr << "Error cannot connect to the socket" << std::endl;
    pthread_exit(NULL);
  }
  freeaddrinfo(res);
  return socket_server;
}

void Proxy::connectTunnel(int socket_server) {
  fd_set listen_ports;
  int maxdes;
  FD_ZERO(&listen_ports);
  int socket_client = request->return_socket_des();
  FD_SET(socket_client, &listen_ports);
  maxdes = socket_client;
  FD_SET(socket_server, &listen_ports);
  if (socket_server > maxdes) {
    maxdes = socket_server;
  }
  int end = 1;
  int error = 0;
  fd_set temp_list;
  FD_ZERO(&temp_list);
  while (end) {
    temp_list = listen_ports;
    error = select(maxdes + 1, &temp_list, NULL, NULL, NULL);
    if (error == -1) {
      std::cerr << "Error cannot select" << std::endl;
      pthread_exit(NULL);
    }
    for (int i = 0; i <= maxdes; i++) {
      if (FD_ISSET(i, &temp_list)) {
        //There is a message received
        std::string input = recvAll(i);
        if (input.size() == 0) {
          //One of the socket closed on there side
          close(socket_server);
          close(socket_client);
          FD_ZERO(&listen_ports);
          end = 0;
        }
        else {
          //We got the message
          const char * reply = input.c_str();
          if (i == socket_server) {
            //The message is from server
            send(socket_client, &reply, strlen(reply), 0);
          }
          else {
            //The message is from client
            send(socket_server, &reply, strlen(reply), 0);
          }
        }  //End of if for recv 0
      }    //End of if for check fd_set
    }      //End of go over the potential descriptor
  }        //main while loop
  return;  //End of func
}