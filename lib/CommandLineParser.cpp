//===- CommandLineParser.cpp ------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Implements the command line parser.
//===----------------------------------------------------------------------===//

#include "OptionMap.h"
#include "buzz/CommandLineParser.h"
#include <cstring>
#include <string>
#include <algorithm>

using namespace buzz;

OptionMap::OptionMap(char **Argv, unsigned Len) {
  Valid = true;
  for(unsigned I = 1; I < Len; I++) {
    char *Key = Argv[I];
    char *CP = std::strchr(Key, '=');
    if(!CP) {
      if(contains(Key))
        Valid = false;
      (*this)[Key] = "";
    }
    else {
      *CP = '\0';
      if(contains(Key))
        Valid = false;
      (*this)[Key] = ++CP;
    }
  }
}

class OptionContextQuary {
  const static std::map<std::string, CommandOptionContext> ContentextMap;
public:
  CommandOptionContext Context;
  bool HasContext;
  bool Valid;

  OptionContextQuary()
      : HasContext(false), Valid(true) {}

  OptionContextQuary(const OptionMap &Map) : OptionContextQuary() {
    for(auto It = ContentextMap.cbegin(), End = ContentextMap.cend();
        It != End;
        ++It) {
      if(!Map.contains(It->first))
        continue;
      if(HasContext || Map.at(It->first) != "") {
        Valid = false;
        return;
      }
      Context = It->second;
      HasContext = true;
    }
  }
};

Command::Command(const OptionMap &Map) {
  OptionContextQuary Q(Map);
  if(Q.HasContext)
    Context = Q.Context;
  else
    Context = CommandOptionContext::Get;
  Valid = Q.Valid;
}

GetContextCommand::GetContextCommand(const OptionMap &Map) : Command(Map) {
  if(!Valid)
    return;
  Min = Map.get<int>("-min", -1);
  Max = Map.get<int>("-max", -1);
  ShowRaw = Map.get<bool>("-show-raw", false);
  Valid = std::all_of(Map.cbegin(), Map.cend(),
                      [](auto &Opt) {
                        return Opt.first == "-get" ||
                            Opt.first == "-min" ||
                            Opt.first == "-max" ||
                            Opt.first == "-show-raw";
                      });
}

template <class Context>
std::unique_ptr<Context> makeContextCommand(const OptionMap &Map) {
  std::unique_ptr<Context> C(new Context(Map));
  if(!C->Valid)
    return nullptr;
  return C;
}

std::unique_ptr<Command> CommandLineParser::parseCommandLine(char **Argv, int Len) {
  OptionMap Map(Argv, Len);
  if(!Map.isValid())
    return nullptr;

  OptionContextQuary Q(Map);
  if(!Q.Valid)
    return nullptr;

  switch(Q.Context) {
  case CommandOptionContext::Get:
    return makeContextCommand<GetContextCommand>(Map);
  case CommandOptionContext::Set:
    return makeContextCommand<SetContextCommand>(Map);
  case CommandOptionContext::Help:
    return makeContextCommand<HelpContextCommand>(Map);
  }
  return nullptr;
}

const std::map<std::string, CommandOptionContext>
OptionContextQuary::ContentextMap = {
  {"-get", CommandOptionContext::Get},
  {"-set", CommandOptionContext::Set},
  {"-help", CommandOptionContext::Help},
};
