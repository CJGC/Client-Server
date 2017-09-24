#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

template<typename T>
vector<T> readGraph(string fileName,size_t& nodes){
   vector<T> m;
   ifstream infile(fileName);
   string line;

   while(getline(infile,line)){
      istringstream iss(line);
      if(line[0] == 'p'){
         string s1, s2;
         iss >> s1 >> s2 >> nodes; 
         m.resize(nodes*nodes);
         cout << "Graph with " << nodes << " nodes" << endl;
      }
      else if(line[0] == 'e'){
         char e;
         unsigned int i,j; 
         iss >> e >> i >> j;
         m[(i-1)*nodes + (j-1)] = 1;
      }
   }
   infile.close();
   return m;
}
