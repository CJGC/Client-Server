#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;
typedef string str;
using vec = vector<str>;

vec macAddr(){
  /* it will get mac address in linux architecture */
  ifstream infile("//proc/net/arp");
  str line; vec macs;
  getline(infile,line); // skipping first line (info line)

  while(getline(infile,line)){
    istringstream iss(line);
    str ip, hwtype, flags, mac, mask, device;
    iss >> ip >> hwtype >> flags >> mac >> mask >> device;
    macs.push_back(mac);
  }

  infile.close();
  return macs;
}
