#include "proxy.hpp"

#include <unistd.h>

#include <memory>

#include "log.hpp"
#include "utils.hpp"

#define PORT "12345"
#define MAXPENDING 10
#define MAXCachingCapacity 100
long requestID = 0;
pthread_mutex_t logLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cacheLock = PTHREAD_MUTEX_INITIALIZER;
Cache * myCache = new Cache(MAXCachingCapacity);
/**
 * 1. std::thread 问了，等回复
 * 2. 502 error: 方法写好了，还没添加到各部分的接受response阶段，error是不是直接退thread更方便
 * 3. cache lock 记得加
 * 4. 一些ico的 parse error问题
 * 5. cache expire： 一些Expire和Max-age为空的情况怎么办, receivedTime肯定不为0，对于一些网站，Expire 为 -1, Max-age 为 0 
 *   （每次都要重请求的意思，这里我直接算特殊case return掉了），在Response里俩都被初始化成0了，所以会出现Max-age为空的网站还要重请求的情况。
 * 6. cache valid 
 * 7. cache update 写好了，在put的时候会直接update，但是记得要log
 * 8. no-store 的解析问题
*/

/**
 * Reference from Beej's tutorial
*/
void proxyListen() {
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list, *a;
  const char * hostname = "0.0.0.0";
  const char * port = PORT;

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

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
      Proxy * myProxy = new Proxy(requestID, client_connection_fd, ip, myCache);
      pthread_create(&thread, NULL, runProxy, myProxy);
      //pthread_join(thread,NULL);
      requestID++;
    }
  }
  return;
}

void * runProxy(void * myProxy) {
  Proxy * Proxy_instance = (Proxy *)myProxy;
  try {
    // std::vector<char> line_send = recvChar(Proxy_instance->return_socket_des());
    std::vector<char> line_send = recvBuff(Proxy_instance->return_socket_des());
    std::string Line(line_send.begin(), line_send.end());
    // std::vector<char> data_buff(65536, 0);
    // int data_rec = 0;
    // data_rec = recv(Proxy_instance->return_socket_des(), &data_buff.data()[0], 65536, 0);
    // if (data_rec <= 0) {
    //   std::cerr << "cannot receive request" << std::endl;
    //   throw std::exception();
    // }
    // // std::cout << "recevied length is:" << data_rec << std::endl;
    // data_buff.resize(data_rec);
    // std::string Line(data_buff.begin(), data_buff.end());
    // std::cout << "Request is " << Line << std::endl;
    Proxy_instance->setRequest(Line, line_send);
    Proxy_instance->judgeRequest();
    delete Proxy_instance;
  }
  catch (std::exception & e) {
    delete Proxy_instance;
    std::cerr << e.what() << std::endl;
  }
  // Proxy_instance->destructProxy();
  return NULL;
}

//Proxy memeber functions:

void Proxy::setRequest(std::string Line, std::vector<char> & line_send) {
  // std::string Received_time = get_current_Time();
  int error;
  time_t curr_time;
  time(&curr_time);
  request =
      new http_Request(this->socket_des, Line, line_send, this->clientIP, curr_time);
  error = request->constructRequest();
  if (error == -1) {
    delete request;
    std::cerr << "Request construction failed" << std::endl;
    log(std::string(to_string(uid) + ": ERROR request construction failed \n"));
    proxyERROR(400);
    pthread_exit(0);
  }
  std::string msg = to_string(uid) + ": \"" + request->return_request() + "\" from " +
                    request->return_ip() + " @ " + parseTime(request->return_time());
  log(msg);
}

void Proxy::judgeRequest() {
  if (request->return_method() == "CONNECT") {
    std::cout << "connect" << std::endl;
    /** If the method is CONNECT: 
     * Setup the connection with target server
     * Reply a HTTP 200 OK 
     * Connect the Tunnel
    */
    log(std::string(to_string(uid) + ": Requesting \"" + request->return_request() +
                    "\" from " + request->return_Host() + "\n"));
    proxyCONNECT();
    log(std::string(to_string(uid) + ": Tunnel closed \n"));
    //Finish connect
    return;
  }
  else if (request->return_method() == "POST") {
    log(std::string(to_string(uid) + ": Requesting \"" + request->return_request() +
                    "\" from " + request->return_Host() + "\n"));
    proxyPOST();

    return;
  }
  else if (request->return_method() == "GET") {
    proxyGET();
    return;
  }
  else {
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
    log(std::string(to_string(uid) +
                    ": ERROR cannot get the address info from the host \n"));
    proxyERROR(404);
    pthread_exit(NULL);
  }
  socket_server = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (socket_server == -1) {
    std::cerr << "Error: cannot create socket for " << std::endl;
    log(std::string(to_string(uid) + ": ERROR cannot create socket for host" +
                    request->return_Host() + "\n"));
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
        // std::vector<char> data_buff = recvChar(i);
        std::vector<char> data_buff = recvBuff(i);
        int length = data_buff.size();
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
            // send(socket_client, &data_buff.data()[0], data_buff.size(), 0);
            sendall(socket_client, &data_buff.data()[0], &length);
          }
          else {
            //The message is from client
            // send(socket_server, &data_buff.data()[0], data_buff.size(), 0);
            sendall(socket_server, &data_buff.data()[0], &length);
          }
        }  //End of if for recv 0
      }    //End of if for check fd_set
    }      //End of go over the potential descriptor
  }        //main while loop
  return;  //End of func
}

