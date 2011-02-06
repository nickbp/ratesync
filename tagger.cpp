/*
  mpdtagger - Synchronizes metadata between MPD stickers and media files.
  Copyright (C) 2010  Nicholas Parker

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tagger.h"
#include "config.h"

#include <sstream>
#include <iostream>

void mpdtagger::Tagger::file_to_db() {

	//iterate over all mpd songs, retrieve local ratings and store in map
	std::list<mpd::song_t> mpd_songs;
	mpd::Access mpd(host, port);
	try {
		mpd.connect();
		mpd.songs(mpd_songs);
	} catch (const mpd::Error& err) {
		throw TaggerError(err.what());
	}
	
	//retrieve all local file ratings in bulk before sending any to mpd
	std::map<mpd::song_t, rating_t> file_ratings;
	std::set<mpd::song_t> file_unrated;
	try{
		media::Access media(dir);
		media.ratings(mpd_songs, file_ratings, file_unrated);
	} catch (const media::Error& err) {
		throw TaggerError(err.what());
	}

	//iterate over file rating cache, update ratings at server as needed
	//(only files found in the specified directory are updated)
	for (std::list<mpd::song_t>::iterator it = mpd_songs.begin();
		 it != mpd_songs.end(); ++it) {
		mpd::song_t& song = *it;
		std::map<mpd::song_t, rating_t>::const_iterator file_rating_iter =
			file_ratings.find(song);
		rating_t mpd_rating;
		bool mpd_has_rating = mpd.rating_get(song, mpd_rating);
		if (file_rating_iter != file_ratings.end()) {
			//file has a rating
			rating_t file_rating = file_rating_iter->second;
			if (!mpd_has_rating) {
				//set rating value
				config::debug("SET %s: UNRATED -> %d",
							  song.c_str(), file_rating);
				mpd.rating_set(song, file_rating);
			} else if (mpd_rating != file_rating) {
				//set rating value
				config::debug("SET %s: %d -> %d",
							  song.c_str(), mpd_rating, file_rating);
				mpd.rating_set(song, file_rating);
			} else {
				config::debug("MATCH %s = %d", song.c_str(), mpd_rating);
			}
		} else if (file_unrated.find(song) != file_unrated.end()) {
			//file is unrated (or rating couldnt be parsed)
			if (mpd_has_rating) {
				//clear mpd rating
				config::debug("CLEAR %s", song.c_str());
				mpd.rating_clear(song);
			} else {
				config::debug("MATCH %s = UNRATED", song.c_str());
			}
		}
	}
}
