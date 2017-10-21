#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
typedef string str;

void loadgraph(str fileName){
  ifstream infile(fileName);
  str line;

  while(getline(infile,line)){
    istringstream iss(line);
    if(line[0] == 'p'){
      str op, word;
      uint nodes,arch;
      iss >> op >> word >> nodes >> arch;
      cout << "Graph with " << nodes << " nodes" << endl;
    }
    else if(line[0] == 'e'){
      char op;
      uint i,j, weight = 1;
      iss >> op >> i >> j; //>> weight;
    }
  }

  infile.close();
}
