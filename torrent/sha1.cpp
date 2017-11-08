#include <iostream>
#include <string>
#include <cmath>
#include <bitset>

using namespace std;
typedef string str;

str strToBin(str _str){
  /* it will turn a string of alphabet letter in binary form */
  str binary = "";
  size_t _strSize = _str.size();
  for(size_t c = 0; c < _strSize; c++){
    bitset<8> bin(_str.c_str()[c]);
    binary += bin.to_string();
  }
  return binary;
}

int strOfBinToNum(str _str){
  /* it will turn a string of binary numbers to decimal form */
  int decimal = 0;
  for(int i = _str.size()-1, exponent = 0; i>= 0; i--, exponent++)
    if(_str[i] == '1') decimal += pow(2,exponent);
  return decimal;
}

uint sha1(str id){
  /* it will calculate an unique key for id string */
  int h0 = 0b01100111010001010010001100000001,\
      h1 = 0b11101111110011011010101110001001,\
      h2 = 0b10011000101110101101110011111110,\
      h3 = 0b00010000001100100101010001110110,\
      h4 = 0b11000011110100101110000111110000;
  idBin = strToBin(id);
  int t = h0 & h1;
  cout << "h0 and h1 = "<< t;
}


int main(int argc, char const *argv[]) {
  sha1("sha1");
  // int bit = 0x02;   //               0010
  // bit |= 1;         // OR  0001 ->   0011
  // bit ^= 1;         // XOR 0001 ->   0010
  // bit ^= 7;         // XOR 0111 ->   0101
  // bit &= 14;        // AND 1110 ->   0100
  // bit <<= 1;        // LSHIFT 1 ->   1000
  // bit >>= 2;        // RSHIFT 2 ->   0010
  // bit = ~bit;       // COMPLEMENT -> 1101
  return 0;
}
