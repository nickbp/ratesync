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

#include "config.h"

#include <stdio.h>
#include <stdarg.h>

namespace ratesong {
	namespace config {
		bool debug_enabled = false;

		void debug(const char* format, ...) {
			if (debug_enabled) {
				va_list args;
				va_start(args, format);
				vfprintf(stdout, format, args);
				va_end(args);
				fprintf(stdout, "\n");
			}
		}
		void debugnn(const char* format, ...) {
			if (debug_enabled) {
				va_list args;
				va_start(args, format);
				vfprintf(stdout, format, args);
				va_end(args);
			}
		}

		void log(const char* format, ...) {
			va_list args;
			va_start(args, format);
			vfprintf(stdout, format, args);
			va_end(args);
			fprintf(stdout, "\n");
		}
		void lognn(const char* format, ...) {
			va_list args;
			va_start(args, format);
			vfprintf(stdout, format, args);
			va_end(args);
		}

		void error(const char* format, ...) {
			va_list args;
			va_start(args, format);
			vfprintf(stderr, format, args);
			va_end(args);
			fprintf(stderr, "\n");
		}
		void errornn(const char* format, ...) {
			va_list args;
			va_start(args, format);
			vfprintf(stderr, format, args);
			va_end(args);
		}
	}
}
