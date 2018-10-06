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
#include <thread>
#include <chrono>

using namespace buzz;

int main(int argc, char **argv) {
  BacklightDaemon DB;
  auto Table = DB.spawnVirtualTable();
  DB.addVirtualTable(Table.get());
  DB.runFor(5000);

  return 0;
}
