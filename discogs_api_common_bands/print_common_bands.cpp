#include <vector>
#include <iostream>
#include "print_common_bands.hpp"
#include "discogs_api.hpp"

void print_common_bands(){
    std::map<std::string, std::vector<std::string>> band_to_artists;
    for(const auto& ptr : artist_ptrs){
        for(const auto& band : ptr->bands) {
            band_to_artists[band].push_back(ptr->name);
        }
    }

    std::cout << "Common groups:\n";
    for(auto& [band, artists] : band_to_artists){
        if(artists.size() > 1){
            std::sort(artists.begin(), artists.end());
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
