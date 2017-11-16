#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

using namespace std;
typedef string str;
using _map_ = map<str,str>;

_map_ getMachInfo(){
  /* it will get machine info (must be linux architecture) */
  _map_ info;
  ifstream infile("//proc/net/arp");
  str line, ip, hwtype, flags, mac, mask, device;
  getline(infile,line); // skipping first line (irrelevant info line)
  getline(infile,line); // second line have relevant info
  istringstream iss(line);
  iss >> ip >> hwtype >> flags >> mac >> mask >> device;
  info["ip"] = ip;
  info["hwtype"] = hwtype;
  info["flags"] = flags;
  info["mac"] = mac;
  info["mask"] = mask;
  info["device"] = device;
  infile.close();
  return info;
}
