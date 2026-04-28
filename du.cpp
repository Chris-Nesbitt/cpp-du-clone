//Chris Nesbitt
//2026-04-11
//A rendition of the UNIX 'du' command

/* ---------VERSION HISTORY---------
v1.0: Program now has a struct to store the various different commands as booleans
      created a method to read the command line args and extract the commands given
      Program now has function --help and --version commands
      PASS : #1, #2
v1.1: Program now has a diskScanner class that will iterate through the given folder(s) (default current dir)
      to find the size of it(them) and its(their) subfolders
      Program defaults to reporting sizes as cluster counts (4096 bytes)
      Output from diskScanner now formatted properly and rounded up to the next cluster count
      PASS : v1.0 + #0, #3, #4, #5, #6, #24, #25
v1.2: Program now can change block size using --block-size=dddd
      Bad block size throws an error
      Pass v1.1 + #20, #21, #23
v1.3: Program will not allow conflicting switches and reports errors for bad args
      PASS v1.2 + #22, #26
v1.4: Added remaining command functionality (s, k, b, h, z, n, r, rz, rn)
      PASS : v1.1 + #7, #8, #9, #10, #11, #12, #13, #14, #15, #16, #17, #18, #19
v1.5: Project is now a multi-file solution
      Precision in -h now depends on the magnitude of the number
v1.6: Comments added
      Code extracted from main and put into functions (printHelp, sortEntries, getMaxWidth, printEntries) --> I HATE LONG MAINS
*/

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "duFunctions.hpp"

using namespace std;

namespace fs = filesystem;

int main(int argc, char* argv[]) {
    try {
        //reads all the command line args and flips the correct booleans, sets blockSize and gets a vector of all the folders
        Options options = parseArgs(argc, argv);

        //outputs the version number
        if (options.version) {
            cout << "1.0.0\n";
            return 0;
        }

        //outputs the help menu --> shows each command and what it does
        if (options.help) {
            printHelp();
            return 0;
        }

        //loops through each of the folders given by the user (or given by default)
        for (auto& folder : options.folders) {

            //collects the data in the directory "tree"
            vector<FolderEntry> entries = scanFolder(fs::path(folder), options);

            //sorts the gathered data based on the flipped booleans
            sortEntries(entries, options);

            //determines how wide the column should be (find the longest number and set it to that)
            int numWidth = getMaxWidth(entries, options);

            //formats the entries given and outputs them
            printEntries(entries, options, numWidth);
        }

    }
    //a safety net just in case an error is thrown
    //catches all exceptions and reports what they were (ex.what)
    catch (const  exception& ex) {
        cout << "Error: " << ex.what() << "\n";
        return 1;
    }

    //if it actually worked
    return 0;
}