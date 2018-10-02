//===- OptionMap.cpp --------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Implements OptionMap.
//===----------------------------------------------------------------------===//

#include "OptionMap.h"
#include <cstring>
#include <map>
#include <string>
#include <stdexcept>

using namespace buzz;

OptionMap::OptionMap(char **Argv, unsigned Len) : Valid(true) {
  for(unsigned I = 1; I < Len; I++) {
    char *Key = Argv[I];
    char *CP = std::strchr(Key, '=');
    if(!CP) {
      if(contains(Key))
        Valid = false;
      operator[](Key) = "";
    }
    else {
      *CP = '\0';
      if(contains(Key))
        Valid = false;
      operator[](Key) = ++CP;
    }
  }
}

template <>
int OptionMap::get(const std::string &Key) const {
  const std::string &Val = at(Key);
  std::size_t I;
  int N = std::stoi(Val, &I);
  if(Val.length() != I)
    throw std::invalid_argument("Must only contain numbers 0-9");
  return N;
}

template <>
bool OptionMap::get(const std::string &Key) const {
  const std::string &Val = at(Key);
  if(Val == "" || Val == "true")
    return true;
  if(Val == "false")
    return false;
  throw std::invalid_argument("Value must be 'true' or 'false' or left out");
}
