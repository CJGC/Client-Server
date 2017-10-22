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

bool linkToRight(vec* nodes,node* newNod){
  /* linking horizontally (building a row) if success link newNod */
  uint i = newNod->i, j = newNod->j;

  // linking first element of row vector
  node *nod = nodes[0][i]; // row
  if(nod == NULL){nodes[0][i] = newNod; return true;}
  if(nodes[0][i]->j == j){
    nodes[0][i]->value = newNod->value;
    return false;
  }
  if(nodes[0][i]->j > j){
    node *aux = nodes[0][i];
    nodes[0][i] = newNod;
    newNod->right = aux;
    return true;
  }

  // linking different elements to the first one of the row vector
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

bool linkToDown(vec* nodes,node* newNod){
  /* linking vertically (building a col) if success link newNod */
  uint i = newNod->i, j = newNod->j;

  // linking first element of col vector
  node *nod = nodes[1][j]; // col
  if(nod == NULL){nodes[1][j] = newNod; return true;}
  if(nodes[1][j]->i == i){
    nodes[1][j]->value = newNod->value;
    return false;
  }
  if(nodes[1][j]->i > i){
    node *aux = nodes[1][j];
    nodes[1][j] = newNod;
    newNod->down = aux;
    return true;
  }

  // linking different elements to the first one of the col vector
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

void linkNode(vec* nodes,node* newNod){
  /* it will link a node to the right and down respectively */
  if(!linkToRight(nodes,newNod)){delete newNod; return;}
  if(!linkToDown(nodes,newNod)){delete newNod; return;}
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