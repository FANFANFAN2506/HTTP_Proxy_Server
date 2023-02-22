#include "request.hpp"

#include <vector>

#include "response.hpp"
#include "utils.hpp"
/*This file is for testing*/
int main(void) {
  std::vector<std::string> result;
  std::string r1 = "GET /uri.cgi HTTP/1.1\r\nUser-Agent: Mozilla/5.0\r\nAccept: "
                   "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
                   "*;q=0.8\r\nHost: 127.0.0.1\r\n\r\n";
  std::string r3 = "HTTP/1.1 200 OK\r\n"
                   "Server: nginx/1.2.1\r\n"
                   "Content-Type: text/html\r\n"
                   "Content-Length: 8\r\n"
                   "Connection: keep-alive\r\n"
                   "Cache-Control: private, max-age=0\r\n"
                   "Expires: Thu, 01 Dec 1994 16:00:00 GMT\r\n"
                   "\r\n"
                   "<html />";
  http_Response h2 = http_Response(0, r3);
  std::cout << "Whole line " << h2.return_line() << std::endl;
  std::cout << "Request LIne " << h2.return_response() << std::endl;
  std::cout << "Http version: " << h2.return_httpver() << std::endl;
  std::cout << "Cache control: " << h2.return_cache_ctrl() << std::endl;
  std::cout << "Expired Time: " << parseTime(h2.return_expire()) << std::endl;
  time_t curr_time;
  time(&curr_time);
  http_Request h1 = http_Request(0, r1, "", curr_time);
  std::cout << "Recevied Time: " << parseTime(h1.return_time()) << std::endl;
  std::cout << "Parsed method " << h1.return_method() << std::endl;
  std::cout << "URI: " << h1.return_uri() << std::endl;
  std::cout << "Http version: " << h1.return_httpver() << std::endl;
  std::cout << "Parsed Host: " << h1.return_Host() << std::endl;
  std::cout << "Header and conetent:" << h1.return_header() << std::endl;
  // std::string strtime = "Thu, 1 Dec 1994 16:00:00 GMT";
  // struct tm tm;
  // memset(&tm, 0, sizeof(tm));
  // const char * format = "%a, %e %h %Y %X";
  // char * res = strptime(strtime.c_str(), format, &tm);
  // if (res == nullptr) {
  //   std::cerr << "wrong time conversion" << std::endl;
  // }
  // time_t ti = mktime(&tm);
  // std::cout << "Time is " << parseTime(ti) << std::endl;
  // if (ti < h1.return_time()) {
  //   std::cout << "Expired" << std::endl;
  // }
  // else {
  //   std::cout << "valid" << std::endl;
  // }

  return 0;
}