#include <cassert>
#include <exception>
#include <iostream>
#include <string>
using namespace std;

class http_Request {
 private:
  long UID;  //UID unique id for log
  int sd;    //socket descriptor
  string Line;
  string Method;
  string Host_name;
  string Host_port;

 public:
  //default constructor
  http_Request() : Line(), Method(), Host_name(), Host_port() {}
  http_Request(string l) : Line(l), Method(), Host_name(), Host_port() {
    getHost();
    getMethod();
  }
  string return_method() { return Method; }
  string return_Host() { return Host_name; }
  string return_port() { return Host_port; }
  void getMethod() {
    try {
      size_t start_line = Line.find_first_of(" ");
      Method = Line.substr(0, start_line);
      string get = "GET";
      string post = "POST";
      string connect = "CONNECT";
      if (Method != get && Method != connect && Method != post) {
        assert(Method.c_str() != get);
        cout << "Illegal method is " << Method.c_str() << endl;
        throw invalid_argument("Only support GET POST CONNECT methods");
      }
    }
    catch (exception & e) {
      cout << "Error" << e.what() << endl;
      return;
    }
  }
  void getHost() {
    try {
      size_t host_start = Line.find("Host: ");
      string host_name = Line.substr(host_start + 6);
      size_t host_end = host_name.find_first_of("\r\n");
      if (host_start == string::npos || host_end == string::npos) {
        throw invalid_argument("Cannot find the host name");
      }
      string Host = host_name.substr(0, host_end);
      cout << "Host start is " << host_start << " Host end is " << host_end
           << "Host name is" << host_name << " Host is " << Host << endl;
      size_t port_start = Host.find_first_of(":\r");
      //if we allowed it to declare port
      //maybe the we could hard code as 80
      if (port_start != string::npos) {
        //the port is declared
        Host_name = host_name.substr(0, port_start);
        Host_port = Host.substr(port_start + 1);
      }
      else {
        //not declared port
        Host_name = Host;
        Host_port = "80";
      }
    }
    catch (exception & e) {
      cout << "Error: " << e.what() << endl;
      return;
    }
  }
};