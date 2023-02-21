#include "request.hpp"

/*This file is for testing*/
int main(void) {
  std::string result = "GET /awesome.txt HTTP/1.1\r\nHost: vcm-31639.vm.duke.edu\r\n\r\n";
  http_Request h1 = http_Request(0, result, "", "");
  std::cout << "Parsed method " << h1.return_method() << std::endl;
  std::cout << "Parsed Host: " << h1.return_Host() << std::endl;
  std::cout << "Header:" << h1.return_header() << std::endl;
  return 0;
}