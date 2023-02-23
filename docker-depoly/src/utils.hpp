#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <ctime>
#include <iostream>
#include <vector>

std::string recvAll(int client_fd) {
  int data_rec;
  int increment = 65536;
  int start = 0;
  std::vector<char> data_buff(increment, 0);
  data_rec = recv(client_fd, &data_buff.data()[start], increment, 0);
  std::cout << "receive from " << client_fd << std::endl;
  if (data_rec == 0) {
    std::cerr << "The connection is closed" << std::endl;
    return "";
  }
  data_buff.resize(data_rec);
  std::string request;
  for (size_t i = 0; i < data_buff.size(); i++) {
    request += data_buff[i];
  }
  return request;
}

<<<<<<< HEAD
std::string receiveAll(int client_fd) {
  char buffer[10];
  std::string ans;
  memset(buffer, 0, 10);
  //std::cout<< recv(client_fd, buffer, sizeof(buffer), 0) <<std::endl;
  while (recv(client_fd, buffer, sizeof(buffer), MSG_WAITALL) > 0) {
    string tmp = buffer;
    ans.append(tmp);
    // std::cout << buffer << std::endl;
    memset(buffer, 0, sizeof(buffer));
=======
std::string receiveAll(int client_fd){
  char buffer[1024];
  std::string ans;
  memset(buffer,0,1024);
  while(recv(client_fd, buffer, sizeof(buffer), 0) > 0){
    string tmp = buffer;
    ans.append(tmp);
    //std::cout << tmp << std::endl;
    memset(buffer,0,sizeof(buffer));
>>>>>>> 2ed3719d1d8e6493c253b52c3fab866f101b8f65
  }
  std::cout << "!!!!!!!!!!!!!" << std::endl;
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