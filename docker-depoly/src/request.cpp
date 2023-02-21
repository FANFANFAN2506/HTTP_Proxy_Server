#include "request.hpp"

/*This file is for testing*/
int main(void) {
  std::string result = "GET /uri.cgi HTTP/1.1\r\nUser-Agent: Mozilla/5.0\r\nAccept: "
                       "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
                       "*;q=0.8\r\nHost: 127.0.0.1\r\n\r\n";
  http_Request h1 = http_Request(0, result, "", "");
  std::cout << "Parsed method " << h1.return_method() << std::endl;
  std::cout << "Http version: " << h1.return_httpver() << std::endl;
  std::cout << "Parsed Host: " << h1.return_Host() << std::endl;
  std::cout << "Header and conetent:" << h1.return_header() << std::endl;

  return 0;
}