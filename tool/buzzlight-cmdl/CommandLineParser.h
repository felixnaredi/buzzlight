//===- CommandLineParser.h --------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Parser for command line arguments.
//===----------------------------------------------------------------------===//

#ifndef BUZZ_COMMANDLINEPARSER_H
#define BUZZ_COMMANDLINEPARSER_H

#include <string>

namespace buzz {

struct CommandOptions {
  unsigned ExplicitGetContext : 1;
  unsigned ExplicitSetContext : 1;
  unsigned ExplicitHelpContext : 1;
  unsigned HasMinValue : 1;
  unsigned HasMaxValue : 1;
  unsigned HasValue : 1;
  unsigned IncreaseBrightness : 1;
  unsigned DecreaseBrightness : 1;
  unsigned RawValueExpression : 1;
  unsigned PercentExpression : 1;
  unsigned HasDuration : 1;
  unsigned ToggleBacklight : 1;
  unsigned InvalidOption : 1;

  int Min;
  int Max;
  int Value;
  int Duration;

  CommandOptions()
      : ExplicitGetContext(0),
        ExplicitSetContext(0),
        ExplicitHelpContext(0),
        HasMinValue(0),
        HasMaxValue(0),
        HasValue(0),
        IncreaseBrightness(0),
        DecreaseBrightness(0),
        RawValueExpression(0),
        PercentExpression(0),
        HasDuration(0),
        ToggleBacklight(0),
        InvalidOption(0) {}

  static std::string HelpString();
  static CommandOptions parseCommandLineArgs(const char **Argv, int Argc);

  bool isValid();

  bool hasExplicitContext() {
    return ExplicitGetContext || ExplicitSetContext || ExplicitHelpContext ||
        ToggleBacklight;
  }
};

} // buzz

#endif // BUZZ_COMMANDLINEPARSER_H
