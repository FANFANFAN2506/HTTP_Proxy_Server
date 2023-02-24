#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <cassert>
#include <ctime>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "httpparser/httprequestparser.h"
#include "httpparser/request.h"

class http_Request {
 private:
  // long UID;                 //UID unique id for log
  int socket_des;       //socket_des descriptor
  std::string Line;     //Whole contents of the request
  std::string REQUEST;  //Request line(first line)
  std::string uri;
  std::string header_data;  //For CONNECT method remove the request line
  std::string IPFROM;       //Client IP
  std::string Method;
  std::string http_ver;
  std::string Host_name;  //Target server IP
  std::string Host_port;  //Target server port
  std::string requestLine;
  std::vector<char> line_send;
  time_t TIME;

 public:
  //default constructor
  http_Request() :
      socket_des(0),
      Line(),
      REQUEST(),
      uri(),
      header_data(),
      IPFROM(),
      Method(),
      http_ver(),
      Host_name(),
      Host_port(),
      line_send(),
      TIME() {}
  http_Request(int sd, std::string l, std::vector<char> & ls, std::string ip, time_t t) :
      socket_des(sd),
      Line(l),
      REQUEST(),
      uri(),
      header_data(),
      IPFROM(ip),
      Method(),
      http_ver(),
      Host_name(),
      Host_port(),
      line_send(ls),
      TIME(t) {
    //Get uri and method
  }
  // long return_UID() const { return UID; }
  int return_socket_des() const { return socket_des; }
  std::string return_Line() const { return Line; }
  std::string return_request() const { return REQUEST; }
  std::string return_uri() const { return uri; }
  std::string return_header() const { return header_data; }
  std::string return_ip() const { return IPFROM; }
  std::string return_method() const { return Method; }
  std::string return_httpver() const { return http_ver; }
  std::string return_Host() const { return Host_name; }
  std::string return_port() const { return Host_port; }
  std::vector<char> return_line_send() const { return line_send; }
  time_t return_time() const { return TIME; }
  int constructRequest() {
    int er1;
    er1 = parseRequest();
    if (er1 == -1) {
      return -1;
    }
    // getHost();
    getRequestLine();
    return 0;
  }
  int parseRequest() {
    httpparser::Request parsed_request;
    httpparser::HttpRequestParser parser;
    // const char * line = this->Line.c_str();
    // httpparser::HttpRequestParser::ParseResult parsed_result =
    //     parser.parse(parsed_request, line, line + strlen(line));
    httpparser::HttpRequestParser::ParseResult parsed_result = parser.parse(
        parsed_request, &line_send.data()[0], &line_send.data()[0] + line_send.size());
    if (parsed_result == httpparser::HttpRequestParser::ParsingCompleted) {
      // std::cout << parsed_request.inspect() << std::endl;
      Method = parsed_request.method;
      uri = parsed_request.uri;
      std::stringstream sstream;
      sstream << "HTTP/" << parsed_request.versionMajor << "."
              << parsed_request.versionMinor;
      http_ver = sstream.str();
      if (getHostIp(parsed_request) < 0) {
        return -1;
      }
      //Host name + Host_port + uri
      uri = Host_name + Host_port + uri;
      // }
      return 0;
    }
    else {
      std::cerr << "request parsing fail" << std::endl;
      return -1;
    }
  }
  int getHostIp(httpparser::Request & parsed_request) {
    std::vector<httpparser::Request::HeaderItem>::const_iterator it;
    for (it = parsed_request.headers.begin(); it != parsed_request.headers.end(); ++it) {
      if (it->name == "Host") {
        std::string host_line = it->value;
        size_t port_start = host_line.find_first_of(":\r");
        //if we allowed it to declare port
        //maybe the we could hard code as 80
        if (port_start != std::string::npos) {
          //the port is declared
          Host_name = host_line.substr(0, port_start);
          Host_port = host_line.substr(port_start + 1);
        }
        else {
          //not declared port; default 80 port for HTTP/TCP
          Host_name = host_line;
          Host_port = "80";
        }
        return 0;
      }
    }
    return -1;
  }
  void getRequestLine() {
    size_t request_line_end = Line.find_first_of("\r\n");
    REQUEST = Line.substr(0, request_line_end);
    header_data = Line.substr(request_line_end + 2);
  }
};