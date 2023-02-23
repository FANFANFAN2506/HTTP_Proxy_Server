#include <stdlib.h>
#include <unordered_map>
#include <list>
#include "response.hpp"

using namespace std;
class Cache {
private:
    size_t capacity;
    list<pair<string, http_Response *> > cache;
    unordered_map<string, list<pair<string, http_Response *> >::iterator> map;

    //mutex
public:
    Cache(size_t capacity) {
        capacity = capacity;
    }
    
    http_Response * get(string url) {
        const auto it = map.find(url);
        // If key does not exist
        if (it == map.cend()) return NULL;
 
        // Move this key to the front of the cache
        cache.splice(cache.begin(), cache, it->second);
        return it->second->second;
    }
    
    void put(string url, http_Response * value) {        
        const auto it = map.find(url);
        
        // Key already exists
        if (it != map.cend()) {
            // Update the value
            it->second->second = value;
            // Move this entry to the front of the cache
            cache.splice(cache.begin(), cache, it->second);
            return;
        }
        
        // Reached the capacity, remove the oldest entry
        if (cache.size() == capacity) {
            const auto& node = cache.back();
            map.erase(node.first);
            cache.pop_back();
        }
        
        // Insert the entry to the front of the cache and update mapping.
        cache.emplace_front(url, value);
        map[url] = cache.begin();
    }
};

//Cache Update问题
//Cache初始化