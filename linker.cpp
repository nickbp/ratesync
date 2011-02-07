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

#include "linker.h"
#include "config.h"

#include <sstream>
#include <iostream>

namespace {
	using mpdtagger::song_t;
	using mpdtagger::rating_t;

	using mpdtagger::song_rating_t;
	using mpdtagger::song_ratings_t;

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
						rating_change.push_back(song_ratings_t(song,
															   std::make_pair(mpd_rating,
																			  file_rating)));
					}
				} else {
					//set mpd rating
					unrated_to_rating.push_back(song_rating_t(song,
															  file_rating));
				}
			} else if (file_unrated.find(song) != file_unrated.end()) {
				//file is unrated (or rating couldnt be parsed)
				if (mpd_rating_iter != mpd_ratings.end()) {
					rating_t mpd_rating = mpd_rating_iter->second;
					//clear mpd rating
					rating_to_unrated.push_back(song_rating_t(song,
															  mpd_rating));
				}
			}
		}		
	}
}

bool mpdtagger::Linker::calculate_changes() {
	//get all symlinks and their ratings:
	std::map<symlink_t, rating_t> symlink_ratings;
	std::list<symlink_t> symlink_dangling;
	{
		symlink::Access symlink(out_dir);
		try {
			symlink.symlinks(symlink_ratings, symlink_dangling);
		} catch (const symlink::Error& err) {
			throw LinkerError(err.what());
		}
	}

	//get local file metadata ratings for everything in the directory:
	std::map<song_t, rating_t> file_ratings;
	std::set<song_t> file_unrated;
	{
		media::Access media(in_dir);
		try {
			media.ratings(file_ratings, file_unrated);
		} catch (const media::Error& err) {
			throw LinkerError(err.what());
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
	return false;
}

void mpdtagger::Linker::print_changes() const {
	bool printed = false;
	if (!unrated_to_rating.empty()) {
		printed = true;
		config::log("%d files unlinked in output directory, rated on disk",
					unrated_to_rating.size());
		int i = 0;
		for (std::list<symlink_rating_t>::const_iterator iter
				 = unrated_to_rating.begin();
			 iter != unrated_to_rating.end(); iter++) {
			config::log("  %d/%d %s: unrated -> %d",
						i++, unrated_to_rating.size(),
						iter->first.first.c_str(), iter->second);
		}
	}

	if (!rating_to_unrated.empty()) {
		if (printed) {
			config::log("");
		}
		printed = true;
		config::log("%d files linked in output directory, unrated on disk",
					rating_to_unrated.size());
		int i = 0;
		for (std::list<symlink_rating_t>::const_iterator iter
				 = rating_to_unrated.begin();
			 iter != rating_to_unrated.end(); iter++) {
			config::log("  %d/%d %s: %d -> unrated",
						i++, rating_to_unrated.size(),
						iter->first.first.c_str(), iter->second);
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
		for (std::list<symlink_ratings_t>::const_iterator iter
				 = rating_change.begin();
			 iter != rating_change.end(); iter++) {
			config::log("  %d/%d %s: %d -> %d",
						i++, rating_change.size(),
						iter->first.first.c_str(),
						iter->second.first, iter->second.second);
		}
	}

	if (!printed) {
		config::log("No changes to be made.");
	} else {
		config::log("");
		config::log("%d total changes to be made in output directory.",
					unrated_to_rating.size() + rating_to_unrated.size() +
					rating_change.size());
	}
}

void mpdtagger::Linker::apply_changes() {
	symlink::Access symlink(out_dir);

	for (std::list<symlink_rating_t>::const_iterator iter
			 = unrated_to_rating.begin();
		 iter != unrated_to_rating.end(); iter++) {
		try {
			symlink.symlink_set(iter->first, iter->second);
		} catch (const symlink::Error& err) {
			throw LinkerError(err.what());
		}
		config::debug("SET %s: UNRATED -> %d",
					  iter->first.first.c_str(), iter->second);
	}

	for (std::list<symlink_rating_t>::const_iterator iter
			 = rating_to_unrated.begin();
		 iter != rating_to_unrated.end(); iter++) {
		try {
			symlink.symlink_clear(iter->first);
		} catch (const symlink::Error& err) {
			throw LinkerError(err.what());
		}
		config::debug("SET %s: %d -> UNRATED",
					  iter->first.first.c_str(), iter->second);
	}

	for (std::list<symlink_ratings_t>::const_iterator iter
			 = rating_change.begin();
		 iter != rating_change.end(); iter++) {
		try {
			symlink.symlink_set(iter->first, iter->second.second);
		} catch (const symlink::Error& err) {
			throw LinkerError(err.what());
		}
		config::debug("SET %s: %d -> %d",
					  iter->first.first.c_str(),
					  iter->second.first, iter->second.second);
	}
}