void Proxy::proxyPOST() {
  int socket_server = connectServer();
  int socket_client = socket_des;
  std::vector<char> send_request = request->return_line_send();
  http_Response * Proxy_temp = proxyFetch(socket_server, socket_client);
  if (Proxy_temp != NULL) {
    delete Proxy_temp;
  }
  close(socket_client);
  close(socket_server);
}

void Proxy::proxyGET() {
  /**
   * 1. Search from cache
   * 2. Check Expire and validation
   * 3. Send validation to server
   * 4. If good: send cached response back
   *    else: send the request, and get response update and reply with the response
  */
  int socket_client = socket_des;
  std::string request_url = request->return_uri();
  //search from the cache
  std::cout << "the request url is " << request_url << std::endl;
  http_Response * response_instance = myCache->get(request_url);
  if (response_instance == NULL) {
    //no response in cache
    std::cout << "not in cache" << std::endl;
    log(std::string(to_string(uid) + ": not in cache\n"));
    int socket_server = connectServer();
    std::cout << "connected to server" << std::endl;
    response_instance = proxyFetch(socket_server, socket_client);
    if (response_instance && response_instance->return_statuscode() == 200) {
      //if the response is 200 we need to cache it
      std::string removed_node = myCache->put(request_url, response_instance);
      //std::cout << "@@@@@" <<cache->get(request_url)->return_response() << std::endl;
      if (removed_node.size() != 0) {
        //There is a node being removed, need to log
        log(std::string("(no-id): NOTE evicted" + removed_node + "from cache"));
      }
    }
    //No need to cache
  }
  else {
    //If we find this in the cache;
    std::cout << "Find in cache" << std::endl;
    //Expiration function
    bool if_expire = cache->checkExpire(request_url);
    if (if_expire) {
      //Expired cached, need update
      HandleValidation(response_instance, request_url);
      return;
    }
    else {
      //Valid, but need validation
      bool if_no_cache = request->return_no_cache();
      if (if_no_cache) {
        //must validation
        HandleValidation(response_instance, request_url);
        return;
      }
      else {
        //no validation required by request, send it back
        log(std::string(to_string(uid) + ": in cache, valid\n"));
        std::vector<char> reply = response_instance->return_line_recv();
        // send(socket_client, &reply.data()[0], reply.size(), 0);
        int length = reply.size();
        sendall(socket_client, &reply.data()[0], &length);
        close(socket_client);
        return;
      }
    }
  }
}

http_Response * Proxy::proxyFetch(int socket_server, int socket_client) {
  /**
 * Get the request, and send to the server
 * Get the response from the server
 * Send the response back to the client
 * return the parsed response object pointer
*/
  std::vector<char> send_request = request->return_line_send();
  http_Response * r1 = NULL;
  int length = send_request.size();
  // std::cout << "send starts" << std::endl;
  int error = sendall(socket_server, &send_request.data()[0], &length);
  // std::cout << "send ends" << error << std::endl;
  // if (send(socket_server, &send_request.data()[0], send_request.size(), 0) > 0) {
  if (error == 0) {
    // std::vector<char> input = recvChar(socket_server);
    std::vector<char> input = recvBuff(socket_server);
    std::string response_test(input.begin(), input.end());
    // std::cout << "response test is :" << response_test << std::endl;
    if (check502(input)) {
      proxyERROR(502);
      pthread_exit(0);
    }
    if (chunkHandle(input, socket_server)) {
      return NULL;
    }
    if (input.size() != 0) {
      int length = input.size();
      int error = sendall(socket_client, &input.data()[0], &length);
      // if (send(socket_client, &input.data()[0], input.size(), 0) > 0) {
      if (error == 0) {
        std::string reply(input.begin(), input.end());
        r1 = new http_Response(socket_server, reply, input);
        int error = r1->parseResponse(input);
        if (error == -1) {
          //error in constructing Reponse;
          //502 error code
          return NULL;
        }
        else {
          // log(std::string(to_string(uid) + ": Responding \"HTTP/1.1 200 OK\" \n"));
          log(std::string(to_string(uid) + ": Responding \"" + r1->return_response() +
                          "\"\n"));
          return r1;
        }
      }
    }
  }
  return r1;
}

