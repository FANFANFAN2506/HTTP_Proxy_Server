#include "proxy.hpp"
#include <unistd.h>
#include "utils.hpp"

//异常处理(对于一些不存在的域名，不需要pending)，跨方法 log, Response 解析
void * runProxy(void * myProxy) {
  Proxy * Proxy_instance = (Proxy *)myProxy;
  // std::string Line = receiveAll(Proxy_instance->socket_des);
  std::string Line = recvAll(Proxy_instance->return_socket_des());
  //std::cout << "Line received is " << Line << std::endl;
  Proxy_instance->setRequest(Line);
  Proxy_instance->judgeRequest();
  // Proxy_instance->destructProxy();
  delete Proxy_instance;
  return NULL;
}

//Proxy memeber functions:

void Proxy::setRequest(std::string Line) {
  try {
    // std::string Received_time = get_current_Time();
    time_t curr_time;
    time(&curr_time);
    request = new http_Request(this->socket_des, Line, this->clientIP, curr_time);
    request->constructRequest();
    std::string msg = to_string(uid) + ": \""  + request->return_request() +"\" from "+
    request->return_ip() + " @ " + parseTime(request->return_time());
    log(msg);
  }
  catch (std::exception & e) {
    delete request;
    //std::cerr << "Request construction failed" << std::endl;
    log(std::string(to_string(uid) + ": ERROR request construction failed \n"));
    pthread_exit(0);
  }
}

void Proxy::judgeRequest() {
  if (request->return_method() == "CONNECT") {
    std::cout << "connect" << std::endl;
    /** If the method is CONNECT: 
     * Setup the connection with target server
     * Reply a HTTP 200 OK 
     * Connect the Tunnel
    */
    log(std::string(to_string(uid) + ": Requesting \"" + request->return_request() + "\" from "+ request->return_Host() + "\n"));
    proxyCONNECT();
    log(std::string(to_string(uid) + ": Tunnel closed \n"));
    //Finish connect
    return;
  }
  else if (request->return_method() == "POST") {
    log(std::string(to_string(uid) + ": Requesting \"" + request->return_request() + "\" from "+ request->return_Host() + "\n"));
    proxyPOST();

    return;
  }
  else if(request->return_method() == "GET"){
    //Thre request is GET
    return;
  }else{
    //400
    return;
  }
}

int Proxy::connectServer() {
  int error;
  int socket_server;
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  error = getaddrinfo(
      request->return_Host().c_str(), request->return_port().c_str(), &hints, &res);
  if (error != 0) {
    std::cerr << "In connection: ";
    std::cerr << "Error: cannot get the address info from the host " << std::endl;
    log(std::string(to_string(uid) + ": ERROR cannot get the address info from the host \n"));
    pthread_exit(NULL);
  }
  socket_server = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (socket_server == -1) {
    std::cerr << "Error: cannot create socket for " << std::endl;
    log(std::string(to_string(uid) + ": ERROR cannot create socket for host" + request->return_Host() + "\n"));
    pthread_exit(NULL);
  }
  error = connect(socket_server, res->ai_addr, res->ai_addrlen);
  if (error == -1) {
    std::cerr << "Error cannot connect to the socket" << std::endl;
    log(std::string(to_string(uid) + ": ERROR cannot connect to the socket \n"));
    pthread_exit(NULL);
  }
  freeaddrinfo(res);
  return socket_server;
}

void Proxy::proxyCONNECT() {
  int socket_server = connectServer();
  fd_set listen_ports;
  int socket_client = this->return_socket_des();
  int error;
  error = send(socket_client, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
  if (error < 0) {
    cerr << "Cannot send back" << std::endl;
    log(std::string(to_string(uid) + ": ERROR cannot send back to client \n"));
    return;
  }
  log(std::string(to_string(uid) + ": Responding \"HTTP/1.1 200 OK\" \n"));
  int maxdes;
  // int socket_client = request->return_socket_des();
  if (socket_client > socket_server) {
    maxdes = socket_client;
  }
  else {
    maxdes = socket_server;
  }
  int end = 1;
  while (end) {
    FD_ZERO(&listen_ports);
    FD_SET(socket_client, &listen_ports);
    FD_SET(socket_server, &listen_ports);
    // temp_list = listen_ports;
    error = select(maxdes + 1, &listen_ports, NULL, NULL, NULL);
    if (error == -1) {
      std::cerr << "Error cannot select" << std::endl;
      log(std::string(to_string(uid) + ": ERROR cannot select \n"));
      pthread_exit(NULL);
    }
    for (int i = 0; i <= maxdes; i++) {
      if (FD_ISSET(i, &listen_ports)) {
        //There is a message received
        std::vector<char> data_buff = recvChar(i);
        if (data_buff.size() == 0) {
          //One of the socket closed on there side
          close(socket_server);
          close(socket_client);
          FD_ZERO(&listen_ports);
          end = 0;
        }
        else {
          // char * message = to_char(data_buff);
          if (i == socket_server) {
            //The message is from server
            send(socket_client, &data_buff.data()[0], data_buff.size(), 0);
          }
          else {
            //The message is from client
            send(socket_server, &data_buff.data()[0], data_buff.size(), 0);
          }
        }  //End of if for recv 0
      }    //End of if for check fd_set
    }      //End of go over the potential descriptor
  }        //main while loop
  return;  //End of func
}

void Proxy::proxyPOST(){
    int socket_server = connectServer();
    int socket_client = socket_des;
    send(socket_server,
         request->return_Line().c_str(),
         strlen(request->return_Line().c_str()),
         0);
    std::vector<char> input = recvChar(socket_server);
    if (input.size() == 0) {
      close(socket_client);
      close(socket_server);
      return;
    }
    send(socket_client, &input.data()[0], input.size(), 0);
    log(std::string(to_string(uid) + ": Responding \"HTTP/1.1 200 OK\" \n"));
    close(socket_client);
    close(socket_server);
}
