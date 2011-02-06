#ifndef MPDTAGGER_TAGGER_H
#define MPDTAGGER_TAGGER_H

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

#include <stdexcept>
#include <string>

#include "mpdaccess.h"
#include "mediaaccess.h"

namespace mpdtagger {
	class TaggerError : public std::runtime_error {
	public:
	TaggerError(const std::string& what) :
		std::runtime_error(what) { }
	};

	class Tagger {
	public:
	Tagger(std::string mpd_host, size_t mpd_port,
		   std::string music_dir)
		: dir(music_dir), host(mpd_host), port(mpd_port) { }

		bool calculate_changes();
		void print_changes() const;
		void apply_changes();

	private:
		const std::string dir, host;
		const size_t port;

		typedef std::pair<mpd::song_t, rating_t> song_rating_t;
		typedef std::pair<mpd::song_t, std::pair<rating_t, rating_t> > song_ratings_t;

		std::list<song_rating_t> unrated_to_rating;
		std::list<song_rating_t> rating_to_unrated;
		std::list<song_ratings_t> rating_change;
	};
}

#endif
