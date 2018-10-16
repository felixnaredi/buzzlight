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

struct Range {
  int Begin;
  int End;

  Range(int begin, int end) : Begin(begin), End(end) {
    assert(begin < end);
  }

  int Length() {
    return End - Begin;
  }

  int relative(int Value, int MaxValue) {
    assert(Value < MaxValue);

    double R = static_cast<double>(Value) / static_cast<double>(MaxValue);
    return static_cast<int>(static_cast<double>(Length()) * R) + Begin;
  }
};

static std::unique_ptr<std::vector<std::string>> getCommandReps(
    CommandOptions &&Options,
    int Brightness,
    int MaxBrightness,
    std::unique_ptr<std::vector<std::string>> Vec = nullptr) {

  std::stringstream SS;

  if(Options.HasMinValue && Options.HasMaxValue) {
    if(Vec == nullptr) {
      Vec = std::unique_ptr<std::vector<std::string>>(
          new std::vector<std::string>());
    }

    SS << Range(Options.Min, Options.Max).relative(Brightness, MaxBrightness)
       << '/'
       << Options.Max;
    Vec->push_back(SS.str());

    Options.HasMinValue = 0;
    Options.HasMaxValue = 0;
    return getCommandReps(std::move(Options),
                          Brightness,
                          MaxBrightness,
                          std::move(Vec));
  }

  if(Options.PercentExpression) {
    if(Vec == nullptr) {
      Vec = std::unique_ptr<std::vector<std::string>>(
          new std::vector<std::string>());
    }

    SS << Range(0, 100).relative(Brightness, MaxBrightness) << '%';
    Vec->push_back(SS.str());

    Options.PercentExpression = 0;
    return getCommandReps(std::move(Options),
                          Brightness,
                          MaxBrightness,
                          std::move(Vec));
  }

  if(Options.RawValueExpression) {
    if(Vec == nullptr) {
      Vec = std::unique_ptr<std::vector<std::string>>(
          new std::vector<std::string>());
    }

    SS << Brightness << "/" << MaxBrightness;
    Vec->push_back(SS.str());

    Options.RawValueExpression = 0;
    return getCommandReps(std::move(Options),
                          Brightness,
                          MaxBrightness,
                          std::move(Vec));
  }

  return Vec;
};

template <typename T>
T min(T a, T b) {
  return a < b ? a : b;
}

template <typename T>
T max(T a, T b) {
  return a > b ? a : b;
}

static int rawBacklightValue(CommandOptions &&Options,
                             int Brightness,
                             int MaxBrightness) {

  if(Options.RawValueExpression) {
    int Value = Options.Value;

    if(Options.IncreaseBrightness)
      Value += Brightness;
    if(Options.DecreaseBrightness)
      Value = Brightness - Value;

    return Value;
  }

  else if(Options.HasMinValue && Options.HasMaxValue) {
    if(Options.IncreaseBrightness || Options.DecreaseBrightness) {
      int V = Range(Options.Min, Options.Max).relative(Brightness,
                                                       MaxBrightness);
      V += Options.IncreaseBrightness ? Options.Value : -Options.Value;

      Options.Value = V;
      Options.IncreaseBrightness = 0;
      Options.DecreaseBrightness = 0;
      return rawBacklightValue(std::move(Options), Brightness, MaxBrightness);
    }

    int Value = max(min(Options.Value, Options.Max), Options.Min);

    return Range(0, MaxBrightness).relative(Value - Options.Min,
                                            Options.Max - Options.Min);
  }

  else if(Options.PercentExpression) {
    float V = static_cast<float>(Options.Value) / 100.0;

    if(Options.IncreaseBrightness)
      return static_cast<float>(Brightness) * (1 + V);
    if(Options.DecreaseBrightness)
      return static_cast<float>(Brightness) * (1 - V);

    return static_cast<float>(MaxBrightness) * V;
  }

  return -1;
}

int main(int argc, char **argv) {
  CommandOptions Options = CommandOptions::parseCommandLineArgs(
      const_cast<const char **>(argv),
      argc);

  if(!Options.isValid() || Options.ExplicitHelpContext) {
    std::cout << CommandOptions::HelpString();
    return 0;
  }

  Backlight Interface;

  if(!Options.hasExplicitContext() || Options.ExplicitGetContext) {
    int Brightness = Interface.Brightness.get();
    int MaxBrightness = Interface.MaxBrightness.get();

    auto ValueReps = getCommandReps(std::move(Options),
                                    Brightness,
                                    MaxBrightness);

    // Default implicit representation
    if(ValueReps == nullptr) {
      Options.PercentExpression = 1;
      ValueReps = getCommandReps(std::move(Options), Brightness, MaxBrightness);
    }

    std::cout << std::accumulate(std::next(std::begin(*ValueReps)),
                                 std::end(*ValueReps),
                                 (*ValueReps)[0],
                                 [](std::string &Acc, std::string &E) {
                                   return Acc + ", " + E;
                                 })
              << '\n';
    return 0;
  }

  if(Options.ToggleBacklight) {
    if(!Interface.IsReady.get())
      return 0;
    Interface.toggleBacklight(Options.HasDuration ? Options.Duration : 0);
    return 0;
  }

  if(Options.ExplicitSetContext) {
    if(!Interface.IsReady.get())
      return 0;
    int Value = rawBacklightValue(std::move(Options),
                                  Interface.Brightness.get(),
                                  Interface.MaxBrightness.get());
    if(!Options.HasDuration) {
      Interface.Brightness.set(Value);
      return 0;
    }

    Interface.setBrightnessSmooth(Value, Options.Duration);
  }


  return 0;
}
