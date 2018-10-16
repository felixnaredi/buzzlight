//===- CommandLineParser.cpp ------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Implements the command line parser.
//===----------------------------------------------------------------------===//

#include "CommandLineParser.h"
#include <cstddef>
#include <cassert>
#include <string>

using namespace buzz;

struct ArgumentPair {
  const std::string Key;
  const std::string Value;

  ArgumentPair(const std::string key, const std::string value)
      : Key(key), Value(value) {}

  static ArgumentPair parseArg(const std::string &Arg) {
    auto Offset = Arg.find_first_of("=");

    if(Offset == std::string::npos)
      return ArgumentPair(Arg, "");

    return ArgumentPair(Arg.substr(0, Offset), Arg.substr(Offset + 1));
  }

};

static int stringToInt(const std::string &S) noexcept {
  std::size_t Pos;
  int V = std::stoi(S, &Pos);
  if(Pos != S.length())
    return -1;
  return V;
}

CommandOptions CommandOptions::parseCommandLineArgs(const char **Argv,
                                                    int Argc) {
  assert(Argc > 0);

  CommandOptions Opt;

  for(auto It = Argv + 1, End = Argv + Argc; It != End; ++It) {
    ArgumentPair Arg = ArgumentPair::parseArg(*It);

    if(Arg.Key == "-get")
      Opt.ExplicitGetContext = 1;
    else if(Arg.Key == "-set")
      Opt.ExplicitSetContext = 1;
    else if(Arg.Key == "-help")
      Opt.ExplicitHelpContext = 1;
    else if(Arg.Key == "-min") {
      Opt.HasMinValue = 1;
      Opt.Min = stringToInt(Arg.Value);
    }
    else if(Arg.Key == "-max") {
      Opt.HasMaxValue = 1;
      Opt.Max = stringToInt(Arg.Value);
    }
    else if(Arg.Key == "-value") {
      Opt.HasValue = 1;
      Opt.Value = stringToInt(Arg.Value);
    }
    else if(Arg.Key == "-increase")
      Opt.IncreaseBrightness = 1;
    else if(Arg.Key == "-decrease")
      Opt.DecreaseBrightness = 1;
    else if(Arg.Key == "-raw-value")
      Opt.RawValueExpression = 1;
    else if(Arg.Key == "-percent")
      Opt.PercentExpression = 1;
    else if(Arg.Key == "-duration") {
      Opt.HasDuration = 1;
      Opt.Duration = stringToInt(Arg.Value);
    }
    else if(Arg.Key == "-toggle")
      Opt.ToggleBacklight = 1;
    else {
      Opt.InvalidOption = 1;
      break;
    }
  }

  return Opt;
}

bool CommandOptions::isValid() {
  if(InvalidOption)
    return false;

  // Only one explicit context can be set
  unsigned Context = (ExplicitGetContext << 0) |
                         (ExplicitSetContext << 1) |
                         (ExplicitHelpContext << 2) |
                         (ToggleBacklight << 3);
  if(!(Context == 0 ||
       Context == 1 ||
       Context == 2 ||
       Context == 4 ||
       Context == 8))
    return false;

  // A get context may not have a value, increase, decrease, duration or toggle
  if(ExplicitGetContext &&
     (HasValue ||
      IncreaseBrightness ||
      DecreaseBrightness ||
      HasDuration ||
      ToggleBacklight))
    return false;

  // A set context specific
  if(ExplicitSetContext) {
    if(!HasValue)
      return false;
    if(RawValueExpression && PercentExpression)
      return false;
  }

  // If toggle backlight is set nothing else may be set.
  if(ToggleBacklight &&
     (RawValueExpression ||
      PercentExpression ||
      HasValue ||
      HasMinValue ||
      HasMaxValue ||
      IncreaseBrightness ||
      DecreaseBrightness))
    return false;

  // Either both min and max is set or none
  if(HasMinValue != HasMaxValue)
    return false;

  // Increase and decrease can't be set at the same time.
  if(IncreaseBrightness && DecreaseBrightness)
    return false;

  // Values must be set to valid values
  if(HasValue) {
    if(Value < 0)
      return false;
  }
  if(HasDuration) {
    if(Duration < 0)
      return false;
  }
  if(HasMinValue) {
    if(Min < 0)
      return false;
  }
  if(HasMaxValue) {
    // MaxValue must be greater than MinValue
    if(Max < 0 || Max <= Min)
      return false;
  }

  return true;
}

std::string CommandOptions::HelpString() {
  return
      "USAGE: buzzlight-cmdl [options]\n"
      "-help                 Print this list\n"
      "-get                  Get brightness\n"
      "-set                  Set brightness\n"
      "-toggle               Toggle backlight\n"
      "-raw-value            Get/Sets raw value\n"
      "-percent              Get/Sets value in percent\n"
      "-min=<unsinged>       Get/Sets within min-max range\n"
      "-max=<unsigned>       Get/Sets within min-max range\n"
      "-duration=<unsigned>  Sets/Toggles smooth during duration in milleseconds\n"
      "-value=<unsigned>     Set brightness to value, required when using -set\n"
      "-increase             Sets brightness increased with value\n"
      "-decrease             Sets brightness decreased with value\n";
}
