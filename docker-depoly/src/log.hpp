#include <pthread.h>
#include <fstream>

extern pthread_mutex_t logLock;

static void log(std::string msg){
    pthread_mutex_lock(&logLock);
    std::ofstream logfile;
    logfile.open("../log/proxy.log",std::ios::app);
    logfile << msg;
    logfile.close();
    pthread_mutex_unlock(&logLock);
}