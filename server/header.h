#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <vector>

void getFiles(std::vector<const char *> &v,const char *format) {
  DIR *dir; // pointer that will contain current directory addr
  struct dirent *file; // pointer that will contain each file from current directory
  char currentDir[FILENAME_MAX]; // currentDir will store current directory path
  getcwd(currentDir,FILENAME_MAX); // getcwd will capture current directory path
  dir = opendir(currentDir); // opendir will open the directory captured by getcwd
  while((file = readdir(dir)) != NULL) // each file into directory will be read
    if(strstr(file->d_name,format) != NULL) v.push_back(file->d_name); // store filename with .format
  closedir(dir); // closedir will close current directory
}
