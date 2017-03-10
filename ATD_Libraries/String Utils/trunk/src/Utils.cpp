#include <windows.h>
#include <winbase.h>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <stack>

std::string find_filepath(const std::string& StartDir, const std::string& fileName)
{
	std::string Path, FileName;
	WIN32_FIND_DATA FindData;

	std::stack <std::string, std::vector <std::string> > DirStack;
	DirStack.push(StartDir);

	while (!DirStack.empty())
	{
		std::string Directory = DirStack.top();
		DirStack.pop();
		Path = Directory + "\\*.*";

		HANDLE ffh = FindFirstFile(Path.c_str(), &FindData);

		if (ffh == INVALID_HANDLE_VALUE)
			continue;
		
		do
		{
			FileName = FindData.cFileName;

			if(FileName == "." || FileName == "..")
				continue;

			// Recurse Into SubDirectories
			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				Path = Directory + FileName + "\\";

				DirStack.push(Path);
			}
			
			// Must not be a directory and client must be search for an extension
			if(fileName != "" && !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			
				if(FileName != fileName)
					continue;

				return Directory + FileName;
			}
		
		} while (FindNextFile(ffh, &FindData));
		
	}

	return "";
}
