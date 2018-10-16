//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Program for buzzlight daemon.
//===----------------------------------------------------------------------===//

#include "BacklightDaemon.h"
#include "buzz/DBus/DBusObject.h"

using namespace buzz;

int main(int argc, char **argv) {
  BacklightDaemon DB;

  auto Table = DB.spawnVirtualTable();
  DB.addVirtualTable(Table.get());

  DB.run();

  return 0;
}
