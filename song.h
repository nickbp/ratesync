#ifndef MPDTAGGER_SONG_H
#define MPDTAGGER_SONG_H

#include <string>

namespace mpdtagger {
	typedef size_t rating_t;


	typedef std::string song_t;
	typedef std::pair<song_t, rating_t> song_rating_t;
	//                                  old       new
	typedef std::pair<song_t, std::pair<rating_t, rating_t> > song_ratings_t;


	//                dest    src
	typedef std::pair<song_t, std::string> symlink_t;
	typedef std::pair<symlink_t, rating_t> symlink_rating_t;
	//                                     old       new
	typedef std::pair<symlink_t, std::pair<rating_t, rating_t> > symlink_ratings_t;
}

#endif