#include <fstream>
#include <iostream>
#include <format>

#include <Windows.h>

#include "FileOperations.h"

char FileOpHelpers::filenameHasIllegalChar(const std::string& name)
{
	static const std::vector<char> illegalChars{'\\', '/', ':', '*', '?', '\"' , '<' , '>' , '|' };

	for (char c : name) {
		for (char illegalChar : illegalChars) {
			if (c == illegalChar) {
				return c;
			}
		}
	}

	// return 0 (which is '\0', which is falsy)
	return 0;
}

std::filesystem::path FileOpHelpers::renamePath(const std::filesystem::path& path, const std::string& newName)
{
	// making this a function since you can't replace the stem (which is how i think about it), so you always have to
	// remember to include the extension in the new file name

	std::filesystem::path newPath{ path };
	newPath.replace_filename(newName + path.extension().string());

	return newPath;
}

bool FileOpHelpers::createFolder(const std::filesystem::path& path, bool fileHasNoExtension)
{
	// returns whether a folder was created

	// create_directories() does nothing if they already exist, so we can freely call it without checking beforehand

	// recognize if something is a file (has an extension or is marked as a file without an extension)
	if (path.has_extension() || fileHasNoExtension) {

		// we have to check if it even has a parent path, since create_directories() cannot take in a blank path
		if (path.has_parent_path()) {
			// only create folders for the parent path (else it'll create a folder using the name of the file as well)
			return std::filesystem::create_directories(path.parent_path());
		}
		else {
			// else it's a file with no parent path, so there are no directories to create
			return false;
		}
	}

	// else we treat it like a bunch of folders and create directories using the entire path
	return std::filesystem::create_directories(path);
}

std::string FileOpHelpers::unSuffix(const std::string& path)
{
	// c++ regex is seriously gross, so i'm just going to do this manually.

	// if we have something like "../text files/file (2).txt", we want to return "../text files/file.txt", but there are
	// a few things i want to take into account. i don't want to just find '(' and ')', delete them and what's in
	// between, and call it a day. i want to make sure that it's the last thing in the filename, and that there are only
	// digits between them, else we scrap it and return an unedited path

	// find open and close parens
	size_t endIdx{ path.find_last_of(')') };
	size_t beginIdx{ path.find_last_of('(', endIdx) };

	if (beginIdx == std::string::npos || endIdx == std::string::npos) {
		return path;
	}

	// make sure that there are only digits between the parens
	for (size_t i{ beginIdx + 1 }; i < endIdx; ++i) {
		char c{ path[i] };
		if (!std::isdigit(c)) {
			// we found something that's not a digit between the parens. return unedited path
			return path;
		}
	}

	// make sure "(n+)" is the last thing in the name. whitespace is allowed, any other char is not.
	for (size_t i{ endIdx + 1 }; i < path.length(); ++i) {
		char c{ path[i] };
		
		if (c == '.') {
			// assume we've hit file extension part of name and quit
			break;
		}

		if (!std::isspace(c)) {
			// we found something that's not whitespace after the closing parenthesis. return unedited path
			return path;
		}
	}

	// gobble up whitespace before '('
	beginIdx = path.find_last_not_of(' ', beginIdx - 1);
	// gobble up whitespace after ')'
	endIdx = path.find_first_not_of(' ', endIdx + 1);
	
	// if there was only whitespace at the end of a filename (file had no extension), endIdx will now be npos, so
	// correct it
	if (endIdx == std::string::npos) endIdx = path.length();

	// both indices will point to the first non-space char, so go back one
	beginIdx += 1;
	endIdx -= 1;

	std::string pathCopy{ path };
	pathCopy.erase(
		pathCopy.begin() + beginIdx,
		pathCopy.begin() + endIdx + 1 // erase is [begin, end), so add one to end to include erasing endIdx
	);

	// don't return a blank filename
	if (pathCopy.empty()) {
		return path;
	}

	return pathCopy;
}

std::filesystem::path FileOpHelpers::getFirstUnusedFileName(const std::filesystem::path& path)
{
	// appends " (n)" to the end of a filename to make it unique

	// first we should delete the suffix if it already has one. because if our path is "text (2).txt", we'll probably
	// want to recognize it as "text.txt", so that this function properly generates "text (3).txt". if you don't do
	// this, you'll get "text (2) (2).txt" instead
	std::filesystem::path basicPath{ FileOpHelpers::unSuffix(path.string()) };
	
	// test the basic path. if it's available, return it
	if (!std::filesystem::exists(basicPath)) {
		return basicPath;
	}

	// else we have to start appending numbers and testing if they exist
	for (unsigned int i{ 2 }; i < 1'000'000; i++) {
		std::filesystem::path testPath{ basicPath };
		std::string testName{ std::format("{} ({}){}", basicPath.stem().string(), i, basicPath.extension().string()) };
		testPath.replace_filename(testName);

		if (!std::filesystem::exists(testPath)) {
			return testPath;
		}
	}
	// OPT this can be made faster with a binary style search, but i never deal with that many files, so i'm not going
	// to worry about it.

	// TODO what do we do if all the file names are used?!
	std::cerr << "ERROR: You have 1M files with the same name... why don't you do something about that?!\n";
	return "pick a different file name";
}

