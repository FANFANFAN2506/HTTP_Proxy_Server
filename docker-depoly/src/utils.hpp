#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <fstream>
#include <ctime>
#include <iostream>
#include <vector>
#include <pthread.h>

extern pthread_mutex_t logLock;

void log(std::string msg){
    pthread_mutex_lock(&logLock);
    std::ofstream logfile;
    logfile.open("../log/proxy.log",std::ios::app);
    logfile << msg;
    logfile.close();
    pthread_mutex_unlock(&logLock);
}

char * to_char(std::string s) {
  char * cstr;
  cstr = new char[s.size() + 1];
  strcpy(cstr, s.c_str());
  return cstr;
}

std::vector<char> recvChar(int client_fd) {
  int data_rec;
  // int increment = 65536;
  int total = 0;
  int increment = 4096;
  int start = 0;
  std::vector<char> data_buff(increment, 0);
  while ((data_rec = recv(client_fd, &data_buff.data()[start], increment, 0)) > 0) {
    //There is data received
    total += data_rec;
    // std::cout << "size is " << data_buff.size() << std::endl;
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

std::string parseTime(time_t time) {
  struct tm * timeinfo;
  timeinfo = localtime(&time);
  std::string time_str = asctime(timeinfo);
  return time_str;
}
