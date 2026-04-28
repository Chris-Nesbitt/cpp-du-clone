#include "duFunctions.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cstdint>

using namespace std;

namespace fs = filesystem;

//takes the args passed in and determines what booleans need to be turned on, the block size, what folders to use and help/version
Options parseArgs(int argc, char* argv[]) {
	Options options;
	bool blockSizeSet = false;
	bool kSet = false;

	//for all the individual white-space-separated args 
	for (int i = 1; i < argc; ++i) {
		string arg = argv[i];

		//checks to see if the arg is --help
		if (arg == "--help") {
			options.help = true;
		}

		//checks to see if the arg is --version
		else if (arg == "--version") {
			options.version = true;
		}

		//checks to see if the arg is --block-size=ddd
		else if (arg.substr(0, 13) == "--block-size=") { //reads the first 13 characters (the exact length of "--block-size=")
			string val = arg.substr(13); //reads what comes after "--block-size="

			//if there is nothing after the =
			if (val.empty()) {
				cout << "Error: block-size value is invalid <>\n";
				exit(1);
			}

			uintmax_t bs = 0;
			//turns the block size to an int
			try {
				bs = stoull(val);
			}

			//if a non-int is given
			catch (...) {
				cout << "Error: block-size value is invalid <" + val + ">";
				exit(1);
			}

			//if the int is 0
			if (bs == 0) {
				cout << "Error: block-size value too small <" + val + ">";
				exit(1);
			}

			//if k and block-size are both attempted to be used
			if (kSet) {
				cout << "Error: -k and --block-size are incompatible.\n";
				exit(1);
			}

			//commiting the block size
			options.blockSize = bs;
			blockSizeSet = true;
		}

		//checking for all the '-[skhznrb] args
		else if (arg[0] == '-' && arg.size() > 1 && arg[1] != '-') { //making sure the input isn't just '-' or some '--' that isn't help, version,etc
			//reads every character in the arg after the '-', flipping the relevant booleans
			for (size_t j = 1; j < arg.size(); ++j) {
				char c = arg[j];
				switch (c) {

					//-s
				case 's':
					options.s = true;
					break;

					//-k
				case 'k':

					//error if block-size and k in same command line args
					if (blockSizeSet) {
						cout << "Error: -k and --block-size are incompatible.\n";
						exit(1);
					}
					options.k = true;
					options.blockSize = 1024;
					kSet = true;
					break;

					//-h
				case 'h':

					//error if -b and -h in same command line args
					if (options.b) {
						cout << "Error: cannot use both -b and -h\n";
						exit(1);
					}
					options.h = true;
					break;

					//-z
				case 'z':

					//error if -n and -z in same command line args
					if (options.n) {
						cout << "Error: -n and -z switches are incompatible.\n";
						exit(1);
					}
					options.z = true;
					break;

					//-n
				case 'n':

					//error if -z and -n in same command line args
					if (options.z) {
						cout << "Error: -n and -z switches are incompatible.\n";
						exit(1);
					}
					options.n = true;
					break;

					//-r
				case 'r':
					options.r = true;
					break;

					//-b
				case 'b':

					//error if -h and -b in same command line args
					if (options.h) {
						cout << "Error: cannot use both -b and -h\n";
						exit(1);
					}
					options.b = true;
					break;

					//if some non-command letter is put after '-'
				default:
					cout << "Error: unknown switches: <" << c << ">\n";
					exit(1);
				}
			}
		}

		//any command line arg that isn't checked above defaults to being an attempted folder to parse
		else {
			options.folders.push_back(arg);
		}
	}

	//if no folders are given, default to the current directory
	if (options.folders.empty()) {
		options.folders.push_back(".");
	}

	return options;
}

//takes a size in bytes and rounds up it to the nearest whole multiple of blockSize (unless bytes == 0)
uintmax_t roundUpToCluster(uintmax_t bytes, uintmax_t blockSize) {
	if (bytes == 0) {
		return 0;
	}
	return ((bytes + blockSize - 1) / blockSize) * blockSize;
}

//returns the sum of the file sizes (properly rounded) of ALL files in the directory (including those in sub-directories), skipping un-readable files/directories
uintmax_t sumDir(const fs::path& dir, uintmax_t blockSize) {
	uintmax_t total = 0;

	try {
		//goes through all files and sub-directories
		for (auto& entry : fs::recursive_directory_iterator(dir)) {
			try {
				//if it is a file(not a directory --> iterator will enter any directories to parse their files)
				if (entry.is_regular_file()) {

					//add the size of the file rounded using roundUpToCluster
					uintmax_t sz = entry.file_size();
					total += roundUpToCluster(sz, blockSize);
				}
			}
			catch (...) {}// if the file can't be read            
		}
	}
	catch (...) {}//if the whole directory can't be read

	return total;
}

//converts the size units to the correct format (does not handle -h)
uintmax_t toDisplayUnits(uintmax_t bytes, const Options& options) {
	//does nothing if bytes is the format (thats the internal storage unit)
	if (options.b) {
		return bytes;
	}

	//turns bytes into clusters if format is the default
	return (bytes + options.blockSize - 1) / options.blockSize;
}

//converts the number of bytes into a human-readable string with a unit at the end
string formatHuman(uintmax_t bytes) {
	//the units table
	static const string units[] = { "B", "K", "M", "G", "T", "P", "E" };
	int i = 0;
	double size = bytes;

	//determines what unit to use based on what 2^x threshold it has reached
	//--> ends at exabytes because WHO has that much storage
	while (size >= 1024 && i < (sizeof(units) / sizeof(units[0]) - 1)) {
		size /= 1024;
		i++;
	}

	ostringstream oss;

	//if the magnitude is < 10.0 --> 1 decimal places
	if (size < 10.0) {
		oss << fixed << setprecision(1) << size << units[i];
	}

	//if the magnitude is >= 10.0 --> 0 decimal places
	else {
		oss << fixed << setprecision(0) << size << units[i];
	}

	//returning the stream as a string
	return oss.str();
}

