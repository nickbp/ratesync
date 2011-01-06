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

#include <sstream>
#include <iostream>

void mpdtagger::Tagger::file_to_db() {

	//iterate over all mpd songs, retrieve local ratings and store in map
	std::list<mpdtagger::MpdSong> mpd_songs;
	try {
		MpdAccess mpd(host);//TODO cant do this, destroys mpd conn instance!
		mpd.songs(mpd_songs);
	} catch (const MpdError& err) {
		throw TaggerError(err.what());
	}
	
	//retrieve all local file ratings in bulk before sending any to mpd
	std::map<mpdtagger::MpdSong,int> file_ratings;
	try{
		MediaAccess media(dir);
		media.ratings(mpd_songs, file_ratings);
	} catch (const MediaError& err) {
		throw TaggerError(err.what());
	}

	//iterate over file rating cache, update ratings at server as needed
	//(only files found in the specified directory are updated)
	for (std::list<mpdtagger::MpdSong>::iterator it = mpd_songs.begin();
		 it != mpd_songs.end(); ++it) {
		mpdtagger::MpdSong& song = *it;
		std::map<mpdtagger::MpdSong,int>::const_iterator file_find =
			file_ratings.find(song);
		int file_rating;
		if (file_find == file_ratings.end()) {
			continue;
		} else {
			file_rating = file_find->second;
		}

		int mpd_rating = song.rating();

		if (mpd_rating != file_rating) {
			//update mpd rating:
			if (file_rating == mpdtagger::MpdAccess::UNRATED) {
				//clear mpd rating
				std::cout << "CLEAR " << song.uri() << std::endl;
				song.rating_clear();
			} else {
				//set rating value
				std::cout << "SET " << song.uri() << ": "
						  << mpd_rating << "->" << file_rating << std::endl;
				song.rating_set(file_rating);
			}
		} else {
			std::cout << "MATCH " << song.uri() << " = " << mpd_rating << std::endl;
		}
	}
}
