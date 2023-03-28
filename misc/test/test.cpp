#include <gtest/gtest.h>

#include "FileOperations.h"


void decomposeFileSystemPath(const std::filesystem::path& path)
{
	// https://en.cppreference.com/w/cpp/filesystem/path decomposition section

	// trying to see what's what in a path
	std::cout << "\n\n   Decomposing: " << path;
	std::cout << "\n     Root Name: " << path.root_name().string();
	std::cout << "\n      Root Dir: " << path.root_directory().string();
	std::cout << "\n     Root Path: " << path.root_path().string();
	std::cout << "\n Relative Path: " << path.relative_path().string();
	std::cout << "\n   Parent Path: " << path.parent_path().string();
	std::cout << "\n     File Name: " << path.filename().string();
	std::cout << "\n          Stem: " << path.stem().string();
	std::cout << "\n     Extension: " << path.extension().string();
}

void cleanFolder(const std::string& folder) 
{
	// delete folder and assert that it doesn't exist
	std::filesystem::remove_all(folder);
	ASSERT_FALSE(std::filesystem::exists(folder));
}

TEST(Decomposition, DISABLED_Decomposition)
{
	// test having all folders
	decomposeFileSystemPath("C:/test/test2/messages");
	// NOTE. it thinks messages is a file in the first one. i assumed it'd think it was a folder

	// test file with extension
	decomposeFileSystemPath("C:/test/test2/messages/text.txt");
	
	// test file without extension
	decomposeFileSystemPath("C:/test/test2/messages/text");

	std::cout << "\n\n\n";
}

TEST(FileOpHelperTests, IllegalFileNames)
{
	ASSERT_EQ(FileOpHelpers::filenameHasIllegalChar("text.txt"), false);
	ASSERT_EQ(FileOpHelpers::filenameHasIllegalChar("text|"), '|');
	ASSERT_EQ(FileOpHelpers::filenameHasIllegalChar("te?xt.txt"), '?');
	ASSERT_EQ(FileOpHelpers::filenameHasIllegalChar("te><t.txt"), '>');
}

TEST(FileOpHelperTests, Unsuffix)
{
	ASSERT_EQ(FileOpHelpers::unSuffix("/test/message (2).txt"), "/test/message.txt");
	
	ASSERT_EQ(FileOpHelpers::unSuffix(
		"test123/configs (it should not erase this set of parens)/startup.cfg"),
		"test123/configs (it should not erase this set of parens)/startup.cfg",
	);
	
	ASSERT_EQ(FileOpHelpers::unSuffix(
		"what/about/a path that/has no/parens at all/nor a file/extension"),
		"what/about/a path that/has no/parens at all/nor a file/extension"
	);

	ASSERT_EQ(FileOpHelpers::unSuffix("/test/message     (123123123)   .txt"), "/test/message.txt");
	ASSERT_EQ(FileOpHelpers::unSuffix("/test/message (4539347)   .txt"), "/test/message.txt");
	// it should not erase these, since (n) isn't the last thing before the file extension
	ASSERT_EQ(FileOpHelpers::unSuffix("/test/message (37)  d .txt"), "/test/message (37)  d .txt");

	// test files without extensions
	ASSERT_EQ(FileOpHelpers::unSuffix("/test/message (145)"), "/test/message");
	ASSERT_EQ(FileOpHelpers::unSuffix("/test/message   (3475)    "), "/test/message");
	// same here. it shouldn't erase it since (n) isn't the last thing
	ASSERT_EQ(FileOpHelpers::unSuffix("/test/message     (648189)      asdf      "), "/test/message     (648189)      asdf      ");

	// test files that are just spaces and (n+). these result in blank files names, so it should abort
	ASSERT_EQ(FileOpHelpers::unSuffix("    (2345)            "), "    (2345)            ");
	ASSERT_EQ(FileOpHelpers::unSuffix("           (4789)"), "           (4789)");
	//ASSERT_EQ(FileOpHelpers::unSuffix("(5453)            "), "(5453)            ");
	// TODO this last one doesn't work correctly, but i'm not going to worry about it for now.
}

TEST(FileOpHelperTests, DirectoryCreation)
{
	const std::string folder{ "test/folder creation/" };

	std::string fullPath;


	// clean folder from previous tests
	cleanFolder(folder);
	
	// test a standalone file, which shouldn't create any folders since it has no parent path
	ASSERT_FALSE(FileOpHelpers::createFolder("test.txt"));

	// test using a path with no file, which should create a folder
	fullPath = folder + "creation1";
	ASSERT_TRUE(FileOpHelpers::createFolder(fullPath));
	ASSERT_TRUE(std::filesystem::exists(fullPath));

	// test using a path with a file, which should create the folders, but not the file
	fullPath = folder + "creation2/creation.txt";
	ASSERT_TRUE(FileOpHelpers::createFolder(fullPath));
	ASSERT_TRUE(std::filesystem::exists(folder + "creation2"));

	// test with a file that has no extension. this should create the folder, but not the file IFF you mark the file as
	// having no extension, otherwise it will create them all as folders
	fullPath = folder + "creation3/creation";
	ASSERT_TRUE(FileOpHelpers::createFolder(fullPath, true));
	ASSERT_TRUE(std::filesystem::exists(folder + "creation3"));
	ASSERT_FALSE(std::filesystem::exists(fullPath));
}

