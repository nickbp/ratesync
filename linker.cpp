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

#include "linker.h"
#include "config.h"

#include <sstream>
#include <iostream>

namespace {
	using ratesync::song_t;
	using ratesync::rating_t;

	using ratesync::song_rating_t;
	using ratesync::song_ratings_t;

	void get_changes(const std::list<song_t>& mpd_songs,
					 const std::map<song_t, rating_t>& mpd_ratings,
					 const std::map<song_t, rating_t>& file_ratings,
					 const std::set<song_t>& file_unrated,
					 std::list<song_rating_t>& unrated_to_rating,
					 std::list<song_rating_t>& rating_to_unrated,
					 std::list<song_ratings_t>& rating_change) {
		for (std::list<song_t>::const_iterator it = mpd_songs.begin();
			 it != mpd_songs.end(); ++it) {
			const song_t& song = *it;
			std::map<song_t, rating_t>::const_iterator
				mpd_rating_iter = mpd_ratings.find(song),
				file_rating_iter = file_ratings.find(song);
			if (file_rating_iter != file_ratings.end()) {
				//file has a rating
				rating_t file_rating = file_rating_iter->second;
				if (mpd_rating_iter != mpd_ratings.end()) {
					//mpd has a rating
					rating_t mpd_rating = mpd_rating_iter->second;
					if (mpd_rating != file_rating) {
						//update mpd rating
						rating_change.push_back(song_ratings_t());
						song_ratings_t& srs = rating_change.back();
						srs.path = song;
						srs.rating_old = mpd_rating;
						srs.rating_new = file_rating;
					}
				} else {
					//set mpd rating
					unrated_to_rating.push_back(song_rating_t());
					song_rating_t& sr = unrated_to_rating.back();
					sr.path = song;
					sr.rating = file_rating;
				}
			} else if (file_unrated.find(song) != file_unrated.end()) {
				//file is unrated (or rating couldnt be parsed)
				if (mpd_rating_iter != mpd_ratings.end()) {
					rating_t mpd_rating = mpd_rating_iter->second;
					//clear mpd rating
					rating_to_unrated.push_back(song_rating_t());
					song_rating_t& sr = rating_to_unrated.back();
					sr.path = song;
					sr.rating = mpd_rating;
				}
			}
		}
	}
}

bool ratesync::Linker::calculate_changes() {
	//get all symlinks and their ratings:
	std::map<song_t,rating_t> symlink_ratings;
	std::list<symlink::symlink_t> symlink_dangling;
	{
		symlink::Access symlink(in_dir, out_dir);
		if (!symlink.symlinks(symlink_ratings, symlink_dangling)) {
			return false;
		}
	}

	//get local file metadata ratings for everything in the directory:
	std::map<song_t,rating_t> file_ratings;
	{
		media::Access media(in_dir);
		if (!media.ratings(file_ratings)) {
			return false;
		}
	}

	//states:
	// song has been deleted, link is dangling (delete symlink)
	// link is out of date -- song is now a different rating (move symlink)
	// link is out of date -- song is now unrated (delete symlink)
	// link is missing -- song has been added recently (add symlink)

	//TODO create symlink::Access (parallel equivalent to mpd::Access) to calculate where files would go and see if that's where they already are
	//need to watch out for recursive eg Music/1/orig_path/file.mp3 -> Music/1/1/orig_path/file.mp3
	//also need to check for files in output path
	return true;
}

bool ratesync::Linker::has_changes() const {
	return (!rating_change.empty() ||
			!rating_add.empty() || !rating_del.empty());
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

void ratesync::Linker::print_changes() const {
	bool printed = false;
	if (!rating_add.empty()) {
		printed = true;
		config::log("%d new files unlinked in output directory",
					rating_add.size());
		int i = 0;
		for (std::list<song_rating_t>::const_iterator iter
				 = rating_add.begin();
			 iter != rating_add.end(); iter++) {
			config::log("  %d/%d %s: %d",
						i++, rating_add.size(),
						iter->path.c_str(), str(iter->rating).c_str());
		}
	}

	if (!rating_del.empty()) {
		if (printed) {
			config::log("");
		}
		printed = true;
		config::log("%d stale files linked in output directory",
					rating_del.size());
		int i = 0;
		for (std::list<song_rating_t>::const_iterator iter
				 = rating_del.begin();
			 iter != rating_del.end(); iter++) {
			config::log("  %d/%d %s: %d",
						i++, rating_del.size(),
						iter->path.c_str(), str(iter->rating).c_str());
		}
	}

	if (!rating_change.empty()) {
		if (printed) {
			config::log("");
		}
		printed = true;
		config::log("%d files linked in output directory, rated differently on disk",
					rating_change.size());
		int i = 0;
		for (std::list<song_ratings_t>::const_iterator iter
				 = rating_change.begin();
			 iter != rating_change.end(); iter++) {
			config::log("  %d/%d %s: %s -> %s",
						i++, rating_change.size(),
						iter->path.c_str(),
						str(iter->rating_old).c_str(),
						str(iter->rating_new).c_str());
		}
	}

	if (!printed) {
		config::log("No changes to be made.");
	} else {
		config::log("");
		config::log("%d total changes to be made in output directory.",
					rating_add.size() + rating_del.size() + rating_change.size());
	}
}

bool ratesync::Linker::apply_changes() {
	symlink::Access symlink(in_dir, out_dir);

	for (std::list<song_rating_t>::const_iterator iter
			 = rating_add.begin(); iter != rating_add.end(); iter++) {
		if (!symlink.symlink_add(*iter)) {
			return false;
		}
		config::debug("ADD %s: %s", iter->path.c_str(),
					  str(iter->rating).c_str());
	}

	for (std::list<song_rating_t>::const_iterator iter
			 = rating_del.begin(); iter != rating_del.end(); iter++) {
		if (!symlink.symlink_clear(*iter)) {
			return false;
		}
		config::debug("DEL %s: %s", iter->path.c_str(),
					  str(iter->rating).c_str());
	}

	for (std::list<song_ratings_t>::const_iterator iter
			 = rating_change.begin();
		 iter != rating_change.end(); iter++) {
		if (!symlink.symlink_update(*iter)) {
			return false;
		}
		config::debug("UPDATE %s: %s -> %s",
					  iter->path.c_str(),
					  str(iter->rating_old).c_str(),
					  str(iter->rating_new).c_str());
	}

	return true;
}
