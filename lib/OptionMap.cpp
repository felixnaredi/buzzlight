//===- OptionMap.cpp --------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Implements OptionMap.
//===----------------------------------------------------------------------===//

#include "OptionMap.h"
#include <map>
#include <string>
#include <functional>
#include <stdexcept>

using namespace buzz;

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
