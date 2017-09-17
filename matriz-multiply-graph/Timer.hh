#include <chrono>
#include <functional>
#include <iostream>
#include <string>

using namespace std;

class Timer {
private:
  chrono::high_resolution_clock::time_point start;
  string name;

public:
  Timer(const string &n)
      : start(chrono::high_resolution_clock::now()), name(n) {}

  long elapsed() {
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
  }
};
