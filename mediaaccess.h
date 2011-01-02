#ifndef MPDTAGGER_MEDIAACCESS_H
#define MPDTAGGER_MEDIAACCESS_H

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

#include "mpdaccess.h"

#include <stdexcept>
#include <string>
#include <map>

namespace mpdtagger { 
	class MediaError : public std::runtime_error {
	public:
	MediaError(const std::string& what) :
		std::runtime_error(what) { }
	};

	class MediaFile {
	public:
	MediaFile(const std::string& path)
		: path(path) { }

		unsigned int rating();
	private:
		const std::string path;
	};

	class MediaAccess {
	public:
		static const int UNRATED = 0;//TODO replace with obj
		
		MediaAccess(const std::string& dir);

		void ratings(const std::list<MpdSong>& mpd_songs,
					 std::map<MpdSong,int>& out);
	private:
		bool check_file(const std::string& filepath, bool isdir, bool throw_err = false);

		const std::string dir;
	};
}

#endif
