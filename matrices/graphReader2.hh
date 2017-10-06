#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace std;
using vec = vector<unsigned int>;
using _map = map<string,unsigned int>;
typedef string str;
typedef unsigned int _uint;

void readGraph(str fileName,vec* indM,_map& M){
   ifstream infile(fileName);
   str line;
   size_t k=0;

   while(getline(infile,line)){
      istringstream iss(line);
      if(line[0] == 'p'){
         str op, word;
         _uint nodes,arch;
         iss >> op >> word >> nodes >> arch; 
         indM[0].resize(arch/2);
         indM[1].resize(arch/2);
         cout << "Graph with " << nodes << " nodes" << endl;
      }
      else if(line[0] == 'e'){
         char op;
         _uint i,j, weight;
         cout << k<< endl;
         iss >> op >> i >> j;
         indM[0][k] = i;
         indM[1][k] = j;
         str key(to_string(i)+to_string(j));
         M[key] = 1;
         k++;
      }
   }
   infile.close();
}
