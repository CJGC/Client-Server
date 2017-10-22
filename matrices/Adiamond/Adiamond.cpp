#include <limits>
#include "graphReader2.hh"

void show(vec* graph){
	for(node* n : graph[0]){
	  node* aux = n;
	  int befor = 0, after = 0;
	  while(aux != NULL){
	    cout << aux->value;
	    befor = aux->j;
	    aux = aux->right;
	    if(aux == NULL) break;
	    after = aux->j;
	    int spaces = after - befor;
	    for(int s=0; s<spaces; s++) cout <<" ";
	  }
	  cout<<endl;
	}
}

uint _min(node* pr,node* pc){
	uint min = numeric_limits<uint>::max();
	while(pr != NULL && pc != NULL){
		if(pr->j == pc->i){
			uint val = pr->value + pc->value;
			if(val < min) min = val;
			pr = pr->right;
			pc = pc->down;
		}
		else if(pr->j > pc->i) pc = pc->down;
		else pr = pr->right;
	}
	return min;
}

void Adiamond(vec* graph){
	uint nodes = graph[0].size();
	for(int i=0; i<nodes; i++){
		for(int j=0; j<nodes; j++){
			if(i == j) continue;
			node *pr = graph[0][i]; // pointer to row
			node *pc = graph[1][j]; // pointer to col
			uint min = _min(pr,pc);
			if(min != numeric_limits<uint>::max()){
				node* newNod = buildNode(i,j,min);
				linkNode(graph,newNod);
			}
		}
	}
}

int main(int argc, char const **argv){
	if(argc != 2){cerr << "Usage <"<<argv[0]<<"> <graph path>"<<'\n'; return -1;}
	vec graph[2];
	loadgraph(argv[1],graph);
	Adiamond(graph);
	show(graph);
	destroyNodes(graph);
	return 0;
}
