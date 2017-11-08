/* This page explain the sha1 algorithm
  http://www.metamorphosite.com/one-way-hash-encryption-sha1-data-software */

#include <iostream>
#include <string>
#include <cmath>
#include <bitset>
#include <vector>

using namespace std;
typedef string str;

str strToBin(str _str){
  /* it will turn a string of alphabet letter in 8 bits binary groups for
  each letter */
  str binary = "";
  size_t strSize = _str.size();
  for(size_t c = 0; c < strSize; c++){
    bitset<8> bin(_str.c_str()[c]);
    binary += bin.to_string();
  }
  return binary;
}

int strBinToNum(str _str){
  /* it will turn a string of binary numbers to decimal value */
  int decimal = 0;
  for(int i = _str.size()-1, exponent = 0; i>= 0; i--, exponent++)
    if(_str[i] == '1') decimal += pow(2,exponent);
  return decimal;
}

void fillWithZeroes(str& strBin, uint zeroes){
  /* it will fill with zeroes */
  for(uint i=0; i<zeroes; i++) strBin.push_back('0');
}

void wordsByChunk(uint start,str& strBin,const uint& h0,const uint& h1, \
          const uint& h2,const uint& h3,const uint& h4){
  /* it will create 80 words from given chunk (chunks are always 512 bits
     length) */

  vector<str> words;
  words.resize(80);

  // breaking 512 chunk in 16 initial words of 32 bits length
  uint wordsAmount = 512/32;
  for(uint i=1,w=0; i<=wordsAmount; i++,w++){
    uint _end = (i*32) + start;
    uint _start = _end - 32;
    words[w] = strBin.substr(_start,32);
  }


}

uint sha1(str _str){
  /* it will calculate an unique key for _str */
  uint h0 = 0b01100111010001010010001100000001,\
      h1 = 0b11101111110011011010101110001001,\
      h2 = 0b10011000101110101101110011111110,\
      h3 = 0b00010000001100100101010001110110,\
      h4 = 0b11000011110100101110000111110000;

  // ------------- padding section -------------
  // we need to build a 512 bits package, this section will try to build the
  // first part (448), then complete the leftover part (448 + 64 = 512)

  // building the first part (448 bits)
  str strBin = strToBin(_str); // example -> this will turn "hull" word to \
                              "01101000 01110101 01101100 01101100"
  strBin.push_back('1');      // padding bit
  int zeroes = 0, bitsLength = strBin.size();
  if(bitsLength % 512 <= 448)  zeroes = 448 - (bitsLength % 512);
  else zeroes = 448 + 512 - (bitsLength % 512);
  fillWithZeroes(strBin,zeroes);

  // building the second part (64 bits)
  size_t strSize = _str.size();
  zeroes = 64 - sizeof(strSize)*8;
  fillWithZeroes(strBin,zeroes);
  strBin += bitset<sizeof(strSize)*8>(strSize*8).to_string();

  // breaking strBin in 512 chunks
  uint chunks = strBin.size()/512;
  for(uint ch=1; ch<=chunks; ch++){
    uint end = ch*512;
    uint start = end - 512;
    wordsByChunk(start,strBin,h0,h1,h2,h3,h4);
  }
}


int main(int argc, char const *argv[]) {
  sha1("A Test");
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