void Proxy::proxyERROR(int code) {
  int client_fd = socket_des;
  // string resp;
  const char * resp;
  switch (code) {
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
  string logLine = resp;
  send(client_fd, resp, strlen(resp), 0);
  log(std::string(to_string(uid) + ": Responding \"" +
                  logLine.substr(0, logLine.size() - 4) + "\"\n"));
  close(client_fd);
}

bool Proxy::chunkHandle(vector<char> & input, int server_fd) {
  // std::string str = char_to_string(input);
  std::string str(input.begin(), input.end());
  size_t start = str.find("Transfer-Encoding:");
  if (start == std::string::npos) {
    return false;
  }
  size_t end = str.find("\n", start);
  std::string encodeLine = str.substr(start, end);
  if (encodeLine.find("chunked") == std::string::npos) {
    return false;
  }
  int length = input.size();
  sendall(socket_des, &input.data()[0], &length);
  // send(socket_des, &input.data()[0], input.size(), 0);
  while (1) {
    std::vector<char> chunkMsg = recvBuff(server_fd);
    // std::vector<char> chunkMsg = recvChar(server_fd);
    if (chunkMsg.size() <= 0) {
      break;
    }
    int length = chunkMsg.size();
    sendall(socket_des, &chunkMsg.data()[0], &length);
    // send(socket_des, &chunkMsg.data()[0], chunkMsg.size(), 0);
  }
  return true;
}

bool Proxy::check502(vector<char> & input) {
  std::string str(input.begin(), input.end());
  if (str.find("\r\n\r\n") == std::string::npos) {
    return true;
  }
  return false;
}

std::vector<char> Proxy::ConstructValidation(http_Response * response_instance) {
  std::string request_line = request->return_request();
  request_line.append("\r\n");
  time_t last_modified = response_instance->return_last();
  if (last_modified != 0) {
    request_line += "If-Modified-Since: ";
    request_line += parseTime(last_modified);
    request_line += "\r\n";
  }
  std::string etags_response = response->return_etags();
  std::cout << "etags is " << etags_response << std::endl;
  if (etags_response.size() != 0) {
    request_line += "If-None-Match: ";
    request_line += etags_response;
    request_line += "\r\n";
  }
  request_line += "\r\n";
  if (last_modified != 0 || etags_response.size() != 0) {
    std::vector<char> reply(request_line.begin(), request_line.end());
    return reply;
  }
  else {
    //shouldn't miss two fields
    std::vector<char> reply;
    return reply;
  }
}

void Proxy::HandleValidation(http_Response * response_instance, std::string request_url) {
  int socket_client = socket_des;
  log(std::string(to_string(uid) + ": in cache, but expired at " +
                  parseTime(response_instance->return_expire()) + "\n"));
  //resend the request, cache update
  int socket_server = connectServer();
  std::vector<char> revalid_request = ConstructValidation(response_instance);
  int length = revalid_request.size();
  sendall(socket_server, &revalid_request.data()[0], &length);
  // send(socket_server, &revalid_request.data()[0], revalid_request.size(), 0);
  //reply with the new response
  // std::vector<char> reply = recvChar(socket_server);
  std::vector<char> reply = recvBuff(socket_server);
  std::string reply_str(reply.begin(), reply.end());
  http_Response * new_response = new http_Response(socket_server, reply_str, reply);
  int error = new_response->parseResponse(reply);
  if (error == -1) {
    //error in constructing Reponse;
    //502 error code
    std::cerr << "Reponse parsing fail" << std::endl;
  }
  else {
    log(std::string(to_string(uid) + ": Responding \"" + new_response->return_response() +
                    "\"\n"));
    if (new_response->return_statuscode() == 200) {
      //update and return to client
      std::string removed_url = cache->put(request_url, new_response);
      if (removed_url.size() != 0) {
        log(std::string("(no-id): NOTE evicted" + removed_url + "from cache"));
      }
      int length = reply.size();
      sendall(socket_client, &reply.data()[0], &length);
      // send(socket_client, &reply.data()[0], reply.size(), 0);
    }
    else if (new_response->return_statuscode() == 304) {
      //not modified return the cached response
      std::vector<char> cached_response = response_instance->return_line_recv();
      int length = cached_response.size();
      sendall(socket_client, &cached_response.data()[0], &length);
      // send(socket_client, &cached_response.data()[0], cached_response.size(), 0);
    }
  }
  close(socket_server);
  close(socket_client);
}

/**SendAll
 * Reference from Beej's tutorial
 * https://beej.us/guide/bgnet/html//index.html#recvman
*/
int Proxy::sendall(int s, char * buf, int * len) {
  int total = 0;         // how many bytes we've sent
  int bytesleft = *len;  // how many we have left to send
  int n;

  while (total < *len) {
    n = send(s, buf + total, bytesleft, 0);
    if (n == -1) {
      break;
    }
    total += n;
    bytesleft -= n;
  }

  *len = total;  // return number actually sent here

  return n == -1 ? -1 : 0;  // return -1 on failure, 0 on success
}