#ifndef RATESYNC_CONFIG_H
#define RATESYNC_CONFIG_H

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

#cmakedefine USE_MPDCLIENT

namespace ratesync {
	namespace config {
		static const int
			VERSION_MAJOR = @ratesync_VERSION_MAJOR@,
			VERSION_MINOR = @ratesync_VERSION_MINOR@,
			VERSION_PATCH = @ratesync_VERSION_PATCH@;

#ifdef USE_MPDCLIENT
		static const char* VERSION_STRING = "@ratesync_VERSION_MAJOR@.@ratesync_VERSION_MINOR@.@ratesync_VERSION_PATCH@-mpd";
#else
		static const char* VERSION_STRING = "@ratesync_VERSION_MAJOR@.@ratesync_VERSION_MINOR@.@ratesync_VERSION_PATCH@-nompd";
#endif
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
