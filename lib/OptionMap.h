//===- OptionMap.h ----------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// A custom map to parse command line arguments.
//===----------------------------------------------------------------------===//

#ifndef BUZZ_OPTIONMAP_H
#define BUZZ_OPTIONMAP_H

#include <map>
#include <string>

namespace buzz {

class OptionMap : public std::map<std::string, std::string> {
  bool Valid;

public:
  OptionMap(char **Argv, unsigned Len);

  template <typename Key>
  bool contains(const Key& K) const {
    return find(K) != cend();
  }

  bool isValid() const {
    return Valid;
  }

  template <typename T>
  T get(const std::string &Key) const;

  template <typename T>
  T get(const std::string &Key, T Default) const {
    if(!contains(Key))
      return Default;
    return get<T>(Key);
  }

};

};

#endif // BUZZ_OPTIONMAP_H
