#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <ctime>
#include <exception>
#include <iostream>
#include <string>
#include <vector>
// using namespace std;

class http_Request {
 private:
  long UID;             //UID unique id for log
  int socket;           //socket descriptor
  std::string Line;     //Whole contents of the request
  std::string REQUEST;  //Request line(first line)
  std::string Header;   //For CONNECT method remove the request line
  std::string IPFROM;   //Request from IP
  std::string Method;
  std::string Host_name;  //Target server IP
  std::string Host_port;  //Target server port
  std::string TIME;

 public:
  //default constructor
  http_Request() :
      UID(0),
      socket(0),
      Line(),
      REQUEST(),
      Header(),
      IPFROM(),
      Method(),
      Host_name(),
      Host_port(),
      TIME() {}
  http_Request(long uid, int sd, std::string l, std::string ip, std::string t) :
      UID(uid),
      socket(sd),
      Line(l),
      REQUEST(),
      Header(),
      IPFROM(ip),
      Method(),
      Host_name(),
      Host_port(),
      TIME(t) {
    getHost();
    getMethod();
    getRequestLine();
  }
  long return_UID() const { return UID; }
  int return_socket() const { return socket; }
  std::string return_Line() const { return Line; }
  std::string return_request() const { return REQUEST; }
  std::string return_header() const { return Header; }
  std::string return_ip() const { return IPFROM; }
  std::string return_method() const { return Method; }
  std::string return_Host() const { return Host_name; }
  std::string return_port() const { return Host_port; }
  std::string return_time() const { return TIME; }
  void getMethod() {
    try {
      size_t start_line = Line.find_first_of(" ");
      Method = Line.substr(0, start_line);
      std::string get = "GET";
      std::string post = "POST";
      std::string connect = "CONNECT";
      if (Method != get && Method != connect && Method != post) {
        assert(Method.c_str() != get);
        std::cout << "Illegal method is " << Method.c_str() << std::endl;
        throw std::invalid_argument("Only support GET POST CONNECT methods");
      }
    }
    catch (std::exception & e) {
      std::cout << "Error" << e.what() << std::endl;
      return;
    }
  }
  void getHost() {
    try {
      size_t host_start = Line.find("Host: ");
      std::string host_name = Line.substr(host_start + 6);
      size_t host_end = host_name.find_first_of("\r\n");
      if (host_start == std::string::npos || host_end == std::string::npos) {
        throw std::invalid_argument("Cannot find the host name");
      }
      std::string Host = host_name.substr(0, host_end);
      std::cout << "Host start is " << host_start << " Host end is " << host_end
                << "Host name is" << host_name << " Host is " << Host << std::endl;
      size_t port_start = Host.find_first_of(":\r");
      //if we allowed it to declare port
      //maybe the we could hard code as 80
      if (port_start != std::string::npos) {
        //the port is declared
        Host_name = host_name.substr(0, port_start);
        Host_port = Host.substr(port_start + 1);
      }
      else {
        //not declared port; default 80 port for HTTP/TCP
        Host_name = Host;
        Host_port = "80";
      }
    }
    catch (std::exception & e) {
      std::cout << "Error: " << e.what() << std::endl;
      return;
    }
  }
  void getRequestLine() {
    try {
      size_t request_line_end = Line.find_first_of("\r\n");
      if (request_line_end == std::string::npos) {
        throw std::invalid_argument("Wrong request line format");
      }
      REQUEST = Line.substr(0, request_line_end);
      Header = Line.substr(request_line_end + 2);
    }
    catch (std::exception & e) {
      std::cout << e.what() << std::endl;
      return;
    }
  }
};