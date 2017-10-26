#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "node.hh"

using namespace std;
typedef string str;

void BuildDiagonal(vec* graph){
  /* it will build i == j diagonal into graph */
  uint nodes = graph[0].size();
  for(uint n=0; n<nodes; n++){
    node *newNode = buildNode(n,n,0);
    linkNode(graph,newNode);
  }
}

void loadgraph(str fileName,vec* graph){
  ifstream infile(fileName);
  str line;

  while(getline(infile,line)){
    istringstream iss(line);
    if(line[0] == 'p'){
      str op, word;
      uint nodes,arch;
      iss >> op >> word >> nodes >> arch;
      graph[0].resize(nodes); // row vector
      graph[1].resize(nodes); // col vector
      cout << "Graph with " << nodes << " nodes" << endl;
    }
    else if(line[0] == 'e' || line[0] == 'a'){
      char op;
      uint i,j, weight = 1;
      iss >> op >> i >> j >> weight;
      node *newNode = buildNode(i-1,j-1,weight);
      linkNode(graph,newNode);
    }
  }
  infile.close();
  BuildDiagonal(graph);
}
