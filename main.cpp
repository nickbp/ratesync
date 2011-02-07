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
#include "linker.h"
#include "config.h"

using mpdtagger::config::error;
using mpdtagger::config::log;
using mpdtagger::config::debug;

#define DEFAULT_MPD_HOST "localhost"
#define DEFAULT_MPD_PORT 6600

namespace {
	bool help_cmd = false, sort_cmd = false, tag_cmd = false,
		no_confirm = false;
	std::string mpd_host = DEFAULT_MPD_HOST, music_dir, output_dir;
	size_t mpd_port = DEFAULT_MPD_PORT;
}

void syntax(char* appname) {
	error("MPD Tagger v%s (built %s)",
		  mpdtagger::config::VERSION_STRING,
		  mpdtagger::config::BUILD_DATE);
	error("Usage: %s [options] <command> <musicdir>", appname);
	error("Commands:");
	error("  tompd     Store song rating metadata into MPD database.");
	error("  linksort  Create symlinks to files, grouped according to their ratings.");
	error("");
	error("Common Options:");
	error("  -h/--help        This help text.");
	error("  -v/--verbose     Show verbose output.");
	error("  -n/--no-confirm  Don't confirm changes before applying them.");
	error("");
	error("tompd Command Options:");
	error("  -m/--mpd-host <host[:port]>  MPD host/port. (default %s:%d)",
		  DEFAULT_MPD_HOST, DEFAULT_MPD_PORT);
	error("");
	error("linksort Command Options:");
	error("  -o/--output-dir <path>  Where to put sorted files/symlinks.");
	error("                          (default: <musicdir>/rating)");
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
			{"no-confirm", 0, NULL, 'n'},
			{"mpd-host", 1, NULL, 'm'},
			{"output-dir", 1, NULL, 'o'},
			{0,0,0,0}
		};

		int option_index = 0;
		c = getopt_long(argc, argv, "hvnm:o:",
						long_options, &option_index);
		if (c == -1) {//unknown arg (doesnt match -x/--x format)
			if (optind >= argc) {
				//at end of successful parse
				break;
			}
			//getopt refuses to continue, so handle command / musicdir manually:
			for (int i = optind; i < argc; ++i) {
				const char* arg = argv[i];
				debug("%d %d %s", argc, i, arg);
				if (!tag_cmd && !sort_cmd) {
					if (strcmp(arg, "tompd") == 0) {
						tag_cmd = true;
					} else if (strcmp(arg, "linksort") == 0) {
						sort_cmd = true;
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
		case 'n':
			no_confirm = true;
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
		default:
			syntax(argv[0]);
			return false;
		}
	}

	if (!tag_cmd && !sort_cmd) {
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
	debug("  no-confirm: %d", no_confirm);
	debug("mpdtag opts (%s)", (tag_cmd ? "enabled" : "disabled"));
	debug("  mpd-host: %s (port %d)", mpd_host.c_str(), mpd_port);
	debug("move opts (%s)", (sort_cmd ? "enabled" : "disabled"));
	debug("  output-dir: %s", output_dir.c_str());

	return true;
}

bool promptYN(const std::string& question) {
	mpdtagger::config::lognn("%s (y/N): ", question.c_str());
	std::string response;
	std::getline(std::cin, response);
	return (!response.empty() &&
			(response[0] == 'y' || response[0] == 'Y'));
}

int main(int argc, char* argv[]) {
	if (!parse_config(argc, argv)) {
		return 1;
	}

	if (help_cmd) {
		syntax(argv[0]);
		return 0;
	} else if (tag_cmd) {
		mpdtagger::Tagger tagger(mpd_host, mpd_port, music_dir);
		log("Calculating tags...");
		try {
			if (tagger.calculate_changes()) {
				log("The following changes are about to be applied to your MPD database:");
				tagger.print_changes();
				if (no_confirm ||
					promptYN("Continue with these changes to your MPD database?")) {
					log("Applying changes...");
					tagger.apply_changes();
					log("Complete.");
				}
			} else {
				log("Your MPD database is up to date.");
			}
		} catch (const mpdtagger::TaggerError& err) {
			error(err.what());
			return 1;
		}
	} else if (sort_cmd) {
		if (output_dir.length() == 0) {
			output_dir = music_dir;//TODO append /rating
		}
		mpdtagger::Linker linker(music_dir, output_dir);
		log("Calculating symlinks...");
		try {
			if (linker.calculate_changes()) {
				log("The following changes are about to be applied to your output directory:");
				linker.print_changes();
				if (no_confirm ||
					promptYN("Continue with these changes to your output directory?")) {
					log("Applying changes...");
					linker.apply_changes();
					log("Complete.");
				}
			} else {
				log("Your output directory is up to date.");
			}
		} catch (const mpdtagger::LinkerError& err) {
			error(err.what());
			return 1;
		}
	}
	return 0;
}
