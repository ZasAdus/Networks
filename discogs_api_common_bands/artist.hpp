#ifndef artist_hpp
#define artist_hpp

#include <string>
#include <vector>
#include <curl/curl.h>
#include "json.hpp"

class Artist{
public:
    int id;
    std::string name;
    std::string buff;
    std::vector<std::string> bands;

    Artist(int artist_id);
    static size_t callback(void* contents, size_t size, size_t nmemb, void* userp);
    void get_bands();
};

#endif