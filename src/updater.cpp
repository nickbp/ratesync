/*
  ratesync - Manages songs according their rating metadata.
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

#include "updater.h"
#include "config.h"

#include <sstream>
#include <iostream>

namespace {
	using ratesync::song_t;
	using ratesync::rating_t;

	using ratesync::song_rating_t;
	using ratesync::song_ratings_t;

	void get_changes(const std::map<song_t, rating_t>& src_ratings,
					 const std::map<song_t, rating_t>& dest_ratings,
					 std::list<song_ratings_t>& dest_rating_change) {
		//ignore files not listed in input
		for (std::map<song_t, rating_t>::const_iterator
				 it = src_ratings.begin(); it != src_ratings.end(); ++it) {
			const song_t& song = it->first;
			std::map<song_t, rating_t>::const_iterator
				dest_it = dest_ratings.find(song);
			if (dest_it != dest_ratings.end()) {
				//file exists, has a rating (or unrated)
				rating_t src_rating = it->second,
					dest_rating = dest_it->second;
				if (src_rating != dest_rating) {
					//update mpd rating
					dest_rating_change.push_back(song_ratings_t());
					song_ratings_t& srs = dest_rating_change.back();
					srs.path = song;
					srs.rating_old = dest_rating;
					srs.rating_new = src_rating;
				}
			}
		}
	}
}

bool ratesync::Updater::Calculate() {
	std::map<song_t, rating_t> src_ratings, dest_ratings;
	if (!src->Get(src_ratings) || !dest->Get(dest_ratings)) {
		return false;
	}

	get_changes(src_ratings, dest_ratings, dest_rating_change);

	return true;
}

bool ratesync::Updater::HasChanges() const {
	return !dest_rating_change.empty();
}

namespace {
	inline std::string str(rating_t rating) {
		std::ostringstream oss;
		if (rating == UNRATED) {
			oss << "unrated";
		} else {
			oss << rating;
		}
		return oss.str();
	}
}

void ratesync::Updater::Print() const {
	if (dest_rating_change.empty()) {
		config::log("No changes to be made.");
	} else {
		size_t i = 0, size = dest_rating_change.size();
		config::log("%d songs out of sync", size);
		for (std::list<song_ratings_t>::const_iterator
				 iter = dest_rating_change.begin();
			 iter != dest_rating_change.end(); iter++) {
			config::log("  %d/%d %s: %d -> %d",
						i++, size, iter->path.c_str(),
						str(iter->rating_old).c_str(),
						str(iter->rating_new).c_str());
		}
	}
}

bool ratesync::Updater::Apply() {
	for (std::list<song_ratings_t>::const_iterator
			 iter = dest_rating_change.begin();
		 iter != dest_rating_change.end(); iter++) {
		if (!dest->Set(*iter)) {
			return false;
		}
		config::debug("SET %s: %d -> %d",
					  iter->path.c_str(),
					  str(iter->rating_old).c_str(),
					  str(iter->rating_new).c_str());
	}

	return true;
}
