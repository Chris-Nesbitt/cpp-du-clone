#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>

struct Options {
	bool s = false;
	bool k = false;
	bool h = false;
	bool z = false;
	bool n = false;
	bool r = false;
	bool b = false;
	bool help = false;
	bool version = false;
	std::uintmax_t blockSize = 4096;
	std::vector<std::string> folders;
};

struct FolderEntry {
	std::filesystem::path path;
	std::uintmax_t clusterBytes = 0;
};

Options parseArgs(int argc, char* argv[]);

std::uintmax_t roundUpToCluster(std::uintmax_t bytes, std::uintmax_t blockSize);
std::uintmax_t sumDir(const std::filesystem::path& dir, std::uintmax_t blockSize);
std::uintmax_t toDisplayUnits(std::uintmax_t bytes, const Options& options);
std::string formatHuman(std::uintmax_t bytes);
std::string formatSize(std::uintmax_t clusterBytes, const Options& options, int numWidth);
std::vector<FolderEntry> scanFolder(const std::filesystem::path& root, const Options& options);
void printHelp();
void sortEntries(std::vector<FolderEntry>& entries, const Options& options);
int getMaxWidth(const std::vector<FolderEntry>& entries, const Options& options);
void printEntries(const std::vector<FolderEntry>& entries, const Options& options, int numWidth);

