#include <limits>
#include <thread>
#include <cmath>
#include "graphReader2.hh"

void show(vec* graph){
	/* it will show the graph */
	for(node *n : graph[0]){
	  node *nod = n;
		while(nod != NULL){
			cout <<" i -> "<< nod->i <<" | j -> "<< nod->j<<" | value -> "<< nod->value <<endl;
			nod = nod->right;
		}
	}
}

void merge(vec* graph,vec* result){
	/* it will merge graph with result in finished iteration */
	for(node *n : result[0]){
		node *nod = n, *next;
		while(nod != NULL){
			next = nod->right;
			nod->right = NULL;
			nod->down = NULL;
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

void pChunk(vec* graph,vec* result,uint nodes,uint ini,uint end){
	/* it will process a chunk of data by a thread */
	for(uint i=ini; i<=end && i<nodes; i++)
		for(uint j=0; j<nodes; j++){
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

void buildResult(vec* graph, vec* result,uint nodes){
	/* it will simulate an iteration of A diamond algorithm by chunks */
	uint threadsAmount = 7, ti = 0, i = 0, actiThreads = 0;
	thread t[threadsAmount];
	uint chunk = floor((double)nodes/threadsAmount);
	int pendRows = nodes;
	if(!chunk) chunk = 1;

	do{
		for(ti=0, i; ti<threadsAmount && pendRows > 0; ti++,i++,actiThreads++){
			uint end = (i+1)*chunk - 1;
			uint ini = end - chunk + 1;
			t[ti] = thread(pChunk,graph,result,nodes,ini,end);
			pendRows -= chunk;
		}
		for(ti=0; ti<actiThreads; ti++) t[ti].join();
		actiThreads=0;
	}while(pendRows > 0);

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
