#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace FileOpHelpers
{
	// returns illegal char if file name has one. 0 if it's good. (ONLY tests the file name, not any directory names)
	char filenameHasIllegalChar(const std::string& name);

	// make a new path with the new name
	std::filesystem::path renamePath(const std::filesystem::path& path, const std::string& newName);

	// creates parent folders if they don't exist. path can be either
	// 1. all folders
	// 2. folders/file.extension
	// 3. folders/file (if you want to use this, make sure to mark fileHasNoExtension = true!)
	bool createFolder(const std::filesystem::path& path, bool fileHasNoExtension = false);

	// removes the " (n+)" that i append to file names.
	std::string unSuffix(const std::string& path);
	// NOTE this is only public so i can easily test it, so i'm putting it in the helper namespace

	// returns the first unused file name, in the format of "folder/filename (n).extension"
	std::filesystem::path getFirstUnusedFileName(const std::filesystem::path& path);
}

namespace FileOperations
{
	// https://en.cppreference.com/w/cpp/filesystem

	// writes single string to a file
	bool writeStringToFile(const std::string& string, const std::filesystem::path& path, std::ios_base::openmode mode = std::ios_base::out);

	// writes vector of strings to file. each string in the vector is automatically put on a new line
	bool writeStringsToFile(const std::vector<std::string>& strings, const std::filesystem::path& path, std::ios_base::openmode mode = std::ios_base::out);
	
	// returns a list of all files found inside of the folder. has optional fileType filter to only retrieve files with that extension
	std::vector<std::filesystem::directory_entry> getAllFilesInFolder(const std::filesystem::path& path, const std::string& fileType = "");

	// newName should *not* include file extension. aborts if something with that name already exists. returns if file
	// was successfully renamed. 
	bool renameFile(const std::filesystem::path& current, const std::string& newName);
	void copyFile(const std::filesystem::path& path);
	bool deleteFile(const std::filesystem::path& path);

	// windows functions
	bool sendToRecycleBin(const std::filesystem::path& path);
	bool openInExplorer(const std::filesystem::path& path);
}
