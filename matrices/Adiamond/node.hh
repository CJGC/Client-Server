#include <vector>

struct node{
  uint i, j, value;
  node *down, *right;
};

using namespace std;
using vec = vector<node *>;

node* buildNode(uint i,uint j,uint value){
  node *nod = new node;
  nod->i = i;
  nod->j = j;
  nod->value = value;
  nod->right = NULL;
  nod->down = NULL;
  return nod;
}

void linkToRight(vec* nodes,node* newNod){
  /* linking horizontally (building a row) */
  uint i = newNod->i, j = newNod->j;
  node *nod = nodes[0][i]; // row
  if(nod == NULL){nodes[0][i] = newNod; return;}
  while(1){
    if(nod->right == NULL){
      nod->right = newNod;
      return;
    }
    if(nod->right->j > j){
      node *aux = nod->right;
      nod->right = newNod;
      newNod->right = aux;
      return;
    }
    nod = nod->right;
  }
}

void linkToDown(vec* nodes,node* newNod){
  /* linking vertically (building a col) */
  uint i = newNod->i, j = newNod->j;
  node *nod = nodes[1][j]; // col
  if(nod == NULL){nodes[1][j] = newNod; return;}
  while(1){
    if(nod->down == NULL){
      nod->down = newNod;
      return;
    }
    if(nod->down->i > i){
      node *aux = nod->down;
      nod->down = newNod;
      newNod->down = aux;
      return;
    }
    nod = nod->down;
  }
}

void linkNode(vec* nodes,node* newNod){
  /* it will link a node to the right and down respectively */
  linkToRight(nodes,newNod);
  linkToDown(nodes,newNod);
}

void destroyNodes(vec* nodes){
  /* it will destroy all linked nodes */
  for(node* nod : nodes[0]){
    node *current = nod;
    while(current != NULL){
      node *next = current->right;
      delete current;
      current = next;
    }
  }
}
