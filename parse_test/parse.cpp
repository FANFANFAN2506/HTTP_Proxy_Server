#include "parse.hpp"

int main(int argc, char * argv[]) {
  /*if (argc != 2) {
    cerr << "Wrong input arguments number" << endl;
  }*/
  string input = argv[1];
  http_Request * r1 = new http_Request(input);
  cout << "Method is: " << r1->return_method() << endl;
  cout << "Host is: " << r1->return_Host() << endl;
  cout << "Port is: " << r1->return_port() << endl;
  delete r1;
  return 0;
}
