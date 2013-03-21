#ifndef _H_FILELIST
#define _H_FILELIST

#include <vector>
#include <string>

class FileList
{
private:
	std::string directory;
	std::vector<std::string> songList;
public:
	void addFile(std::string filename);
	std::vector<std::string> getFiles();
	std::string getFilesString();
	std::string getDirectory();
};

#endif