//decides how to format the file sizes based on the booleans flipped in options
string formatSize(uintmax_t clusterBytes, const Options& options, int numWidth) {
	//runs the human-readable if -h
	if (options.h) {
		return formatHuman(clusterBytes);
	}

	//otherwise lets toDisplayUnits decide between -b and cluster count
	uintmax_t val = toDisplayUnits(clusterBytes, options);

	//adds padding so the number aligns with numWidth characters(the number of digits in the largest number being output)
	ostringstream oss;
	oss << setw(numWidth) << val;
	return oss.str();
}

//gathers the data of all the directories that need to be output and stores them in a vector of FolderEntry structs (their path and file size)
vector<FolderEntry> scanFolder(const fs::path& root, const Options& options) {
	//ensures the given directory both exists and is actually a directory
	if (!fs::exists(root) || !fs::is_directory(root))
		throw runtime_error("Cannot access '" + root.string() + "': No such directory");

	//a vector storing the information of all directories (root and immediate sub-directories)
	vector<FolderEntry> results;

	//Summary (-s) --> only shows the total of the directory (calls sumDir)
	if (options.s) {
		// Summary mode: only the root with its full recursive total
		results.push_back({ root, sumDir(root, options.blockSize) });
	}

	//if -s is not used
	else {
		uintmax_t rootTotal = 0;
		try {
			//looks at all the entries one level down from the root --> skips unreadable files/directories
			for (const auto& entry : fs::directory_iterator(root,
				fs::directory_options::skip_permission_denied)) {
				try {
					//if the entry is a file --> add its rounded size to the total of the directory
					if (entry.is_regular_file()) {
						uintmax_t sz = entry.file_size();
						rootTotal += roundUpToCluster(sz, options.blockSize);
					}
					//if the entry is a directory --> find the size of that sub-directory and add it to the total
					else if (entry.is_directory()) {
						uintmax_t sub = sumDir(entry.path(), options.blockSize);
						rootTotal += sub;
						results.push_back({ entry.path(), sub });
					}
				}
				catch (...) {} // Skips any unreadable files or directories             
			}
		}
		catch (...) {} // Skip unreadable root directory entries

		// adding the data of the root directory
		results.push_back({ root, rootTotal });
	}

	return results;
}

void printHelp() {
	cout <<
		"du (c) 2018-26, Garth Santor\n"
		"===========================================================\n"
		"Version 1.0.0\n"
		"A disk usage utility inspired by the UNIX du command.\n"
		"Usage: du [-skhznrb] [--help] [--version] [--block-size=dddd] [folder]*\n"
		"Examples:\n"
		"  du\n"
		"    > display the sum of the cluster sizes of each directory\n"
		"      starting the cwd\n"
		"  du folder\n"
		"    > display the sum of the cluster sizes of each directory\n"
		"      starting with 'folder'\n"
		"  du -h\n"
		"    > display the results in a human readable form\n"
		"\n"
		"  du -s\n"
		"    > display only the final summary\n"
		"\n"
		"  du -b\n"
		"    > display in bytes\n"
		"\n"
		"  du -k\n"
		"    > cluster size is 1024\n"
		"\n"
		"  du -z\n"
		"    > display the list sorted by size\n"
		"\n"
		"  du -n\n"
		"    > display the list sorted by name\n"
		"\n"
		"  du -r\n"
		"    > display the list in reverse order\n"
		"\n"
		"  du --block-size=dddd\n"
		"    > set the cluster size to the specified integer > 0\n"
		"\n"
		"  du --help\n"
		"    > displays the help\n"
		"\n"
		"  du --version\n"
		"    > displays version number in the format d.d.d\n";
}

//sorts the entries based on the flipped booleans
void sortEntries(vector<FolderEntry>& entries, const Options& options) {

	//sorted by size (-z)
	if (options.z) {
		sort(entries.begin(), entries.end(),

			//lambda --> performing the same function as an operator overload for < comparing size
			[](const FolderEntry& a, const FolderEntry& b) {
				return a.clusterBytes < b.clusterBytes;
			});
	}

	//sorted by name (-n)
	else if (options.n) {
		sort(entries.begin(), entries.end(),

			//lambda --> performing the same function as an operator overload for < comparing path name
			[](const FolderEntry& a, const FolderEntry& b) {
				return a.path.string() < b.path.string();
			});
	}

	//lamdas had to be used because I can't overload < in two different ways

	//reverse sorted (-r)
	if (options.r)
		reverse(entries.begin(), entries.end());
}

//returns the required size of the column to format output properly 
int getMaxWidth(const vector<FolderEntry>& entries, const Options& options) {
	// Dynamic numeric column width
	int numWidth = 1;
	//if the display type is human-readable (-h) then this doesn't apply
	if (!options.h) {
		//loops through all the entries to find the file size that has the most characters
		for (auto& e : entries) {
			int w = static_cast<int>(to_string(toDisplayUnits(e.clusterBytes, options)).size());

			//find the largest file size (in # of digits)
			if (w > numWidth) {
				numWidth = w;
			}
		}
	}

	return numWidth;
}

//prints the formatted entries (size and path) to the console
void printEntries(const vector<FolderEntry>& entries, const Options& options, int numWidth) {
	for (auto& e : entries) {
		cout << formatSize(e.clusterBytes, options, numWidth) << "   " << e.path.string() << "\n";
	}
}
