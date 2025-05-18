#include <iostream>
#include <curl/curl.h>
#include <string>
#include <algorithm>
#include "discogs_api.hpp"
#include "artist.hpp"
#include "bands.hpp"
#include "json.hpp" //biblioteka dostepna pod linkiem: https://github.com/nlohmann/json/releases

//g++ main.cpp artist.cpp discogs_api.cpp bands.cpp -o main -Wall -lcurl

int main(int argc, char* argv[]){
    if(argc < 3){
        std::cerr << "Error, argv should contain at least two artist ids" << std::endl;
        return 1;
    }
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    for(int i = 1; i < argc; i++){
        std::string arg(argv[i]);
        if(!std::all_of(arg.begin(), arg.end(), ::isdigit)){
            std::cerr << "Error, artist id must be a valid positive integer: " << arg << std::endl;
            curl_global_cleanup();
            return 1;
        }
        try{
            int artist_id = std::stoi(arg);
            if(fetch_artist(artist_id) != 0){
                std::cerr << "Error, couldn't fetch artist with id: " << artist_id << std::endl;
                curl_global_cleanup();
                return 1;
            }
        }catch(const std::invalid_argument&){
            std::cerr << "Error, artist id must be a valid positive integer: " << arg << std::endl;
            curl_global_cleanup();
            return 1;
        }catch(const std::out_of_range&){
            std::cerr << "Error: artist id is out of integers range: " << arg << std::endl;
            curl_global_cleanup();
            return 1;
        }
    }
    print_common_groups();
    curl_global_cleanup();
    return 0;
}