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
#include <fstream>
#include <unordered_map>

using namespace std;

int getFiles(vector<string> &v,const char *format,const char *directory="") {
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

vector<char> getPackage(string& musPath,string& songName,size_t limit,unsigned int part,size_t &bits){
  string dir = musPath+songName;
  ifstream ifs(dir,ios::binary | ios::ate);
  ifstream::pos_type songSize = ifs.tellg();
  size_t size = (size_t)songSize;
  size_t leftoverPart = size - (part-1)*limit;
  if(leftoverPart >= limit) bits = limit;
  else bits = leftoverPart;
  ifs.seekg((part-1)*limit,ios::beg);
  vector<char> package(bits);
  ifs.read(package.data(),bits);
  ifs.close();
  return package;
}

size_t meetParts(string& musPath,string& songName,size_t limit){
  string dir = musPath+songName;
  ifstream ifs(dir, ios::binary | ios::ate);
  ifstream::pos_type songSize = ifs.tellg();
  ifs.close();
  size_t size = (size_t)songSize;
  size_t parts = ceil((double)size/limit);
  return parts;
}
