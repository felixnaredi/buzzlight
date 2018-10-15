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
#include <vector>
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
  unsigned ExplicitContext = (ExplicitGetContext << 0) |
                                 (ExplicitSetContext << 1) |
                                 (ExplicitHelpContext << 2);
  if(!(ExplicitContext == 0 ||
       ExplicitContext == 1 ||
       ExplicitContext == 2 ||
       ExplicitContext == 4))
    return false;

  // A get context may not have a value, increase or decrease
  if(ExplicitGetContext &&
     (HasValue || IncreaseBrightness || DecreaseBrightness))
    return false;

  // A set context specific
  if(ExplicitSetContext) {
    if(!HasValue)
      return false;
    if(RawValueExpression && PercentExpression)
      return false;
  }

  // Value must be valid if set
  if(HasValue) {
    if(Value < 0)
      return false;
  }

  // Either both min and max is set or none
  if(!((HasMinValue && HasMaxValue) || (!HasMinValue && !HasMaxValue)))
    return false;

  // MinValue and MaxValue must be set to valid values if set.
  if(HasMinValue) {
    if(Min < 0)
      return false;
  }
  if(HasMaxValue) {
    // MaxValue must be greater than MinValue
    if(Max < 0 || Max <= Min)
      return false;
  }

  // Increase and decrease can't be set at the same time.
  if(IncreaseBrightness && DecreaseBrightness)
    return false;

  return true;
}
