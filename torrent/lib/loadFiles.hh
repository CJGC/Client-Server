#include <unistd.h>
#include <dirent.h>
#include <string>
#include "sha1.hh"

using namespace std;
typedef string str;

DIR * openDir(str inputDir){
  /* it will try to open a dir */

  DIR *dir; str directory = "";
  if(inputDir == "/files"){
    char currtDir[FILENAME_MAX];
    getcwd(currtDir,FILENAME_MAX);
    directory = currtDir;
    directory += inputDir;
  }
  else directory = inputDir;
  dir = opendir(directory.c_str());
  if(dir == NULL){
    cerr << "Unable to open given directory\n";
    exit(-1);
  }
  return dir;
}

str getFiles(str ownerName,str inputDir="/files") {
  /* it will get files name from a given directory */
  str ownFiles = "";
  DIR *dir = openDir(inputDir);
  struct dirent *file;
  while( (file = readdir(dir)) != NULL ){
    str filename = file->d_name;
    str key = sha1(ownerName+":"+filename);
    ownFiles += key + " " + ownerName + " " + filename + " ";
  }
  closedir(dir);
  return ownFiles;
}
