//===- DBus.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Description goes here
//===----------------------------------------------------------------------===//

#include "buzz/DBus.h"
#include <cstdint>
#include <stdexcept>
#include <systemd/sd-bus.h>

#include <iostream>

using namespace buzz;
using namespace dbus;

std::uint64_t TrivialProperty::get() {
  // Ret must be set to 0 before getting the value
  std::uint64_t Ret = 0;
  int R = sd_bus_get_property_trivial(BusObject->Bus,
                                      BusObject->Destination,
                                      BusObject->Path,
                                      BusObject->Interface,
                                      Member,
                                      NULL,
                                      Type,
                                      &Ret);
  if(R < 0)
    throw std::runtime_error(strerror(-R));
  return Ret;
}
