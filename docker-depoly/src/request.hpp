#include <cassert>
#include <exception>
#include <sstream>
#include <string>

#include "httpparser/httprequestparser.h"
#include "httpparser/request.h"
#include "utils.hpp"

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
  std::string TIME;

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
      TIME() {}
  http_Request(int sd, std::string l, std::string ip, std::string t) :
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
      TIME(t) {
    //Get uri and method
    parseRequest();
    getHost();
    //getMethod();
    getRequestLine();
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
  std::string return_time() const { return TIME; }
  void parseRequest() {
    httpparser::Request parsed_request;
    httpparser::HttpRequestParser parser;
    const char * line = this->Line.c_str();
    // const char * line = "GET /uri.cgi HTTP/1.1\r\nUser-Agent: Mozilla/5.0\r\nAccept: "
    //                     "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
    //                     "*;q=0.8\r\nHost: 127.0.0.1\r\n\r\n";
    httpparser::HttpRequestParser::ParseResult parsed_result =
        parser.parse(parsed_request, line, line + strlen(line));
    if (parsed_result == httpparser::HttpRequestParser::ParsingCompleted) {
      // std::cout << parsed_request.inspect() << std::endl;
      Method = parsed_request.method;
      uri = parsed_request.uri;
      std::stringstream sstream;
      sstream << "HTTP/" << parsed_request.versionMajor << "."
              << parsed_request.versionMinor;
      http_ver = sstream.str();
      return;
    }
    else {
      std::stringstream sstream;
      std::cout << "The proxy cannot understand" << std::endl;
      return;
    }
  }
  void getMethod() {
    try {
      size_t start_line = Line.find_first_of(" ");
      Method = Line.substr(0, start_line);
      std::string get = "GET";
      std::string post = "POST";
      std::string connect = "CONNECT";
      if (Method != get && Method != connect && Method != post) {
        assert(Method.c_str() != get);
        std::cout << "Illegal method is " << Method.c_str() << std::endl;
        throw std::invalid_argument("Only support GET POST CONNECT methods");
      }
    }
    catch (std::exception & e) {
      std::stringstream sstream;
      sstream << "Bad request" << e.what() << std::endl;
      std::cout << "The proxy cannot understand" << e.what() << std::endl;
      const char * message = sstream.str().c_str();
      send(socket_des, &message, strlen(message), 0);
      return;
    }
  }
  void getHost() {
    try {
      size_t host_start = Line.find("Host: ");
      std::string host_name = Line.substr(host_start + 6);
      size_t host_end = host_name.find_first_of("\r\n");
      if (host_start == std::string::npos || host_end == std::string::npos) {
        throw std::invalid_argument("Cannot find the host name");
      }
      std::string Host = host_name.substr(0, host_end);
      size_t port_start = Host.find_first_of(":\r");
      //if we allowed it to declare port
      //maybe the we could hard code as 80
      if (port_start != std::string::npos) {
        //the port is declared
        Host_name = host_name.substr(0, port_start);
        Host_port = Host.substr(port_start + 1);
      }
      else {
        //not declared port; default 80 port for HTTP/TCP
        Host_name = Host;
        Host_port = "80";
      }
    }
    catch (std::exception & e) {
      std::stringstream sstream;
      sstream << "Bad request" << e.what() << std::endl;
      std::cout << "The proxy cannot understand" << e.what() << std::endl;
      const char * message = sstream.str().c_str();
      send(socket_des, &message, strlen(message), 0);
      return;
    }
  }
  void getRequestLine() {
    try {
      size_t request_line_end = Line.find_first_of("\r\n");
      if (request_line_end == std::string::npos) {
        throw std::invalid_argument("Wrong request line format");
      }
      REQUEST = Line.substr(0, request_line_end);
      header_data = Line.substr(request_line_end + 2);
    }
    catch (std::exception & e) {
      std::stringstream sstream;
      sstream << "Bad request" << e.what() << std::endl;
      std::cout << "The proxy cannot understand" << e.what() << std::endl;
      const char * message = sstream.str().c_str();
      send(socket_des, &message, strlen(message), 0);
      return;
    }
  }
};