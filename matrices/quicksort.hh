#include <vector>
using vec = vector<unsigned int>;

void swap(vec* A,int& l,int& r,const size_t& k,const size_t& t){
  unsigned int aux = A[k][l];
  A[k][l] = A[k][r];
  A[k][r] = aux;

  aux = A[t][l];
  A[t][l] = A[t][r];
  A[t][r] = aux;
  l++; r--;
}

void quicksort(vec* A,int left,int right,const size_t& k,const size_t& t){
  int l = 0, r = 0, pivot = 0;
  if(left >= right) return;
  l = left; r = right;
  pivot = A[k][(left + right)/2];
  while(l <= r){
    while(A[k][l] < pivot) l++;
    while(A[k][r] > pivot) r--;
    if(l <= r) swap(A,l,r,k,t);
  }
  quicksort(A,left,r,k,t);
  quicksort(A,l,right,k,t);
}

void fquicksort(vec* A){
  size_t lower = 0, higher = 0;
  for(auto& item : A[1]){
    if(A[1][lower] != item){
      quicksort(A,lower,higher-1,0,1);
      lower = higher;
    }
    higher++;
  }
}
