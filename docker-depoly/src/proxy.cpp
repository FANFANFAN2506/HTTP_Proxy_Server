#include "request.hpp"

void * runProxy(void * myProxy) {
}

void requestAccept() {
  /***Below should be passed in as argument***/
  long uid;
  int sd;
  std::string ip;
  /*End of argument passed in*/
  std::string Line = recvAll(sd);
  requestHandle(uid, sd, Line, ip);
}

void requestHandle(long uid, int sd, std::string l, std::string ip) {
  std::string Received_time = get_current_Time();
  http_Request * Request_instance = new http_Request(uid, sd, l, ip, Received_time);
  if (Request_instance->return_method().c_str() == "CONNECT") {
    //The request is CONNECT
  }
  else if (Request_instance->return_method().c_str() == "POST") {
    //The request is POST
  }
  else {
    //Thre request is GET
  }
}