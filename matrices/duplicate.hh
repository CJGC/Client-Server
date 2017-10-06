#include <vector>
using vec = vector<unsigned int>;

void duplicate(vec* v1,vec* v2,const size_t& size){
  v2[0].resize(size);
  v2[1].resize(size);

  for(size_t k=0; k<size; k++){
    v2[0][k] = v1[0][k];
    v2[1][k] = v1[1][k];
  }
}
