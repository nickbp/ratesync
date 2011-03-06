#ifndef RATESONG_CONFIG_H
#define RATESONG_CONFIG_H

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

namespace ratesong {
	namespace config {
		static const int
			VERSION_MAJOR = @ratesong_VERSION_MAJOR@,
			VERSION_MINOR = @ratesong_VERSION_MINOR@,
			VERSION_PATCH = @ratesong_VERSION_PATCH@;

		static const char* VERSION_STRING = "@ratesong_VERSION_MAJOR@.@ratesong_VERSION_MINOR@.@ratesong_VERSION_PATCH@";
		static const char* BUILD_DATE = __TIMESTAMP__;

		extern bool debug_enabled;

		void debug(const char* format, ...);
		void debugnn(const char* format, ...);
		void log(const char* format, ...);
		void lognn(const char* format, ...);
		void error(const char* format, ...);
		void errornn(const char* format, ...);
	}
}

#endif
