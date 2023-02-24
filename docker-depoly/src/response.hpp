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
  std::vector<char> line_recv;
  std::string http_ver;
  unsigned int statusCode;
  std::string status;
  std::string cache_ctrl;
  std::string etags;
  time_t Date;
  time_t EXPIRES;
  time_t Lastmodified;
  int max_age;

 public:
  http_Response() :
      socket_server(0),
      Line(),
      line_recv(),
      http_ver(),
      statusCode(0),
      status(),
      cache_ctrl(),
      etags(),
      Date(),
      EXPIRES(),
      Lastmodified(),
      max_age() {}
  http_Response(int sd, std::string l, std::vector<char> lr) :
      socket_server(sd),
      Line(l),
      line_recv(lr),
      http_ver(),
      statusCode(0),
      status(),
      cache_ctrl(""),
      etags(""),
      Date(0),
      EXPIRES(0),
      Lastmodified(0),
      max_age(0) {
    //Get the http_ver,status,statuscode
    //get cache_Control expires
  }
  int return_socket() const { return socket_server; }
  std::string return_line() const { return Line; }
  std::string return_httpver() const { return http_ver; }
  std::vector<char> return_line_recv() const { return line_recv; }
  unsigned int return_statuscode() const { return statusCode; }
  std::string return_status() const { return status; }
  std::string return_cache_ctrl() const { return cache_ctrl; }
  std::string return_etags() const { return etags; }
  time_t return_date() const { return Date; }
  time_t return_expire() const { return EXPIRES; }
  time_t return_last() const { return Lastmodified; }
  int return_max() const { return max_age; }
  std::string return_response() const {
    std::stringstream sstream;
    sstream << http_ver << " " << statusCode << " " << status;
    return sstream.str();
  }

  int parseResponse() {
    httpparser::Response parsed_response;
    httpparser::HttpResponseParser parser;
    httpparser::HttpResponseParser::ParseResult result = parser.parse(
        parsed_response, &line_recv.data()[0], &line_recv.data()[0] + line_recv.size());
    if (result == httpparser::HttpResponseParser::ParsingCompleted) {
      status = parsed_response.status;
      statusCode = parsed_response.statusCode;
      std::cout << "!!!!" << statusCode << std::endl;
      std::stringstream sstream;
      sstream << "HTTP/" << parsed_response.versionMajor << "."
              << parsed_response.versionMinor;
      http_ver = sstream.str();
      int error = get_cache_expire(parsed_response);
      if (error == -1) {
        return -1;
      }
      return 0;
    }
    else {
      std::cerr << "Parsing failed" << std::endl;
      return -1;
    }
  }

  int get_cache_expire(httpparser::Response & parsed_response) {
    std::vector<httpparser::Response::HeaderItem>::const_iterator it;
    for (it = parsed_response.headers.begin(); it != parsed_response.headers.end();
         ++it) {
      if (it->name == "Cache-Control") {
        cache_ctrl = it->value;
        size_t max_age_start = cache_ctrl.find("max-age=");
        if (max_age_start != std::string::npos) {
          std::string max_age_whole = cache_ctrl.substr(max_age_start + 8);
          size_t max_age_end = max_age_whole.find(",");
          if (max_age_end == std::string::npos) {
            max_age_end = max_age_whole.find("\r\n");
          }
          max_age = stoi(max_age_whole.substr(0, max_age_end));
        }
      }
      else if (it->name == "Expires") {
        EXPIRES = stringTotime(it->value);
      }
      else if (it->name == "Date") {
        Date = stringTotime(it->value);
      }
      else if (it->name == "Last-Modified") {
        Lastmodified = stringTotime(it->value);
      }
      else if (it->name == "ETag") {
        etags = it->value;
      }
    }
    return 0;
  }
  time_t stringTotime(std::string time_str) {
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    const char * format = "%a, %e %h %Y %X";
    char * res = strptime(time_str.c_str(), format, &tm);
    if (res == nullptr) {
      std::cerr << "wrong time conversion" << std::endl;
      return -1;
    }
    time_t ti = mktime(&tm);
    return ti;
  }
};