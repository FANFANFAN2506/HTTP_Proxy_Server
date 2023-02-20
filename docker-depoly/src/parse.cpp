#include "parse.hpp"

int main() {
  string input = "GET /awesome.txt HTTP/1.1\r\nHost: vcm-31639.vm.duke.edu\r\n\r\n";
  string input2 = "GET / HTTP/1.1\r\nHost: vcm-30458.vm.duke.edu:8000\r\n\r\n";
  http_Request * r1 = new http_Request(input);
  cout << "Method is: " << r1->return_method() << endl;
  cout << "Host is: " << r1->return_Host() << endl;
  cout << "Port is: " << r1->return_port() << endl;
  delete r1;
  http_Request * r2 = new http_Request(input2);
  cout << "Method is: " << r2->return_method() << endl;
  cout << "Host is: " << r2->return_Host() << endl;
  cout << "Port is: " << r2->return_port() << endl;
  delete r2;

  return 0;
}