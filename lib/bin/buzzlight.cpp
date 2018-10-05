//===- buzzlight.cpp --------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Frontend program.
//===----------------------------------------------------------------------===//

#include "buzz/DBus.h"
#include "buzz/Backlight.h"
#include "buzz/CommandLineParser.h"
#include <cstdint>
#include <iostream>

using namespace buzz;

static void smoothSet(Backlight &Bl, std::uint32_t Value) {
  Bl.setBrightnessSmooth(Value, 100, true);
}

int main(int argc, char **argv) {
  auto Cmd = CommandLineParser::parseCommandLine(argv, argc);
  if(!Cmd->Valid)
    return -1;

  if(Cmd->Context == CommandOptionContext::Get) {
    Backlight Bl;
    std::cout << Bl.Brightness.get()
              << " / "
              << Bl.MaxBrightness.get()
              << "\n";
    return 0;
  }

  if(Cmd->Context == CommandOptionContext::Set) {
    auto SetCmd = static_cast<SetContextCommand *>(Cmd.get());
    Backlight Bl;

    if(SetCmd->Value > -1)
      smoothSet(Bl, SetCmd->Value);
    else if(SetCmd->Increase > -1)
      smoothSet(Bl, Bl.Brightness.get() + SetCmd->Increase);
    else if(SetCmd->Decrease > -1)
      smoothSet(Bl, Bl.Brightness.get() - SetCmd->Decrease);
  }

  return 0;
}
