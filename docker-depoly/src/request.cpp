#include "request.hpp"

#include <vector>
/*This file is for testing*/
int main(void) {
  std::vector<std::string> result;
  std::string r1 = "GET /uri.cgi HTTP/1.1\r\nUser-Agent: Mozilla/5.0\r\nAccept: "
                   "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
                   "*;q=0.8\r\nHost: 127.0.0.1\r\n\r\n";
  std::string r2 = "GET /awesome.txt HTTP/1.1\r\nHost: vcm-31639.vm.duke.edu\r\n\r\n";
  result.push_back(r1);
  result.push_back(r2);
  for (size_t i = 0; i < result.size(); i++) {
    time_t curr_time;
    time(&curr_time);
    http_Request h1 = http_Request(0, result[i], "", curr_time);
    std::cout << "Parsed method " << h1.return_method() << std::endl;
    std::cout << "URI: " << h1.return_uri() << std::endl;
    std::cout << "Http version: " << h1.return_httpver() << std::endl;
    std::cout << "Parsed Host: " << h1.return_Host() << std::endl;
    std::cout << "Header and conetent:" << h1.return_header() << std::endl;
    std::cout << "Time: " << parseTime(h1.return_time()) << std::endl;
  }

  return 0;
}