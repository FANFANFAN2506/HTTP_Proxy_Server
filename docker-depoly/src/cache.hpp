#include <stdlib.h>

#include <list>
#include <mutex>
#include <unordered_map>

#include "response.hpp"
// extern pthread_mutex_t cacheLock;

using namespace std;
class Cache {
 public:
  std::mutex cacheLock;
  size_t capacity;
  list<pair<string, http_Response *> > respList;
  unordered_map<string, list<pair<string, http_Response *> >::iterator> map;

 public:
  Cache(size_t size) { capacity = size; }

  /**
   * @func: get response pointer from the cache using URI
   * @param: {url: URI of the response}  
   * @return: {response pointer if found, else NULL} 
   */
  http_Response * get(string url) {
    // pthread_mutex_lock(&cacheLock);
    cacheLock.lock();
    const auto it = map.find(url);
    // If key does not exist
    if (it == map.cend()) {
      // pthread_mutex_unlock(&cacheLock);
      cacheLock.unlock();
      return NULL;
    }
    // Move this key to the front of the cache
    respList.splice(respList.begin(), respList, it->second);
    // pthread_mutex_unlock(&cacheLock);
    cacheLock.unlock();
    return it->second->second;
  }

  /**
   * @func: put the response in the cache by URI
   * @param: {url: URI string of the response. value: http_Response response pointer}  
   * @return: {the URI string if cache exceed max capacity and removed, else empty string} 
   */
  string put(string url, http_Response * value) {
    // pthread_mutex_lock(&cacheLock);
    cacheLock.lock();
    const auto it = map.find(url);
    string ans = "";

    // Key already exists
    if (it != map.cend()) {
      // Update the value
      delete it->second->second;
      it->second->second = value;
      // Move this entry to the front of the cache
      respList.splice(respList.begin(), respList, it->second);
      // pthread_mutex_unlock(&cacheLock);
      cacheLock.unlock();
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
    cacheLock.unlock();
    // pthread_mutex_unlock(&cacheLock);
    return ans;
  }

  /**
   * @func: check wether the response is expired.
   * @param: {url: URI string of the website}  
   * @return: {True: expired or not found. False: not expired} 
   */
  int checkExpire(string url, int rmaxAge) {
    const auto it = map.find(url);
    if (it != map.cend()) {
      http_Response * resp = (it->second->second);
      time_t maxAge = resp->return_max();
      time_t expireTime = resp->return_expire();
      time_t receivedTime = resp->return_date();
      time_t currTime;
      currTime = time(0);
      time_t minMaxAge;
      if (maxAge != -1 && rmaxAge != -1) {
        minMaxAge = maxAge > rmaxAge ? rmaxAge : maxAge;
      }
      else {
        minMaxAge = maxAge < rmaxAge ? rmaxAge : maxAge;
      }
      if ((currTime - receivedTime < minMaxAge || minMaxAge == -1) &&
          (currTime < expireTime || expireTime == 0)) {
        return false;
      }
    }
    return true;
  }
};