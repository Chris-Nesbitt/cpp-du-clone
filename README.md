# Disk Usage Utility (C++)

## Description
A C++ console application that replicates core functionality of the UNIX
`du` (disk usage) command. The program scans directories and reports disk
usage for files and folders while supporting a variety of command-line
options for formatting, sorting, and summarizing results.

The application recursively traverses directory structures, calculates disk
usage based on configurable cluster sizes, and outputs formatted results in
multiple representations.

## Features

### Disk Usage Analysis
- Recursively scans directories to calculate storage usage
- Supports scanning the current working directory or user-specified folders
- Displays usage for individual folders or summary totals

### Command-Line Options
The program supports several CLI flags similar to the UNIX `du` command:

- `-s` display only summary totals for each folder
- `-k` set cluster size to 1024 bytes
- `-b` display output in bytes rather than clusters
- `-h` human-readable output (KiB, MiB, GiB, TiB)
- `-z` sort results by size
- `-n` sort results by folder name
- `-r` reverse sort order
- `--block-size=dddd` specify a custom cluster size
- `--help` display program usage information
- `--version` display version number

## Purpose
This project was developed as a programming exercise to practice file system interaction, command-line interface design, and formatted output in C++.
