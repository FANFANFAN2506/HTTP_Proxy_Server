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
  int increment = 65536;
  std::vector<char> data_buff(increment, 0);
  int start = 0;
  do {
    last_rec = data_rec;
    start += data_rec;
    data_buff.resize(start + increment);
    data_rec = recv(client_fd, &data_buff.data()[start], increment, 0);
    if (data_rec == 0) {
      std::cerr << "The connection is closed" << std::endl;
      return "";
    }
    std::cout << "data received has a size of " << data_buff.size() << std::endl;
    for (size_t i = 0; i < data_buff.size(); i++) {
      std::cout << data_buff[i];
    }
    std::cout << std::endl;
    std::cout << "recev return " << data_rec << std::endl;
  } while (data_rec >= last_rec);
  int diff = increment - data_rec;
  data_buff.resize(data_buff.size() - diff);
  std::string request;
  for (size_t i = 0; i < data_buff.size(); i++) {
    request += data_buff[i];
  }
  return request;
}

std::string receiveAll(int client_fd){
  char buffer[10];
  std::string ans;
  memset(buffer,0,10);
  //std::cout<< recv(client_fd, buffer, sizeof(buffer), 0) <<std::endl;
  while(recv(client_fd, buffer, sizeof(buffer), MSG_WAITALL) > 0){
    string tmp = buffer;
    ans.append(tmp);
    // std::cout << buffer << std::endl;
    memset(buffer,0,sizeof(buffer));
  }
  std::cout << ans << std::endl;
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