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
	class MpdError : public std::runtime_error {
	public:
	MpdError(const std::string& what) :
		std::runtime_error(what) { }
	};

	class MpdSong {
	public:
	MpdSong(struct mpd_connection* conn, const std::string& uri)
		: conn(conn), uri_(uri) { };

		const std::string& uri() const;
		unsigned int rating() const;

		void rating_clear();
		void rating_set(unsigned int rating);

		bool operator==(const MpdSong& s) const {
			return uri() == s.uri();
		}
		bool operator<(const MpdSong& s) const {
			return uri() < s.uri();
		}
	private:
		struct mpd_connection* conn;
		const std::string uri_;
	};

	class MpdAccess {
	public:
		static const int UNRATED = 0;//TODO replace with obj

		MpdAccess(const std::string& host);
		virtual ~MpdAccess();

		void songs(std::list<MpdSong>& out) const;
	private:
		const std::string host;
		struct mpd_connection* conn;
	};
}

#endif
