/*
* Developed by Michael Furst February 19th, 2017
* The purpose of this program is to sort lab submissions downloaded from canvas. This file is programmed to
* support Windows, OSX, and Linux environments. For windows compilation you will need to obtain a local 
* version of dirent.h. I recommend Toni Ronko's version: https://github.com/tronkko/dirent
* This code has been tested on each platform supported, but if you find any bugs you may submit the issues
* to this project on GitHub. 
* https://github.com/mafurst/sortLabs
*/

#ifdef _WIN32
#include "dirent.h"
#elif __APPLE__
#include <copyfile.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#elif __linux__
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <unistd.h>
#endif

#include <fstream>
#include <iostream>
#include <string>
using namespace std;

bool createDirectory(string dirName);
bool copyFile(string fileSrc, string fileDest);

#ifdef _WIN32
/*
* Windows does not need a reformat, as it will recognize a 
* file path with \, / or both.
*/
bool createDirectory(string dirName) {
	//Use bool CreateDirectory function built into windows
	return CreateDirectoryA(dirName.c_str(),NULL);
}
bool copyFile(string fileSrc, string fileDest) {
	//Use bool CopyFile function built into windows
	return CopyFileA(fileSrc.c_str(),fileDest.c_str(),false);
}
#elif __APPLE__
/*
* reformatUnix and createDirectory exists the same in both Apple and Linux sections.
*  I am not putting them in a container that reaches all UNIX systems becuase this code 
* does not have full UNIX support, and I do not want anyone to be confused into thinking
* that.
*/
void reformatUnix(string *filePath) {
	//reformat so / becomes \ to prevent directory issues, only neccessary on UNIX systems
	for (int i=0;i<filePath->length();i++) {
		if ((*filePath)[i]=='\\')(*filePath)[i]='/';
	}	
}
bool createDirectory(string dirName) {
	reformatUnix(&dirName);
	//use mkdir function
	if (mkdir(dirName.c_str(),0777)) {
		return false;
	}
	//use chmod function to ensure proper permissions
	if (chmod(dirName.c_str(),07777)) {
		return false;
	}
	return true;
}
bool copyFile(string fileSrc, string fileDest) {
	reformatUnix(&fileSrc);
	reformatUnix(&fileDest);
	//try to use the macOS copyfile function, which returns 0 on success
	if (copyfile(fileSrc.c_str(),fileDest.c_str(),copyfile_state_alloc(),COPYFILE_ALL)==0) {
		return true;
	}
	else {
		//macOS limits the amount of calls to copyfile, so in the case it fails, we attempt to run a copy with bash
		string bashCommand = "cp -f \"";
		bashCommand+=fileSrc;
		bashCommand+="\" \"";
		bashCommand+=fileDest;
		bashCommand+="\"";
		int check = system(bashCommand.c_str()); //get the return of the cp command
		return (check==0);//cp exits 0 on success, so return true if the check is 0
	}
}
#elif __linux__
void reformatUnix(string *filePath) {
	//reformat so / becomes \ to prevent directory issues, only neccessary on UNIX systems
	for (int i=0;i<filePath->length();i++) {
		if ((*filePath)[i]=='\\')(*filePath)[i]='/';
	}
}
bool createDirectory(string dirName) {
	reformatUnix(&dirName);
	//use mkdir function
	if (mkdir(dirName.c_str(),0777)) {
		return false;
	}
	//use chmod function to ensure proper permissions
	if (chmod(dirName.c_str(),07777)) {
		return false;
	}
	return true;
}
bool copyFile(string fileSrc, string fileDest) {
	reformatUnix(&fileSrc);
	reformatUnix(&fileDest);
	//create read and write file variables
	int read;
	int write;
	//create stat structure and off_t structure
	struct stat stat_buf;
	off_t offset = 0;
	//open read file
	read = open (fileSrc.c_str(), O_RDONLY);
	//get the file stats for the read file into the stat structure
	fstat (read, &stat_buf);
	//open/create the write file with same attributes as the read file
	write = open (fileDest.c_str(), O_WRONLY | O_CREAT, stat_buf.st_mode);
	//use sendfile command to send the read file to the write file
	sendfile (write, read, &offset, stat_buf.st_size);
	//close read and write files
	close (read);
	close (write);
}
#endif

int findIndex(string str, char c) {
	for (int i=0;i<str.length();i++) {
		if (str[i]==c)return i;
	}
	return -1;
}
string lowerCase(string str) {
	string ret = str;
	for (int i=0;i<ret.length();i++) {
		ret[i]=tolower(ret[i]);
	}
	return ret;
}
int main(int argc, char** argv) {
	if (argc!=2)return -1;//requires directory as argument or does nothing
	DIR *dir;
	struct dirent *ent;
	if ((dir=opendir(argv[1])) != NULL) {//check if able to open the directory passed as an argument
		while ((ent=readdir(dir)) != NULL) {//run to read all files in the directory
			if (opendir(ent->d_name)==NULL) {//make sure that the file is a file, not a directory
				string fileName = ent->d_name;
				cout << "--------------------------\n";
				cout << fileName << endl;//print original file name
				int pos = findIndex(fileName,'_');//get index of first _, everything before it is the users name formated lastfirst
				if (pos!=-1) {
					cout << fileName.substr(0,pos) << endl;//output the name
					string filePath=argv[1];
					filePath+="/";
					filePath+=fileName.substr(0,pos);
					if (opendir(filePath.c_str())==NULL) {//check if dir already exists
						string dirName = argv[1];
						dirName+="/"+fileName.substr(0,pos);
						createDirectory(dirName);
					}
					//Get the new file name as just the file name
					string newFileName = fileName;
					while ( (pos=findIndex(newFileName,'_')) != -1) {
						newFileName = newFileName.substr(pos+1);
					}
					//remove any non-name parts, ie - 1 or - 2, etc.
					if ( (pos=findIndex(newFileName,'-')) != -1) {//check if - exists
						int dotPos = findIndex(newFileName,'.');
						if (dotPos>pos) {//if the . is after the -
							newFileName = newFileName.substr(0,pos) + newFileName.substr(dotPos);
						}
					}
					cout << newFileName << endl;

					//copy the file
					string file1=argv[1];
					file1+="/"+fileName;
					string file2 = filePath + "/" + newFileName;
					if (copyFile(file1,file2)) {
						cout << "File Copied Successfully!" << endl;
					}
					else {
						cout << "File Not Copied Successfully!" << endl;
						cout << "--" << file1 << " --" << file2 << endl;
					}
				}
			}
		}
	}
	cin.get();
	return 0;
}
