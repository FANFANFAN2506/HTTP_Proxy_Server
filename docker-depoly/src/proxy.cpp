#include <unistd.h>
#include <memory>

#include "utils.hpp"
#include "proxy.hpp"
#include "log.hpp"

#define PORT "12345"
#define MAXPENDING 10
#define MAXCachingCapacity 100
long requestID = 0;
pthread_mutex_t logLock = PTHREAD_MUTEX_INITIALIZER;

void proxyListen(){
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list, *a;
  const char * hostname = "0.0.0.0";
  const char * port = PORT;

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    log(std::string("(no-id): ERROR request construction failed \n"));
    return;
  }
  for (a = host_info_list; a != NULL; a = a->ai_next) {
    socket_fd = socket(a->ai_family, a->ai_socktype, a->ai_protocol);
    if (socket_fd == -1) {
      log(std::string("(no-id): ERROR cannot create socket \n"));
      continue;
    }

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, a->ai_addr, a->ai_addrlen);
    if (status == -1) {
      close(socket_fd);
      log(std::string("(no-id): ERROR cannot bind socket \n"));
      continue;
    }
    break;
  }
  if (a == NULL) {
    log(std::string("(no-id): ERROR selectserver failed to bind \n"));
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(host_info_list);
  status = listen(socket_fd, MAXPENDING);
  if (status == -1) {
    log(std::string("(no-id): ERROR request construction failed \n"));
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return;
  }

  Cache * cache = new Cache(MAXCachingCapacity);
  cout << "Waiting for connection on port " << port << endl;
  struct sockaddr_in socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd;
  while (1) {
    client_connection_fd =
        accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
      log("(no-id): Failed to estblish connection with client");
    }
    else {
      string ip = inet_ntoa(socket_addr.sin_addr);
      pthread_t thread;
      //This pointer may need to be considered for RAII0
      Proxy * myProxy = new Proxy(requestID, client_connection_fd, ip, cache);
      pthread_create(&thread, NULL, runProxy, myProxy);
      requestID++;
    }
  }
  return;
}

//异常处理(对于一些不存在的域名，不需要pending)，跨方法 log, Response 解析
void * runProxy(void * myProxy) {
  Proxy * Proxy_instance = (Proxy *)myProxy;

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
    std::cout << request->return_request() << std::endl;
    proxyERROR(404);
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
    proxyERROR(404);
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

void Proxy::proxyERROR(int code){
  int clinet_fd = socket_des;
  string resp;
  switch(code){
    case 404:
      resp = "HTTP/1.1 404 Not Found\r\n\r\n";
      break;
    case 400:
      resp = "HTTP/1.1 400 Bad Request\r\n\r\n";
      break;
    case 502:
      resp = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
      break;
  }
  send(clinet_fd, resp.c_str(), resp.length(), 0);
}