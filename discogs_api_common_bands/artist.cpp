#include "artist.hpp"
#include <iostream>

Artist::Artist(int artist_id) : id(artist_id){}

size_t Artist::callback(void* contents, size_t size, size_t nmemb, void* userp){
    size_t real_size = size * nmemb;
    Artist* artist = static_cast<Artist*>(userp);
    artist->buff.append(static_cast<char*>(contents), real_size);
    return real_size;
}

void Artist::get_bands(){
    try{
        auto root = nlohmann::json::parse(buff);
        if(root.contains("name")){
            //std::cout << "Imie: " << root["name"]<< std::endl;
            name = root["name"];
        }
        if(root.contains("groups") && root["groups"].is_array()){
            for(const auto& group : root["groups"]){
                if(group.contains("name") && group["name"].is_string()){
                    bands.push_back(group["name"]);
                    //std::cout << "Grupa: " << group["name"] << std::endl;
                }
            }
        }
    }catch(const nlohmann::json::parse_error& e){
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
}