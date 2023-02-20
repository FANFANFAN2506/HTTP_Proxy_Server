#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>

pthread_mutex_t logLock = PTHREAD_MUTEX_INITIALIZER;

void log(std::string msg){
    pthread_mutex_lock(&logLock);
    std::ofstream logfile;
    logfile.open("/var/log/erss/log.txt");
    logfile << msg.c_str() << std::endl;
    logfile.close();
    pthread_mutex_unlock(&logLock);
}