#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <ctime>
#include <iostream>
#include <vector>

std::string recvAll(int client_fd) {
  int data_rec = 0;
  int last_rec = 0;
  //each received max length,as large as possible to reduce recv time
  int increment = 1024;
  std::vector<char> data_buff(increment, 0);
  int start = 0;
  do {
    last_rec = data_rec;
    start += data_rec;
    data_buff.resize(start + increment);
    data_rec = recv(client_fd, &data_buff.data()[start], increment, 0);
    std::cout << "data received has a size of " << data_buff.size() << std::endl;
    for (size_t i = 0; i < data_buff.size(); i++) {
      std::cout << data_buff[i];
    }
    std::cout << std::endl;
    std::cout << "recev return " << data_rec << std::endl;
  } while (data_rec >= last_rec && data_rec != 0);
  int diff = increment - data_rec;
  data_buff.resize(data_buff.size() - diff);
  char temp[data_buff.size()];
  std::string request;
  for (size_t i = 0; i < data_buff.size(); i++) {
    request += data_buff[i];
  }
  return request;
}

std::string get_current_Time() {
  time_t curr_time;
  time(&curr_time);
  struct tm * timeinfo;
  timeinfo = localtime(&curr_time);
  std::string Curr_time = asctime(timeinfo);
  return Curr_time;
}