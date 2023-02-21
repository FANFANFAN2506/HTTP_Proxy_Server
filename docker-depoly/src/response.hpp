#include "utils.hpp"

class http_Response {
  int socket_server;
  std::string http_ver;
  unsigned int statusCode;
  std::string status;
  std::string cache_ctrl;
  std::string expires;
};