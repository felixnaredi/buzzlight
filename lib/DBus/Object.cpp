//===- Object.cpp -----------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Implements DBus object.
//===----------------------------------------------------------------------===//

#include "buzz/DBus/Object.h"
#include <stdexcept>
#include <systemd/sd-bus.h>

using namespace buzz;
using namespace dbus;

sd_bus *defaultBus(DefaultBus K) {
  sd_bus *Bus;
  int R;

  switch(K) {
  case DefaultBus::System:
    R = sd_bus_default_system(&Bus);
    break;
  case DefaultBus::User:
    R = sd_bus_default_user(&Bus);
    break;
  }

  if(R < 0)
    throw std::runtime_error(strerror(-R));

  return Bus;
}


Object::Object(DefaultBus K,
               const char *destination,
               const char *path,
               const char *interface)
    : Object(defaultBus(K), destination, path, interface) {}
