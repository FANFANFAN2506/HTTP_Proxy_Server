#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <ctime>
#include <iostream>
#include <vector>

char * to_char(std::string s) {
  char * cstr;
  cstr = new char[s.size() + 1];
  strcpy(cstr, s.c_str());
  return cstr;
}

std::vector<char> recvChar(int client_fd) {
  int data_rec;
  int increment = 65536;
  int start = 0;
  std::vector<char> data_buff(increment, 0);
  data_rec = recv(client_fd, &data_buff.data()[start], increment, 0);
  // std::cout << "receive from " << client_fd << std::endl;
  if (data_rec == 0) {
    std::cerr << "The connection is closed" << std::endl;
    std::vector<char> fail;
    return fail;
  }
  data_buff.resize(data_rec);
  return data_buff;
}

std::string recvAll(int client_fd) {
  std::vector<char> data_buff = recvChar(client_fd);
  if (data_buff.size() == 0) {
    return "";
  }
  std::string request;
  for (size_t i = 0; i < data_buff.size(); i++) {
    request += data_buff[i];
  }
  return request;
}

std::string receiveAll(int client_fd) {
  char buffer[10];
  std::string ans;
  memset(buffer, 0, 10);
  int i = 0;
  while ((i = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
    string tmp = buffer;
    ans.append(tmp);
    memset(buffer, 0, sizeof(buffer));
    if (i < 10) {
      break;
    }
  }
  return ans;
}

std::string parseTime(time_t curr_time) {
  // time_t curr_time;
  // time(&curr_time);
  struct tm * timeinfo;
  timeinfo = localtime(&curr_time);
  std::string Curr_time = asctime(timeinfo);
  return Curr_time;
}