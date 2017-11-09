/* This page explain the sha1 algorithm
  http://www.metamorphosite.com/one-way-hash-encryption-sha1-data-software */

#include <iostream>
#include <string>
#include <cmath>
#include <bitset>
#include <vector>

using namespace std;
typedef string str;

void leftRotate(str& strBin,uint bits){
  /* it will rotate leftmost bits from strBin toward rightmost */
  str leftmost = strBin.substr(0,bits);
  strBin.erase(0,bits);
  strBin.append(leftmost);
}

void _not(str& strBin){
  /* it will negate strBin */
  str newStrBin = "";
  size_t strBinSize = strBin.size();
  for(uint c = 0; c < strBinSize; c++){
    if(strBin[c] == '1') newStrBin.push_back('0');
    else newStrBin.push_back('1');
  }
  strBin = newStrBin;
}

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

uint strBinToNum(str _str){
  /* it will turn a string of binary numbers to decimal value */
  uint decimal = 0;
  for(int i = _str.size()-1, exponent = 0; i>= 0; i--, exponent++)
    if(_str[i] == '1') decimal += pow(2,exponent);
  return decimal;
}

void fillWithZeroes(str& strBin, uint zeroes){
  /* it will fill with zeroes */
  for(uint i=0; i<zeroes; i++) strBin.push_back('0');
}

void wordsByChunk(uint start,str& strBin,const str& h0,const str& h1, \
          const str& h2,const str& h3,const str& h4){
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

  // completing wordsAmount to 80 words
  wordsAmount += 64;
  for(uint i=16; i<wordsAmount; i++){
    // performing xor operations
    uint xorEd = strBinToNum(words[i-3]) xor strBinToNum(words[i-8]);
    xorEd = xorEd xor strBinToNum(words[i-14]);
    xorEd = xorEd xor strBinToNum(words[i-16]);

    // carrying leftmost bit to rightmost bit
    str newWord = bitset<32>(xorEd).to_string();
    leftRotate(newWord,1);

    // storing new word
    words[i] = newWord;
  }

  // performing four choices, each 20 iterations will choose a different \
    function
  str A = h0, B = h1, C = h2, D = h3, E = h4, F = "", K = "";
  for(uint w=0; w<wordsAmount; w++){
    if(w < 20){
      funct1(); // F = (B AND C) OR (!B AND D)
      K = "01011010100000100111100110011001";
    }
    else if(w >= 20 && w < 40){
      funct2(); // F = B XOR C XOR D
      K = "01101110110110011110101110100001";
    }
    else if(w >= 40 && w < 60){
      funct3(); // F = (B AND C) OR (B AND D) OR (C AND D)
      K = "10001111000110111011110011011100";
    }
    else{
      funct2(); //F = B XOR C XOR D
      K = "11001010011000101100000111010110";
    }
    D = C; leftRotate(B,30); C = B; B = A; //A = temp;
  }
}

uint sha1(str _str){
  /* it will calculate an unique key for _str */
  str h0 = "01100111010001010010001100000001",\
      h1 = "11101111110011011010101110001001",\
      h2 = "10011000101110101101110011111110",\
      h3 = "00010000001100100101010001110110",\
      h4 = "11000011110100101110000111110000";

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
