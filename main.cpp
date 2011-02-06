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

#include <getopt.h>
#include <string.h>
#include <sys/stat.h>

#include "tagger.h"
#include "config.h"

using mpdtagger::config::error;
using mpdtagger::config::log;
using mpdtagger::config::debug;

#define DEFAULT_MPD_HOST "localhost"
#define DEFAULT_MPD_PORT 6600

namespace {
	bool help_cmd = false, move_cmd = false, tag_cmd = false, output_copy = false;
	std::string mpd_host = DEFAULT_MPD_HOST, music_dir, output_dir;
	size_t mpd_port = DEFAULT_MPD_PORT;
}

void syntax(char* appname) {
	error("MPD Tagger v%s (built %s)",
		  mpdtagger::config::VERSION_STRING,
		  mpdtagger::config::BUILD_DATE);
	error("Usage: %s [options] <command> <musicdir>", appname);
	error("Commands:");
	error("  tag   Store song rating metadata in mpd tag database.");
	error("  move  Sort files into numbered directories according to their ratings.");
	error("");
	error("Common Options:");
	error("  -h/--help     This help text.");
	error("  -v/--verbose  Show verbose output.");
	error("");
	error("Tag Options:");
	error("  -m/--mpd-host <host[:port]>  MPD host/port. (default %s:%d)",
		  DEFAULT_MPD_HOST, DEFAULT_MPD_PORT);
	error("");
	error("Move Options:");
	error("  -o/--output-dir <path>  Where to move/copy sorted files. Required.");
	error("  -c/--output-copy        Copy files instead of moving them.");
}

bool check_dir(const char* dirpath, bool check_write = false) {
	struct stat dirstat;
	if (stat(dirpath, &dirstat) != 0) {
		error("Unable to get status for directory '%s'", dirpath);
		return false;
	}
	if (!S_ISDIR(dirstat.st_mode)) {
		error("'%s' is not a directory.", dirpath);
		return false;
	}
	//TODO need to test how things act with exec off:
	if (check_write && access(dirpath, W_OK) != 0) {
		error("Directory '%s' does not allow write access.", dirpath);
		return false;
	} else if (access(dirpath, X_OK | R_OK) != 0) {
		//need to ls music dir with move command
		error("Directory '%s' does not allow read+execute access.", dirpath);
		return false;
	}
	return true;
}

bool parse_config(int argc, char* argv[]) {
	if (argc == 1) {
		syntax(argv[0]);
		return false;
	}
	int c;
	while (1) {
		static struct option long_options[] = {
			{"help", 0, NULL, 'h'},
			{"verbose", 0, NULL, 'v'},
			{"mpd-host", 1, NULL, 'm'},
			{"output-dir", 1, NULL, 'o'},
			{"output-copy", 0, NULL, 'c'},
			{0,0,0,0}
		};

		c = getopt_long(argc, argv, "hvm:o:c",
						long_options, NULL);
		if (c == -1) {//unknown arg (doesnt match -x/--x format)
			if (optind >= argc) {
				//at end of successful parse
				break;
			}
			//getopt refuses to continue, so handle command / musicdir manually:
			for (int i = optind; i < argc; ++i) {
				const char* arg = argv[i];
				debug("%d %d %s", argc, i, arg);
				if (!tag_cmd && !move_cmd) {
					if (strcmp(arg, "tag") == 0) {
						tag_cmd = true;
					} else if (strcmp(arg, "move") == 0) {
						move_cmd = true;
					} else {
						error("%s: unknown argument: '%s'", argv[0], argv[i]);
						syntax(argv[0]);
						return false;
					}
				} else {
					if (music_dir.length() > 0) {
						error("%s: unknown argument: '%s'", argv[0], argv[i]);
						syntax(argv[0]);
						return false;
					}
					if (!check_dir(arg)) {
						return false;
					}
					music_dir = std::string(arg);
				}
			}
			break;
		}

		switch (c) {
		case 'h':
			help_cmd = true;
			return true;
		case 'v':
			mpdtagger::config::debug_enabled = true;
			break;
		case 'm':
			{
				std::string host_port(optarg);
				size_t pos = host_port.find_first_of(':');
				if (pos == std::string::npos) {
					mpd_host = host_port;
				} else {
					mpd_host = host_port.substr(0,pos);
					std::string port_str = host_port.substr(pos+1);
					std::stringstream ss(port_str);
					if ((ss >> mpd_port).fail()) {
						error("%s: invalid port '%s' -> '%s'",
							  argv[0], host_port.c_str(), port_str.c_str());
						return false;
					}
				}
			}
			break;
		case 'o':
			if (!check_dir(optarg, true)) {
				return false;
			}
			output_dir = std::string(optarg);
			break;
		case 'c':
			output_copy = true;
			break;
		default:
			syntax(argv[0]);
			return false;
		}
	}

	if (!tag_cmd && !move_cmd) {
		error("%s: no command specified", argv[0]);
		syntax(argv[0]);
		return false;
	}
	if (music_dir.length() == 0) {
		error("%s: no music directory specified", argv[0]);
		syntax(argv[0]);
		return false;
	}

	debug("common opts:");
	debug("  music-dir: %s", music_dir.c_str());
	debug("mpdtag opts (%s)", (tag_cmd ? "enabled" : "disabled"));
	debug("  mpd-host: %s (port %d)", mpd_host.c_str(), mpd_port);
	debug("move opts (%s)", (move_cmd ? "enabled" : "disabled"));
	debug("  output-copy: %s", (output_copy ? "enabled" : "disabled"));
	debug("  output-dir: %s", output_dir.c_str());

	return true;
}

int main(int argc, char* argv[]) {
	if (!parse_config(argc, argv)) {
		return 1;
	}

	if (help_cmd) {
		syntax(argv[0]);
		return 0;
	} else if (tag_cmd) {
		try {
			mpdtagger::Tagger tagger(mpd_host, mpd_port, music_dir);
			tagger.file_to_db();
		} catch (const mpdtagger::TaggerError& err) {
			error(err.what());
			return 1;
		}
	} else if (move_cmd) {
		if (output_dir.length() == 0) {
			error("%s: output directory required for move command", argv[0]);
			return 1;
		}
		log("file move support WIP");//TODO
		//output_copy, output_dir
	}
	return 0;
}
