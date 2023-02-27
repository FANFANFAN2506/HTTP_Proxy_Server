#include <stdlib.h>
#include <list>
#include <unordered_map>

#include "response.hpp"
extern pthread_mutex_t cacheLock;
using namespace std;
class Cache {
 public:
  size_t capacity;
  list<pair<string, http_Response *> > respList;
  unordered_map<string, list<pair<string, http_Response *> >::iterator> map;

 public:
  Cache(size_t size){
    capacity = size;
  }

  /**
   * @func: get response pointer from the cache using URI
   * @param: {url: URI of the response}  
   * @return: {response pointer if found, else NULL} 
   */
  http_Response * get(string url) {
    pthread_mutex_lock(&cacheLock);
    const auto it = map.find(url);
    // If key does not exist
    if (it == map.cend()){
      pthread_mutex_unlock(&cacheLock);
      return NULL;
    }
    // Move this key to the front of the cache
    respList.splice(respList.begin(), respList, it->second);
    pthread_mutex_unlock(&cacheLock);
    return it->second->second;
  }

  /**
   * @func: put the response in the cache by URI
   * @param: {url: URI string of the response. value: http_Response response pointer}  
   * @return: {the URI string if cache exceed max capacity and removed, else empty string} 
   */
  string put(string url, http_Response * value) {
    pthread_mutex_lock(&cacheLock);
    const auto it = map.find(url);
    string ans = "";

    // Key already exists
    if (it != map.cend()) {
      // Update the value
      delete it->second->second;
      it->second->second = value;
      // Move this entry to the front of the cache
      respList.splice(respList.begin(), respList, it->second);
      pthread_mutex_unlock(&cacheLock);
      return "";
    }

    // Reached the capacity, remove the oldest entry
    if (respList.size() == capacity) {
      const auto & node = respList.back();
      ans = node.first;
      map.erase(node.first);
      respList.pop_back();
    }

    // Insert the entry to the front of the cache and update mapping.
    respList.emplace_front(url, value);
    map[url] = respList.begin();
    pthread_mutex_unlock(&cacheLock);
    return ans;
  }

  /**
   * @func: check wether the response is expired.
   * @param: {url: URI string of the website}  
   * @return: {True: expired or not found. False: not expired} 
   */
  int checkExpire(string url){
    const auto it = map.find(url);
    if (it != map.cend()){
      http_Response * resp = (it->second->second);
      time_t maxAge = resp->return_max();
      time_t expireTime  = resp->return_expire();
      time_t receivedTime = resp->return_date();
      time_t currTime;
      time(&currTime);

      if((currTime - receivedTime > maxAge || maxAge == -1) && (currTime < expireTime || expireTime == 0)){
        return false;
      }
    }
    return true;
  }
};