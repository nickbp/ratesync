#ifndef RATESONG_TAGGER_H
#define RATESONG_TAGGER_H

/*
  ratesong - Synchronizes metadata between MPD stickers and media files.
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

#include "mpd-access.h"
#include "media-access.h"

namespace ratesong {
	class Tagger {
	public:
	Tagger(const std::string& mpd_host, size_t mpd_port,
		   const std::string& music_dir)
		: dir(music_dir), host(mpd_host), port(mpd_port) { }

		bool calculate_changes();
		bool has_changes() const;
		void print_changes() const;
		bool apply_changes();

	private:
		const std::string dir, host;
		const size_t port;

		std::list<song_rating_t> unrated_to_rating;
		std::list<song_rating_t> rating_to_unrated;
		std::list<song_ratings_t> rating_change;
	};
}

#endif
