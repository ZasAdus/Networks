#include "discogs_api.hpp"
#include <curl/curl.h>
#include <iostream>
#include <unistd.h>

std::vector<std::unique_ptr<Artist>> artist_ptrs;

int fetch_artist(int artist_id){
    CURL* curl = curl_easy_init();
    if(!curl){
        std::cerr << "Error, couldn't init curl" << std::endl;
        return 1;
    }

    std::string url = "https://api.discogs.com/artists/" + std::to_string(artist_id);
    auto artist = std::make_unique<Artist>(artist_id);
    Artist* ptr = artist.get();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Artist::callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, ptr);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "cokolwiek");

    int attempt = 1;
    const int max_attempts = 10;
    bool is_response_200 = false;
    long http_code = 0; 

    CURLcode res;
    while(attempt <= max_attempts && !is_response_200){
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if(res != CURLE_OK){
            std::cerr << "Error, curl_easy_perform" << std::endl;
            break;
        }

        if(http_code == 200){
            ptr->get_bands();
            artist_ptrs.push_back(std::move(artist));
            is_response_200 = true;
        }else if(http_code == 429){
            std::cerr << "Rate limited, retrying attempt: " << attempt << "/10" << std::endl;
            ++attempt;
            sleep(1);
        }else{
            std::cerr << "Error, http: " << http_code << std::endl;
            break;
        }
    }

    curl_easy_cleanup(curl);

    return is_response_200 ? 0 : 1;
}
