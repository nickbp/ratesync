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

#include <mpd/client.h>

namespace mpdtagger {

	typedef size_t rating_t;

	namespace mpd {
		class Error : public std::runtime_error {
		public:
		Error(const std::string& what) :
			std::runtime_error(what) { }
		};

		class Song {
		public:
		Song(struct mpd_connection* conn, const std::string& uri)
			: conn(conn), uri_(uri) { };

			const std::string& uri() const;
			bool rating(rating_t& out) const;

			void rating_clear();
			void rating_set(rating_t rating);

			bool operator==(const Song& s) const {
				return uri() == s.uri();
			}
			bool operator<(const Song& s) const {
				return uri() < s.uri();
			}
		private:
			struct mpd_connection* conn;
			const std::string uri_;
		};

		class Access {
		public:
			Access(const std::string& host, size_t port);
			virtual ~Access();

			void songs(std::list<Song>& out) const;
		private:
			const std::string host;
			struct mpd_connection* conn;
		};
	}
}

#endif
