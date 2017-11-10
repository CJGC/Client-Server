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

str _not(const str& strBin){
  /* it will negate strBin */
  str negStrBin = "";
  size_t strBinSize = strBin.size();
  for(uint c = 0; c < strBinSize; c++){
    if(strBin[c] == '1') negStrBin.push_back('0');
    else negStrBin.push_back('1');
  }
  return negStrBin;
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

void funct1(const str& B,const str& C,const str& D,str& F){
  /* it will set F = (B AND C) OR (!B AND D) */
  uint and1 = strBinToNum(B) & strBinToNum(C);
  uint and2 = strBinToNum(_not(B)) & strBinToNum(D);
  uint or1 = and1 | and2;
  F = bitset<32>(or1).to_string();
}

void funct2(const str& B,const str& C,const str& D,str& F){
  /* it will set F = B XOR C XOR D */
  uint xor1 = strBinToNum(B) ^ strBinToNum(C);
  uint xor2 = xor1 ^ strBinToNum(D);
  F = bitset<32>(xor2).to_string();
}

void funct3(const str& B,const str& C,const str& D,str& F){
  /* it will set F = (B AND C) OR (B AND D) OR (C AND D) */
  uint and1 = strBinToNum(B) & strBinToNum(C);
  uint and2 = strBinToNum(B) & strBinToNum(D);
  uint and3 = strBinToNum(C) & strBinToNum(D);
  uint or1 = and1 | and2;
  uint or2 = or1 | and3;
  F = bitset<32>(or2).to_string();
}

void processChunk(uint start,str& strBin,str& h0,str& h1,str& h2,str& h3, \
                  str& h4){
  /* it will process a given chunk (chunks are always 512 bits length) */

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
    uint xorEd = strBinToNum(words[i-3]) ^ strBinToNum(words[i-8]);
    xorEd = xorEd ^ strBinToNum(words[i-14]);
    xorEd = xorEd ^ strBinToNum(words[i-16]);

    // carrying leftmost bit to rightmost bit
    str newWord = bitset<32>(xorEd).to_string();
    leftRotate(newWord,1);

    // storing new word
    words[i] = newWord;
  }

  // performing binary operations, each 20 iterations will choose a different \
    operation from a given function
  str A = h0, B = h1, C = h2, D = h3, E = h4, F = "", K = "";
  for(uint w=0; w<wordsAmount; w++){
    if(w < 20){
      funct1(B,C,D,F); // F = (B AND C) OR (!B AND D)
      K = "01011010100000100111100110011001";
    }
    else if(w < 40){
      funct2(B,C,D,F); // F = B XOR C XOR D
      K = "01101110110110011110101110100001";
    }
    else if(w < 60){
      funct3(B,C,D,F); // F = (B AND C) OR (B AND D) OR (C AND D)
      K = "10001111000110111011110011011100";
    }
    else{
      funct2(B,C,D,F); //F = B XOR C XOR D
      K = "11001010011000101100000111010110";
    }

    str oldA = A;
    leftRotate(A,5);
    ulong temp = strBinToNum(A) + strBinToNum(F) + \
       strBinToNum(E) + strBinToNum(K) + strBinToNum(words[w]);

    E = D; D = C;
    leftRotate(B,30);
    C = B; B = oldA;
    A = bitset<32>(temp).to_string();
  }

  // setting hn values to the next 512 bits chunk
  ulong uh0 = strBinToNum(h0) + strBinToNum(A);
  ulong uh1 = strBinToNum(h1) + strBinToNum(B);
  ulong uh2 = strBinToNum(h2) + strBinToNum(C);
  ulong uh3 = strBinToNum(h3) + strBinToNum(D);
  ulong uh4 = strBinToNum(h4) + strBinToNum(E);

  h0 = bitset<32>(uh0).to_string();
  h1 = bitset<32>(uh1).to_string();
  h2 = bitset<32>(uh2).to_string();
  h3 = bitset<32>(uh3).to_string();
  h4 = bitset<32>(uh4).to_string();
}

str getHex(uint& decimal){
  /* it will turn decimal number to hexadecimal number */
  str hexVal = "";
  if(decimal > 9) hexVal = (char)(decimal - 10 + 'a');
  else hexVal = to_string(decimal);
  return hexVal;
}

str turnGroupToHex(str& hi,uint start){
  /* it will buld a 4 bits group and turn in hexadecimal notation */
  str group = hi.substr(start,4);
  uint decimal = strBinToNum(group);
  return getHex(decimal);
}

str breakHi(str& hi){
  /* it will break each 32 bits length hi value, into 4 bits groups */
  str hexadecimal = "";
  size_t hnParts = hi.size()/4;
  for(uint hnP = 1; hnP <= hnParts; hnP++){
    uint end = 4*hnP;
    uint start = end - 4;
    hexadecimal += turnGroupToHex(hi,start);
  }
  return hexadecimal;
}

str turnToHex(str& h0,str& h1,str& h2,str& h3,str& h4){
  /* it will concatenate each turned hn value into hexadecimal string */
  vector<str> hn; hn.resize(5);
  hn[0] = h0; hn[1] = h1; hn[2] = h2; hn[3] = h3; hn[4] = h4;

  str hexadecimal = "";
  size_t hnGroupSize = hn.size();
  for(uint hi=0; hi<hnGroupSize; hi++){
    hexadecimal += breakHi(hn[hi]);
  }

  return hexadecimal;
}

str sha1(str _str){
  /* it will calculate an unique key for _str */
  str h0 = "01100111010001010010001100000001";
  str h1 = "11101111110011011010101110001001";
  str h2 = "10011000101110101101110011111110";
  str h3 = "00010000001100100101010001110110";
  str h4 = "11000011110100101110000111110000";

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

  // breaking strBin in 512 chunks, each chunk will set hn new values
  uint chunks = strBin.size()/512;
  for(uint ch=1; ch<=chunks; ch++){
    uint end = ch*512;
    uint start = end - 512;
    processChunk(start,strBin,h0,h1,h2,h3,h4);
  }

  // building hexadecimal notation with hn values
  return turnToHex(h0,h1,h2,h3,h4);
}


int main(int argc, char const *argv[]) {
  cout << sha1("A Test")<<endl;
  return 0;
}
