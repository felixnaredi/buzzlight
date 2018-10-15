//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Main program for the command line tool.
//===----------------------------------------------------------------------===//

#include "CommandLineParser.h"
#include "buzz/Backlight.h"
#include <cassert>
#include <vector>
#include <string>
#include <numeric>
#include <sstream>
#include <iostream>

using namespace buzz;

struct Ratio {
  int Num;
  int Denum;

  Ratio(int num, int denum) : Num(num), Denum(denum) {}

  int relative(int Value, int MaxValue) {
    assert(Value < MaxValue);

    double R = static_cast<double>(Value) / static_cast<double>(MaxValue);
    return static_cast<int>(static_cast<double>(Denum) * R);
  }
};

std::vector<std::string> getCommandReps(
    CommandOptions &Options,
    int Brightness,
    int MaxBrightness,
    std::vector<std::string> *Vec = nullptr) {

  if(Vec == nullptr)
    Vec = new std::vector<std::string>();

  std::stringstream SS;

  if(Options.HasMinValue && Options.HasMaxValue) {
    SS << Ratio(Options.Min, Options.Max).relative(Brightness, MaxBrightness)
       << '/'
       << Options.Max;
    Vec->push_back(SS.str());

    Options.HasMinValue = 0;
    Options.HasMaxValue = 0;
    return getCommandReps(Options, Brightness, MaxBrightness, Vec);
  }

  if(Options.PercentExpression) {
    SS << Ratio(0, 100).relative(Brightness, MaxBrightness) << '%';
    Vec->push_back(SS.str());

    Options.PercentExpression = 0;
    return getCommandReps(Options, Brightness, MaxBrightness, Vec);
  }

  if(Options.RawValueExpression) {
    SS << Brightness << "/" << MaxBrightness;
    Vec->push_back(SS.str());

    Options.RawValueExpression = 0;
    return getCommandReps(Options, Brightness, MaxBrightness, Vec);
  }

  return *Vec;
};

int main(int argc, char **argv) {
  CommandOptions Options = CommandOptions::parseCommandLineArgs(
      const_cast<const char **>(argv),
      argc);

  if(!Options.isValid()) {
    std::cout << "Invalid command\n";
    return -1;
  }

  Backlight Interface;

  if(!Options.hasExplicitContext() || Options.ExplicitGetContext) {
    auto ValueReps = getCommandReps(Options,
                                    Interface.Brightness.get(),
                                    Interface.MaxBrightness.get());
    std::cout << std::accumulate(std::next(std::begin(ValueReps)),
                                 std::end(ValueReps),
                                 ValueReps[0],
                                 [](std::string &Acc, std::string &E) {
                                   return Acc + ", " + E;
                                 })
              << '\n';
    return 0;
  }

  return 0;
}
