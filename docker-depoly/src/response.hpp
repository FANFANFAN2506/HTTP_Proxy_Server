#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <ctime>
#include <iostream>
#include <vector>

#include "httpparser/httpresponseparser.h"
#include "httpparser/response.h"

class http_Response {
 private:
  int socket_server;
  std::string Line;
  std::string http_ver;
  unsigned int statusCode;
  std::string status;
  std::string cache_ctrl;
  std::string etags;
  time_t EXPIRES;

 public:
  http_Response():
      socket_server(0),
      Line(),
      http_ver(),
      statusCode(0),
      status(),
      cache_ctrl(),
      EXPIRES() {}
  http_Response(int sd, std::string l):
      socket_server(sd),
      Line(l),
      http_ver(),
      statusCode(),
      status(),
      cache_ctrl(),
      EXPIRES() {
    //Get the http_ver,status,statuscode
    //get cache_Control expires
    parseResponse();
  }
  int return_socket() const { return socket_server; }
  std::string return_line() const { return Line; }
  std::string return_httpver() const { return http_ver; }
  unsigned int return_statuscode() const { return statusCode; }
  std::string return_status() const { return status; }
  std::string return_cache_ctrl() const { return cache_ctrl; }
  std::string return_etags() const { return etags; }
  time_t return_expire() const { return EXPIRES; }
  std::string return_response() const {
    std::stringstream sstream;
    sstream << http_ver << " " << statusCode << " " << status << "\r\n";
    return sstream.str();
  }
  
  void parseResponse() {
    httpparser::Response parsed_response;
    httpparser::HttpResponseParser parser;
    const char * line = this->Line.c_str();
    httpparser::HttpResponseParser::ParseResult result =
        parser.parse(parsed_response, line, line + strlen(line));
    if (result == httpparser::HttpResponseParser::ParsingCompleted) {
      // std::cout << parsed_response.inspect() << std::endl;
      status = parsed_response.status;
      statusCode = parsed_response.statusCode;
      std::stringstream sstream;
      sstream << "HTTP/" << parsed_response.versionMajor << "."
              << parsed_response.versionMinor;
      http_ver = sstream.str();
      get_cache_expire(parsed_response);
      return;
    }
    else {
      std::cerr << "Parsing failed" << std::endl;
      return;
    }
  }

  void get_cache_expire(httpparser::Response & parsed_response) {
    std::vector<httpparser::Response::HeaderItem>::const_iterator it;
    for (it = parsed_response.headers.begin(); it != parsed_response.headers.end();
         ++it) {
      if (it->name == "Cache-Control") {
        cache_ctrl = it->value;
      }
      else if (it->name == "Expires") {
        std::string expire_time = it->value;
        struct tm tm;
        memset(&tm, 0, sizeof(tm));
        const char * format = "%a, %e %h %Y %X";
        char * res = strptime(expire_time.c_str(), format, &tm);
        if (res == nullptr) {
          std::cerr << "wrong time conversion" << std::endl;
        }
        time_t ti = mktime(&tm);
        EXPIRES = ti;
      }
    }
  }
};