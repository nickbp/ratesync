#ifndef RATESYNC_SONG_H
#define RATESYNC_SONG_H

#include <string>

#ifdef _WIN32
#define SEP '\\'
#else
#define SEP '/'
#endif

#define UNRATED -1

namespace ratesync {
	typedef int rating_t;
	typedef std::string song_t;

	typedef struct {
		song_t path;
		rating_t rating;
	} song_rating_t;

	typedef struct {
		song_t path;
		rating_t rating_old;
		rating_t rating_new;
	} song_ratings_t;
}

#endif
