//===- CommandLineParser.h --------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Parser for command line arguments.
//===----------------------------------------------------------------------===//

#ifndef BUZZ_COMMANDLINEPARSER_H
#define BUZZ_COMMANDLINEPARSER_H

#include <memory>

namespace buzz {

enum class CommandOptionContext {
  Get,
  Set,
  Help,
};

class OptionMap;

struct Command {
  CommandOptionContext Context;
  bool Valid;

  Command(CommandOptionContext C, bool V)
      : Context(C), Valid(V) {}

  Command(const OptionMap &Map);
};

class CommandLineParser {
public:
  static std::unique_ptr<Command> parseCommandLine(char **Argv, int Len);
};

struct GetContextCommand : public Command {
  int Min;
  int Max;
  bool ShowRaw;

  GetContextCommand(const OptionMap &Map);
};

struct SetContextCommand : public Command {
  int Min;
  int Max;
  int Value;
  int Increase;
  int Decrease;

  SetContextCommand(const OptionMap &Map);
};
struct HelpContextCommand : public Command {
  HelpContextCommand(const OptionMap &Map) : Command(Map) {}
};

}

#endif // BUZZ_COMMANDLINEPARSER_H
