//===- CMP.cpp --------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Description goes here
//===----------------------------------------------------------------------===//

#include "buzz/CommandLineParser.h"
#include <iostream>

using namespace buzz;

int main(int argc, char **argv) {
  auto Cmd = CommandLineParser::parseCommandLine(argv, argc);
  if(Cmd == nullptr) {
    std::cout << "Invalid command\n";
    return -1;
  }
  auto GetCmd = static_cast<GetContextCommand *>(Cmd.get());
  switch(Cmd->Context) {
  case CommandOptionContext::Get:
    std::cout << "Context: Get, Min: " << GetCmd->Min << "\n";
    break;
  case CommandOptionContext::Set:
    std::cout << "Context: Set\n";
    break;
  case CommandOptionContext::Help:
    std::cout << "Context: Help\n";
    break;
  }
  return 0;
}
