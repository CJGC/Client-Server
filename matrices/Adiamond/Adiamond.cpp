#include <limits>
#include "graphReader2.hh"

void show(vec* graph){
	/* it will show the graph */
	for(node *n : graph[0]){
	  node *nod = n;
	  int before = 0, after = 0;
	  while(nod != NULL){
	    cout << nod->value;
	    before = nod->j;
	    nod = nod->right;
	    if(nod == NULL) break;
	    after = nod->j;
	    int spaces = after - before;
	    for(int s=0; s<spaces; s++) cout <<" ";
	  }
	  cout<<endl;
	}
}

void merge(vec* graph,vec* result){
	/* it will merge graph with result in finished iteration */
	for(node *n : result[0]){
		node *nod = n, *next;
		while(nod != NULL){
			next = nod->right;
			linkNode(graph,nod);
			nod = next;
		}
	}
}

uint _min(node* pr,node* pc){
	/* it will find the min value between pr row and pc col */
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

void buildResult(vec* graph, vec* result,uint nodes){
	/* it will simulate an iteration of A diamond algorithm */
	for(int i=0; i<nodes; i++)
		for(int j=0; j<nodes; j++){
			if(i == j) continue;
			node *pr = graph[0][i]; // pointer to row
			node *pc = graph[1][j]; // pointer to col
			uint min = _min(pr,pc);
			if(min != numeric_limits<uint>::max()){
				node *newNod = buildNode(i,j,min);
				linkNode(result,newNod);
			}
		}
}

void Adiamond(vec* graph){
	/* it will simulate A diamond algorithm using a vector of linked list */
	uint nodes = graph[0].size();
	int steeps = nodes-1;
	do{
		vec result[2];
		result[0].resize(nodes);
		result[1].resize(nodes);
		buildResult(graph,result,nodes);
		merge(graph,result);
		steeps--;
	}while(steeps > 0);
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
