#include <cmath>
#include <vector>

using namespace std;

template <typename T> double arithMean(const vector<T> &vi) {
  double cont = 0;
  for (const T &t : vi)
    cont += t;
  return (cont / vi.size());
}

template <typename T> double stdDev(const vector<T> &vi) {
  double mean = arithMean(vi);
  double cont = 0;

  for (int i = 0; i < vi.size(); ++i) {
    cont += pow((vi[i] - mean), 2);
  }

  return sqrt(cont / vi.size());
}
