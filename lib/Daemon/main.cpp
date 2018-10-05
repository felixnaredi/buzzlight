//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Program for buzzlight daemon.
//===----------------------------------------------------------------------===//

#include "buzz/DBus/Object.h"
#include "buzz/Daemon/BacklightDaemon.h"
#include <iostream>

using namespace buzz;

int main(int argc, char **argv) {
  std::cout << "Hella daemon\n";
  BacklightDaemon Daemon;
  Daemon.run();
  return 0;
}