bool FileOperations::writeStringToFile(const std::string& string, const std::filesystem::path& path, std::ios_base::openmode mode)
{
	FileOpHelpers::createFolder(path);

	std::ofstream f{ path, mode };
	if (!f) {
		return false;
	}

	f << string << '\n';

	f.flush();
	f.close();
	return true;
}

bool FileOperations::writeStringsToFile(const std::vector<std::string>& strings, const std::filesystem::path& path, std::ios_base::openmode mode)
{
	FileOpHelpers::createFolder(path);

	std::ofstream f{ path, mode };
	if (!f) {
		return false;
	}

	for (const std::string& s : strings) {
		f << s << '\n';
	}

	f.flush();
	f.close();
	return true;
}

std::vector<std::filesystem::directory_entry> FileOperations::getAllFilesInFolder(const std::filesystem::path& path, const std::string& fileType)
{
	std::vector<std::filesystem::directory_entry> entries;

	for (auto& entry : std::filesystem::directory_iterator(path)) {

		// ignore folders
		if (entry.is_directory()) {
			continue;
		}
		// NOTE this plays nice with files without extensions. it properly recognizes that something is a file without
		// an extension, and won't ignore it

		// if we have a file type and it doesn't match this entry's extension. (compare() returns 0 if they match)
		if (!fileType.empty() && fileType.compare(entry.path().extension().string()) != 0) {
			continue;
		}
		// TODO do case-insensitive lowercase comparison?

		entries.push_back(entry);
	}

	return entries;
}

void FileOperations::copyFile(const std::filesystem::path& path)
{
	std::filesystem::path newPath{ FileOpHelpers::getFirstUnusedFileName(path.string()) };

	std::filesystem::copy(path, newPath);
}

bool FileOperations::renameFile(const std::filesystem::path& currentPath, const std::string& newName)
{
	// check if the new name has illegal chars
	char c{ FileOpHelpers::filenameHasIllegalChar(newName) };
	if (c) {
		std::cerr << "ERROR: FileOperations::renameFile() aborting! File contains illegal character: '" << c << "'\n";
		return false;
	}
	
	// construct new path
	std::filesystem::path newPath{ FileOpHelpers::renamePath(currentPath, newName) };

	// check if a file with that name already exists
	if (std::filesystem::exists(newPath)) {
		std::cerr << "ERROR: FileOperations::renameFile() aborting! File named " << newPath.filename() << " already exists!\n";
		return false;
	}
	// TODO i could easily generate a suffixed name here, but i'm not sure that's what i wantwant

	std::filesystem::rename(currentPath, newPath);
	return true;
}

bool FileOperations::deleteFile(const std::filesystem::path& path)
{
	return std::filesystem::remove(path);
}

bool FileOperations::sendToRecycleBin(const std::filesystem::path& path)
{
	// i guess std::filesystem doesn't support this? so i have to use winapi

	// https://stackoverflow.com/questions/70257751/move-a-file-or-folder-to-the-recyclebin-trash-c17
	
	if (!std::filesystem::exists(path)) {
		std::cerr << "ERROR: FileOperations::sendToRecycleBin() tried on a file that doesn't exist. Aborting.\n";
		return false;
	}

	// std::filesystem doesn't require canonical paths. it works find with relative paths. 
	//
	// BUT windows is different, so always make sure it's a canonical path. that way we can support relative paths as
	// well as using '/' (when windows actually requires '\\'). canonical() will convert for us
	std::string pathStr{ std::filesystem::canonical(path).string() };

	// it requires strings to be double null terminated
	pathStr += '\0';

	// https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-shfileopstructa
	SHFILEOPSTRUCTA fileOp;
	fileOp.hwnd = NULL;
	fileOp.wFunc = FO_DELETE;
	fileOp.pFrom = pathStr.data();
	fileOp.pTo = NULL;
	fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION;
	
	// https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shfileoperationa
	int result{ SHFileOperationA(&fileOp) };
	
	if (result != 0) {
		std::cerr << "ERROR: FileOperations::sendToTrash() returned error code: " << result << '\n';
		return false;
	}
	// TODO figure out how to get descriptive error messages

	return true;
}

bool FileOperations::openInExplorer(const std::filesystem::path& path)
{
	// https://stackoverflow.com/questions/354902/open-in-explorer
	
	if (!std::filesystem::exists(path)) {
		std::cerr << "ERROR: FileOperations::openInExplorer() tried opening a file that doesn't exist. Aborting.\n";
		return false;
	}

	// same thing here. always use canonical path, so that passing in relative paths and paths with '/' is supported.
	std::string pathStr{ std::filesystem::canonical(path).string() };
	
	// https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutea
	HINSTANCE _result{ ShellExecuteA(NULL, "open", pathStr.data(), NULL, NULL, SW_SHOWDEFAULT) };
	// uh... int ptr is not a ptr, but it's a long long. neat
	INT_PTR result{ reinterpret_cast<INT_PTR>(_result) };
	if (result <= 32) { // this function returns a value greater than 32 if it succeeds
		std::cerr << "ERROR: FileOperations::openInExplorer() returned error code: " << result << '\n';
		return false;
	}
	// TODO figure out how to get descriptive error messages
	
	//ERROR_FILE_NOT_FOUND  // this will take you to the header where they're all defined

	return true;
}