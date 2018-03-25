#include <iostream>
#include <unordered_map>
#include <vector>
#include <set>
#include <string>
/*
//table for (a|b)*abb(a|b)* final DFA
std::vector<std::unordered_map<char, std::vector<int>>> table = {
  { {255, {}}, {'a', {1}}, {'b', {0}} },    //0
  { {255, {}}, {'a', {1}}, {'b', {2}} },       //1
  { {255, {}}, {'a', {1}}, {'b', {3}} },       //2
  { {255, {}}, {'a', {3}}, {'b', {3}} }       //3
};
std::set<int> endstate = {3};
//*/

/*
// table for (a|b)*abb(a|b)* NFA->DFA
std::vector<std::unordered_map<char, std::vector<int>>> table = {
  { {255, {}}, {'a', {1}}, {'b', {2}} },    //0
  { {255, {}}, {'a', {1}}, {'b', {3}} },       //1
  { {255, {}}, {'a', {1}}, {'b', {2}} },       //2
  { {255, {}}, {'a', {1}}, {'b', {4}} },       //3
  { {255, {}}, {'a', {5}}, {'b', {6}} },       //4
  { {255, {}}, {'a', {5}}, {'b', {7}} },       //5
  { {255, {}}, {'a', {5}}, {'b', {6}} },       //6
  { {255, {}}, {'a', {5}}, {'b', {6}} }       //7
};
std::set<int> endstate = {5, 6, 7, 8};
//*/

/*
// table for (a|b)*abb(a|b)* NFA
std::vector<std::unordered_map<char, std::vector<int>>> table = {
  { {255, {1, 7}}, {'a', {}}, {'b', {}} },    //0
  { {255, {2, 4}}, {'a', {}}, {'b', {}} },       //1
  { {255, {}}, {'a', {3}}, {'b', {}} },       //2
  { {255, {6}}, {'a', {}}, {'b', {}} },       //3
  { {255, {}}, {'a', {}}, {'b', {5}} },       //4
  { {255, {6}}, {'a', {}}, {'b', {}} },       //5
  { {255, {1, 7}}, {'a', {}}, {'b', {}} },       //6
  { {255, {}}, {'a', {8}}, {'b', {}} },       //7
  { {255, {}}, {'a', {}}, {'b', {9}} },       //8
  { {255, {}}, {'a', {}}, {'b', {10}} },       //9
  { {255, {11, 17}}, {'a', {}}, {'b', {}} },       //10
  { {255, {12, 14}}, {'a', {}}, {'b', {}} },       //11
  { {255, {}}, {'a', {13}}, {'b', {}} },       //12
  { {255, {16}}, {'a', {}}, {'b', {}} },       //13
  { {255, {}}, {'a', {}}, {'b', {15}} },       //14
  { {255, {16}}, {'a', {}}, {'b', {}} },       //15
  { {255, {11, 17}}, {'a', {}}, {'b', {}} },       //16
  { {255, {}}, {'a', {}}, {'b', {}} }       //17
};
std::set<int> endstate = {17};
*/

//*
// table for aa*|bb*
std::vector<std::unordered_map<char, std::vector<int>>> table = {
  { {255, {1, 3}}, {'a', {}}, {'b', {}} },
  { {255, {}}, {'a', {2}}, {'b', {}} },
  { {255, {}}, {'a', {2}}, {'b', {}} },
  { {255, {}}, {'a', {}}, {'b', {4}} },
  { {255, {}}, {'a', {}}, {'b', {4}} }
};
std::set<int> endstate = {2, 4};
//*/

/*
// table for (a|b)*ab
std::vector<std::unordered_map<char, std::vector<int>>> table = {
  { {255, {}}, {'a', {0, 1}}, {'b', {0}} },
  { {255, {}}, {'a', {}}, {'b', {2}} },
  { {255, {}}, {'a', {}}, {'b', {}} }
};
std::set<int> endstate = {2};
*/

std::string s;
/*
int func(int c, int end, int state) {
  for (int it : table[state][255]) {
    if (func(c, end, it)) {
      return 1;
    } 
  }
  if (c == end) {
    if (endstate.find(state) != endstate.end() ) {
      return 1;
    } else {
      return 0;
    }
  }
  for (int it : table[state][s[c]]) {
    if (func(c + 1, end, it)) {
      return 1;
    }
  }
  return 0;
}
*/

int func(int c, int state) {
  int end = 0;
  if (c == s.size()) {
    if (endstate.find(state) != endstate.end()) {
      return c;
    } else {
      return 0;
    }
  }
  if (endstate.find(state) != endstate.end() ) {
    end = c;
  }
  for (int it : table[state][255]) {
    int ret = func(c, it);
    if (ret > end) end = ret;
  }
  for (int it : table[state][s[c]]) {
    int ret = func(c + 1, it);
    if (ret > end) end = ret;
  }
  return end;
}

int main() {
  getline(std::cin, s);
  int begin = 0;
  while (begin < s.size()) {
    int ret = func(begin, 0);
    if (ret) {
      std::cout << "match found:";
      for (int i = begin; i < ret; i++) {
        std::cout << s[i];
      }
      std::cout << std::endl;
      begin = ret;
    } else {
      begin++;
    }
  }
  return 0;
}