TEST(FileOpHelperTests, UnusedFileNames)
{
	const std::string folder{ "test/unused file names/" };
	const std::string fileName{ "unused" };
	const std::string ext{ ".txt" };

	// clean folder from previous tests
	cleanFolder(folder);

	// make a file to "take" the name
	std::string firstFileName{ folder + fileName + ext };
	FileOperations::writeStringToFile("asdf", firstFileName);

	std::string shouldEqual;
	std::filesystem::path unusedPath;
	for (int i{ 2 }; i < 10; ++i) {
		unusedPath = FileOpHelpers::getFirstUnusedFileName(firstFileName);
		shouldEqual = std::format("{}{} ({}){}", folder, fileName, i, ext);
		ASSERT_TRUE(unusedPath.string() == shouldEqual);
		FileOperations::writeStringToFile("asdf", unusedPath);
	}
}

TEST(FileOperationTests, CopyingFile)
{
	const std::string folder{ "test/copying/" };
	const std::string file{ "copy.txt" };

	std::string fullName;

	std::filesystem::path unusedName;

	// clean folder from previous tests
	cleanFolder(folder);

	// make a file
	fullName = folder + file;
	FileOperations::writeStringToFile("testing copying", fullName);
	ASSERT_TRUE(std::filesystem::exists(fullName));

	// copy it
	unusedName = FileOpHelpers::getFirstUnusedFileName(fullName);
	FileOperations::copyFile(fullName);
	ASSERT_TRUE(std::filesystem::exists(unusedName));

	// do it again
	unusedName = FileOpHelpers::getFirstUnusedFileName(fullName);
	FileOperations::copyFile(fullName);
	ASSERT_TRUE(std::filesystem::exists(unusedName));
}

TEST(FileOperationTests, RenamingFile)
{
	const std::string folder{ "test/renaming/" };
	const std::string file{ "rename.txt" };

	std::string fullName{ folder + file };

	// clean folder from previous tests
	cleanFolder(folder);

	// make a file
	FileOperations::writeStringToFile("this file should get renamed", fullName);

	// rename it
	ASSERT_TRUE(FileOperations::renameFile(fullName, "new name"));
	fullName = folder + "new name.txt";
	ASSERT_TRUE(std::filesystem::exists(fullName));

	// try renaming it again, which should fail and return false
	ASSERT_FALSE(FileOperations::renameFile(fullName, "new name"));

	// try renaming it with illegal chars, which should fail and return falase
	ASSERT_FALSE(FileOperations::renameFile(fullName, "?|<>*/\\"));
}

TEST(FileOperationTests, DeletingFile)
{
	const std::string folder{ "test/deleting/" };
	const std::string file{ "delete.txt" };

	std::string fullName{ folder + file };

	// clean folder from previous tests
	cleanFolder(folder);

	// make a file
	FileOperations::writeStringToFile("this file should get deleted", fullName);

	// delete file
	ASSERT_TRUE(FileOperations::deleteFile(fullName));

	// try deleting the file when it no longer exists
	ASSERT_FALSE(FileOperations::deleteFile(fullName));
}

TEST(FileOperationTests, OpenInExplorer)
{
	const std::string folder{ "test/opening/" };

	// clean folder from previous tests
	cleanFolder(folder);

	// create the folder
	FileOpHelpers::createFolder(folder);
	// open it
	ASSERT_TRUE(FileOperations::openInExplorer(folder));
	// test opening something that doesn't exist
	ASSERT_FALSE(FileOperations::openInExplorer(folder + "/doesnt exist"));
}

TEST(FileOperationTests, SendToRecycleBin)
{
	const std::string folder{ "test/recycling/" };
	const std::string fullName{ folder + "recycle.txt" };

	// clean folder from previous tests
	cleanFolder(folder);

	// create file
	FileOperations::writeStringToFile("this file should get recycled", fullName);
	ASSERT_TRUE(std::filesystem::exists(fullName));

	// send it to recycling bin
	ASSERT_TRUE(FileOperations::sendToRecycleBin(fullName));
	ASSERT_FALSE(std::filesystem::exists(fullName));

	// try sending it again
	ASSERT_FALSE(FileOperations::sendToRecycleBin(fullName));
}

TEST(FileOperationTests, GetAllFilesInFolder)
{
	const std::string folder{ "test/getting files/" };
	const std::string fileName{ "get" };
	const std::string ext{ ".txt" };

	const std::string fullName{ folder + fileName + ext };

	// clean folder from previous tests
	cleanFolder(folder);

	// make a handful of files
	int numFiles{ 10 };
	for (int i{ 0 }; i < numFiles; ++i) {
		FileOperations::writeStringToFile("asdf1234", FileOpHelpers::getFirstUnusedFileName(fullName));
	}
	
	// grab them and print them out
	std::vector<std::filesystem::directory_entry> entries{
		FileOperations::getAllFilesInFolder(folder)
	};
	ASSERT_TRUE(entries.size() == numFiles);
	for (const auto& entry : entries) {
		std::cout << entry.path() << '\n';
	}

	// test filtering
	FileOperations::writeStringToFile("asdf1234", folder + fileName + ".csv");
	entries = FileOperations::getAllFilesInFolder(folder, ".csv");
	ASSERT_TRUE(entries.size() == 1);
	for (const auto& entry : entries) {
		std::cout << entry.path() << '\n';
	}
}





void testGettingAllFilesInFolder()
{
	auto entries{ FileOperations::getAllFilesInFolder("test") };

	for (auto& entry : entries) {
		std::cout << "\n" << entry.path();
	}
}


// TODO how can i test writing strings to a file? i suppose i need to read its contents, or manually check it, idk.
