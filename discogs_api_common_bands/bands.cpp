#include <vector>
#include <iostream>
#include "bands.hpp"
#include "discogs_api.hpp"

void print_common_groups(){
    std::map<std::string, std::vector<std::string>> band_to_artists;
    for(const auto& ptr : artist_ptrs){
        for(const auto& band : ptr->bands) {
            band_to_artists[band].push_back(ptr->name);
        }
    }
    for(auto& [band, artists] : band_to_artists){
        std::sort(artists.begin(), artists.end());
    }
    std::cout << "Common groups:\n";
    for(const auto& [band, artists] : band_to_artists){
        if(artists.size() > 1){
            std::cout << band << ": ";
            for(size_t i = 0; i < artists.size(); ++i){
                std::cout << artists[i];
                if(i < artists.size() - 1){
                    std::cout << ", ";
                }
            }
            std::cout << std::endl;
        }
    }
};
