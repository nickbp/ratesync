#ifndef MPDTAGGER_CONFIG_H
#define MPDTAGGER_CONFIG_H

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

namespace mpdtagger {
	namespace config {
		static const int
			VERSION_MAJOR = @mpdtagger_VERSION_MAJOR@,
			VERSION_MINOR = @mpdtagger_VERSION_MINOR@,
			VERSION_PATCH = @mpdtagger_VERSION_PATCH@;

		const std::string version_string() {
			return "@mpdtagger_VERSION_MAJOR@.@mpdtagger_VERSION_MINOR@.@mpdtagger_VERSION_PATCH@";
		}

		const std::string build_date() {
			return __TIMESTAMP__;
		}

		//TODO clean this up a bit, make more like printf, convert all couts/cerrs to use these:

		static bool debug_enabled = false;

		void debug(const std::string& msg) {
			if (debug_enabled) {
				std::cout << msg << std::endl;
			}
		}

		void log(const std::string& msg) {
			std::cout << msg << std::endl;
		}

		void err(const std::string& msg) {
			std::cerr << msg << std::endl;
		}
	}
}

#endif
