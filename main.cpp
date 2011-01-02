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

#include <string>
#include <sstream>
#include <iostream>

#include "tagger.h"

void syntax(char* appname) {
	std::cerr << "Syntax: " << appname << " <mpdhost> <music directory>" << std::endl;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		syntax(argv[0]);
		return 1;
	}
	std::string host(argv[1]);
	std::ostringstream musicdir_ss;
	for (int i=2; i < argc; i++) {
		musicdir_ss << argv[i];
		if (i+1 < argc) {
			musicdir_ss << " ";
		}
	}
	std::string musicdir(musicdir_ss.str());
	musicdir += "/";

	try {
		mpdtagger::Tagger tagger(host, musicdir);
		tagger.run();
	} catch (const mpdtagger::TaggerError& err) {
		std::cerr << err.what() << std::endl;
		return 1;
	}
	return 0;
}
