#include <vector>

struct node{
  uint i, j, value;
  node *down, *right;
};

using namespace std;
using vec = vector<node *>;

node* buildNode(uint i,uint j,uint value){
  /* it will build a new node */
  node *nod = new node;
  nod->i = i;
  nod->j = j;
  nod->value = value;
  nod->right = NULL;
  nod->down = NULL;
  return nod;
}

bool linkToRight(vec* graph,node* newNod){
  /* it will try to link horizontally (building a row) */
  uint i = newNod->i, j = newNod->j;

  // linking first node to the rows vector
  node *nod = graph[0][i]; // row
  if(nod == NULL){graph[0][i] = newNod; return true;}
  if(graph[0][i]->j == j){
    graph[0][i]->value = newNod->value;
    return false;
  }
  if(graph[0][i]->j > j){
    node *aux = graph[0][i];
    graph[0][i] = newNod;
    newNod->right = aux;
    return true;
  }

  // linking the rest nodes to the row vector
  while(1){
    if(nod->right == NULL){
      nod->right = newNod;
      return true;
    }
    if(nod->right->j == j){
      nod->right->value = newNod->value;
      return false;
    }
    if(nod->right->j > j){
      node *aux = nod->right;
      nod->right = newNod;
      newNod->right = aux;
      return true;
    }
    nod = nod->right;
  }

}

bool linkToDown(vec* graph,node* newNod){
  /* it will try to link vertically (building a col)  */
  uint i = newNod->i, j = newNod->j;

  // linking first node to the col vector
  node *nod = graph[1][j]; // col
  if(nod == NULL){graph[1][j] = newNod; return true;}
  if(graph[1][j]->i == i){
    graph[1][j]->value = newNod->value;
    return false;
  }
  if(graph[1][j]->i > i){
    node *aux = graph[1][j];
    graph[1][j] = newNod;
    newNod->down = aux;
    return true;
  }

  // linking the rest nodes to the col vector
  while(1){
    if(nod->down == NULL){
      nod->down = newNod;
      return true;
    }
    if(nod->down->i == i){
      nod->down->value = newNod->value;
      return false;
    }
    if(nod->down->i > i){
      node *aux = nod->down;
      nod->down = newNod;
      newNod->down = aux;
      return true;
    }
    nod = nod->down;
  }

}

void linkNode(vec* graph,node* newNod){
  /* it will link a node with row and col vector espectively */
  if(!linkToRight(graph,newNod)){delete newNod; return;}
  if(!linkToDown(graph,newNod)){delete newNod; return;}
}

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

void merge(vec* destGraph,vec* sourcGraph){
	/* it will merge two graphs */
	for(node *n : sourcGraph[0]){
		node *nod = n, *next;
		while(nod != NULL){
			next = nod->right;
			nod->right = NULL;
			nod->down = NULL;
			linkNode(destGraph,nod);
			nod = next;
		}
	}
}

void makeCopy(vec* destGraph,vec* sourcGraph){
	/* it will make a copy of source graph */
	for(node *nod : sourcGraph[0]){
		node *current = nod;
		while(current != NULL){
			node *newNod = buildNode(current->i,current->j,current->value);
			linkNode(destGraph,newNod);
			current = current->right;
		}
	}
}

void destroyGraph(vec* graph){
  /* it will destroy a graph */
  for(node* nod : graph[0]){
    node *current = nod;
    while(current != NULL){
      node *next = current->right;
      delete current;
      current = next;
    }
  }
}
