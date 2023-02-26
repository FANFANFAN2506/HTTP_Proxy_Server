#include <stdlib.h>
#include <list>
#include <unordered_map>

#include "response.hpp"

using namespace std;
class Cache {
 public:
  size_t capacity;
  list<pair<string, http_Response *> > respList;
  unordered_map<string, list<pair<string, http_Response *> >::iterator> map;

  //mutex
 public:
  Cache(size_t size){
    capacity = size;
  }

  http_Response * get(string url) {
    const auto it = map.find(url);
    // If key does not exist
    if (it == map.cend()){
      return NULL;
    }
    // Move this key to the front of the cache
    respList.splice(respList.begin(), respList, it->second);
    return it->second->second;
  }

  string put(string url, http_Response * value) {
    const auto it = map.find(url);
    string ans = "";
    // Key already exists
    if (it != map.cend()) {
      // Update the value
      delete it->second->second;
      it->second->second = value;
      // Move this entry to the front of the cache
      respList.splice(respList.begin(), respList, it->second);
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
    return ans;
  }

  /**
   * Input: URI of the website
   * Output: True: expired or not find, need update, False: not expire, no update needed.
  */
  bool checkExpire(string url){
    const auto it = map.find(url);
    if (it != map.cend()){
      http_Response * resp = (it->second->second);
      time_t maxAge = resp->return_max();
      time_t expireTime  = resp->return_expire();
      time_t receivedTime = resp->return_date();
      time_t currTime;
      time(&currTime);
      if(maxAge == -1 && currTime < expireTime){
        return false;
      }
      if(currTime - receivedTime > maxAge || currTime > expireTime){
        return true;
      }
      return false;
    }
    return true;
  }
};