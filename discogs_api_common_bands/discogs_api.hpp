#ifndef discogs_api_hpp
#define discogs_api_hpp

#include <vector>
#include <memory>
#include "artist.hpp"

extern std::vector<std::unique_ptr<Artist>> artist_ptrs;

int fetch_artist(int artist_id);

#endif