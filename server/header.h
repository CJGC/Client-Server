#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cassert>
#include <cmath>
#include <unordered_map>

int getFiles(std::vector<std::string> &v,const char *format,const char *directory="") {
  DIR *dir; // pointer that will contain current directory addr
  struct dirent *file; // pointer that will contain each file from current directory
  char currentDir[FILENAME_MAX]; // currentDir will store current directory path
  if(directory == "") getcwd(currentDir,FILENAME_MAX); // getcwd will capture current directory path
  else strcpy(currentDir,directory);
  dir = opendir(currentDir); // opendir will open the directory captured by getcwd
  if(dir == NULL) return -1;
  while((file = readdir(dir)) != NULL) // each file into directory will be read
    if(strstr(file->d_name,format) != NULL) v.push_back(file->d_name); // store filename with .format
  closedir(dir); // closedir will close current directory
  return 0;
}

std::vector<char> readFileToBytes(const std::string& fileName){
	std::ifstream ifs(fileName, std::ios::binary | std::ios::ate);
	std::ifstream::pos_type pos = ifs.tellg();
	std::vector<char> result(pos);
	ifs.seekg(0, std::ios::beg);
	ifs.read(result.data(), pos);
	return result;
}

std::vector<char> getPackage(std::vector<char> &v,size_t &i,int limit,size_t &counter){
  std::vector<char> package(limit);
  size_t sizeV = v.size();
  for(i,counter = 0;i < sizeV && counter != limit;i++,counter++) package[counter] = v[i];
  return package;
}
