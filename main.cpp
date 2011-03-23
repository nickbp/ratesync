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

#include <string>
#include <sstream>
#include <iostream>

#include <getopt.h>
#include <string.h>
#include <sys/stat.h>

#include "updater.h"

#include "sink-file.h"
#include "sink-mpd.h"
#include "sink-symlink.h"

#include "config.h"

using ratesync::config::error;
using ratesync::config::log;
using ratesync::config::debug;

#define DEFAULT_MPD_HOST "localhost"
#define DEFAULT_MPD_PORT 6600

namespace {
	enum CMD { UNKNOWN, HELP, SYMLINK, MPD };
	CMD run_cmd = UNKNOWN;
	bool no_confirm = false;
	std::string mpd_host = DEFAULT_MPD_HOST, music_dir, symlink_dir;
	size_t mpd_port = DEFAULT_MPD_PORT;
}

void syntax(char* appname) {
	error("ratesync v%s (built %s)",
		  ratesync::config::VERSION_STRING,
		  ratesync::config::BUILD_DATE);
	error("Usage: %s [options] <command> <musicdir>", appname);
	error("Commands:");
	error("  mpd     Store song rating metadata into MPD database.");
	error("  links   Create symlinks to files, grouped according to their ratings.");
	error("");
	error("Common Options:");
	error("  -h/--help        This help text.");
	error("  -v/--verbose     Show verbose output.");
	error("  -n/--no-confirm  Don't confirm changes before applying them.");
	error("");
	error("mpd Command Options:");
	error("  -m/--mpd-host <host[:port]>  MPD host/port. (default %s:%d)",
		  DEFAULT_MPD_HOST, DEFAULT_MPD_PORT);
	error("");
	error("links Command Options:");
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

void format_dir(std::string& dir) {
	if (dir.length() > 0 && dir.at(dir.size()-1) != SEP) {
		dir += SEP;
	}
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
				if (run_cmd == UNKNOWN) {
					if (strcmp(arg, "mpd") == 0) {
						run_cmd = MPD;
					} else if (strcmp(arg, "links") == 0) {
						run_cmd = SYMLINK;
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
			run_cmd = HELP;
			return true;
		case 'v':
			ratesync::config::debug_enabled = true;
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
			symlink_dir = std::string(optarg);
			break;
		default:
			syntax(argv[0]);
			return false;
		}
	}

	if (music_dir.length() == 0) {
		error("%s: no music directory specified", argv[0]);
		syntax(argv[0]);
		return false;
	}
	format_dir(music_dir);

	debug("common opts:");
	debug("  music-dir: %s", music_dir.c_str());
	debug("  no-confirm: %d", no_confirm);
	debug("mpdtag opts (%s)", (run_cmd == MPD ? "enabled" : "disabled"));
	debug("  mpd-host: %s (port %d)", mpd_host.c_str(), mpd_port);
	debug("link opts (%s)", (run_cmd == SYMLINK ? "enabled" : "disabled"));
	debug("  symlink-dir: %s", symlink_dir.c_str());

	return true;
}

bool promptYN(const std::string& question) {
	ratesync::config::lognn("%s (y/N): ", question.c_str());
	std::string response;
	std::getline(std::cin, response);
	return (!response.empty() &&
			(response[0] == 'y' || response[0] == 'Y'));
}

int main(int argc, char* argv[]) {
	if (!parse_config(argc, argv)) {
		return 1;
	}

	std::string dest_label;
	ratesync::ISink *in_ptr = NULL, *out_ptr = NULL;
	switch (run_cmd) {
	case HELP:
		syntax(argv[0]);
		return 0;
	case MPD:
		dest_label = "MPD database";
		in_ptr = new ratesync::sink::File(music_dir);
		out_ptr = new ratesync::sink::Mpd(mpd_host, mpd_port);
		break;
	case SYMLINK:
		dest_label = "symlink directory";
		in_ptr = new ratesync::sink::File(music_dir);

		if (symlink_dir.length() == 0) {
			symlink_dir = music_dir+"rating"+SEP;
		} else {
			format_dir(symlink_dir);
		}
		out_ptr = new ratesync::sink::Symlink(music_dir, symlink_dir);
		break;
	default:
		error("%s: no command specified", argv[0]);
		syntax(argv[0]);
		return 1;
	}

	int ret = 0;
	{
		ratesync::Updater updater(in_ptr, out_ptr);
		log("Calculating changes...");
		if (updater.Calculate()) {
			if (updater.HasChanges()) {
				log("The following changes are about to be applied to your %s:",
					dest_label.c_str());
				updater.Print();
				std::ostringstream txt;
				txt << "Continue with these changes to your " << dest_label << "?";
				if (no_confirm || promptYN(txt.str())) {
					log("Applying changes...");
					updater.Apply();
					log("Complete.");
				}
			} else {
				log("Your %s is up to date.", dest_label.c_str());
			}
		} else {
			log("Encountered error when calculating changes, giving up.");
			ret = 1;
		}
	}
	if (in_ptr != NULL) {
		delete in_ptr;
		in_ptr = NULL;
	}
	if (out_ptr != NULL) {
		delete out_ptr;
		out_ptr = NULL;
	}

	return ret;
}
