//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Program for buzzlight daemon.
//===----------------------------------------------------------------------===//

#include "buzz/DBus/DBusObject.h"
#include "buzz/BacklightDaemon.h"

using namespace buzz;

int main(int argc, char **argv) {
  BacklightDaemon DB;
  auto Table = DB.spawnVirtualTable();
  DB.addVirtualTable(Table.get());
  DB.runFor(10000);

  return 0;
}
