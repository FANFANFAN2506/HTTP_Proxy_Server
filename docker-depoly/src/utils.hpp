#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>

std::vector<char> recvBuff(int client_fd) {
  std::vector<char> data_buff(65536, 0);
  int data_rec = 0;
  data_rec = recv(client_fd, &data_buff.data()[0], 65536, 0);
  data_buff.resize(data_rec);
  return data_buff;
}

std::vector<char> recvChar(int client_fd) {
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  int data_rec = 0;
  int total = 0;
  int increment = 20480;
  int start = 0;
  std::vector<char> data_buff(increment, 0);
  setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
  while ((data_rec = recv(client_fd, &data_buff.data()[start], increment, 0)) > 0) {
    //There is data received
    total += data_rec;
    if (data_rec < increment) {
      //connection close
      break;
    }
    //need to do another recv
    data_buff.resize(data_buff.size() + increment);
    start += data_rec;
  }
  data_buff.resize(total);
  return data_buff;
}

std::string parseTime(time_t time) {
  struct tm * timeinfo;
  timeinfo = localtime(&time);
  std::string time_str = asctime(timeinfo);
  return time_str;
}