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
  std::string last_str;
  std::string no_cache_reason;
  time_t Date;
  time_t EXPIRES;
  int max_age;
  bool
      no_store;  //if 1, cannot cache(there is "no-store"), if 0 can cache(there is "no-cache" also set to 0)
  bool no_cache;
  bool if_chunk;

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
      last_str(),
      no_cache_reason(),
      Date(),
      EXPIRES(),
      max_age(),
      no_store(),
      if_chunk() {}
  http_Response(int sd, std::string l, std::vector<char> lr) :
      socket_server(sd),
      Line(l),
      line_recv(lr),
      http_ver(),
      statusCode(0),
      status(),
      cache_ctrl(""),
      etags(""),
      last_str(""),
      no_cache_reason(""),
      Date(0),
      EXPIRES(0),
      max_age(-1),

      no_store(false),
      no_cache(false),
      if_chunk(false) {
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
  std::string return_last_str() const { return last_str; }
  time_t return_date() const { return Date; }
  time_t return_expire() const { return EXPIRES; }
  int return_max() const { return max_age; }
  // int return_fresh() const { return freshness; }
  bool return_no_store() const { return no_store; }
  std::string return_no_cache_reason() const { return no_cache_reason; }
  bool return_no_cache() const { return no_cache; }
  std::string return_response() const {
    std::stringstream sstream;
    sstream << http_ver << " " << statusCode << " " << status;
    return sstream.str();
  }

  int parseResponse(std::vector<char> & lr) {
    httpparser::Response parsed_response;
    httpparser::HttpResponseParser parser;
    // httpparser::HttpResponseParser::ParseResult result = parser.parse(
    //     parsed_response, &line_recv.data()[0], &line_recv.data()[0] + line_recv.size());
    httpparser::HttpResponseParser::ParseResult result =
        parser.parse(parsed_response, &lr.data()[0], &lr.data()[0] + line_recv.size());
    if (result == httpparser::HttpResponseParser::ParsingCompleted) {
      status = parsed_response.status;
      statusCode = parsed_response.statusCode;
      std::stringstream sstream;
      sstream << "HTTP/" << parsed_response.versionMajor << "."
              << parsed_response.versionMinor;
      http_ver = sstream.str();
      get_cache_expire(parsed_response);
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
        if (findNumber("max-age=") >= 0) {
          max_age = findNumber("max-age=");
          std::cout << max_age << std::endl;
        }

        size_t no_store_start = cache_ctrl.find("no-store");
        if (no_store_start != std::string::npos) {
          std::string no_store_str = cache_ctrl.substr(no_store_start, 8);
          no_cache_reason += no_store_str;
          no_store = true;
        }
        size_t no_cache_start = cache_ctrl.find("no-cache");
        if (no_cache_start != std::string::npos) {
          no_cache = true;
        }
        size_t must_valid_start = cache_ctrl.find("must-revalidate");
        if (must_valid_start != std::string::npos) {
          no_cache = true;
        }
        size_t private_start = cache_ctrl.find("private");
        if (private_start != std::string::npos) {
          std::string private_str = cache_ctrl.substr(private_start, 7);
          if (no_cache_reason.size() != 0) {
            no_cache_reason += " ";
          }
          no_cache_reason += private_str;
          no_store = true;
        }
      }
      else if (it->name == "Expires") {
        if (it->value == "-1") {
          EXPIRES = -1;
        }
        else {
          EXPIRES = stringTotime(it->value);
        }
      }
      else if (it->name == "Date") {
        // Date = stringTotime(it->value);
        time_t currTime;
        currTime = time(0);
        Date = currTime;
      }
      else if (it->name == "Last-Modified") {
        last_str = it->value;
      }
      else if (it->name == "ETag") {
        etags = it->value;
      }
      else if (it->name == "Transfer-Encoding") {
        std::string encoding = it->value;
        size_t chunked_start = encoding.find("chunked");
        if (chunked_start != std::string::npos) {
          if_chunk = true;
        }
      }
    }
    return 0;
  }

  int findNumber(std::string header) {
    size_t filed_start = cache_ctrl.find(header.c_str());
    if (filed_start != std::string::npos) {
      std::string filed_whole = cache_ctrl.substr(filed_start + header.size());
      size_t filed_end = filed_whole.find(",");
      if (filed_end == std::string::npos) {
        filed_end = filed_whole.find("\r\n");
      }
      int return_value = stoi(filed_whole.substr(0, filed_end));
      return return_value;
    }
    return -1;
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