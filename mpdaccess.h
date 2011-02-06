#ifndef MPDTAGGER_MPDACCESS_H
#define MPDTAGGER_MPDACCESS_H

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
#include <list>
#include <map>
#include <set>

struct mpd_connection;

namespace mpdtagger {

	typedef size_t rating_t;

	namespace mpd {
		class Error : public std::runtime_error {
		public:
		Error(const std::string& what) :
			std::runtime_error(what) { }
		};

		typedef std::string song_t;

		class Access {
		public:
			Access(const std::string& host, size_t port);
			virtual ~Access();

			void connect();

			void ratings(std::list<song_t>& out_all,
						 std::map<song_t,rating_t>& out_rating) const;

			void rating_clear(const song_t& song);
			void rating_set(const song_t& song, rating_t rating);
		private:
			const std::string host;
			const size_t port;
			struct mpd_connection* conn;
		};
	}
}

#endif
