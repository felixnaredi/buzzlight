//===- Object.cpp -----------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Implements DBus object.
//===----------------------------------------------------------------------===//

#include "buzz/DBus/Object.h"
#include <cassert>
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
    : Object(defaultBus(K), nullptr, destination, path, interface) {}

Object::~Object() {
  sd_bus_slot_unref(Slot);
  sd_bus_unref(Bus);
}

void Object::run() {
  assert(Bus);
  while(1) {
    int R = sd_bus_process(Bus, NULL);
    if(R < 0)
      throw std::runtime_error(strerror(-R));
    if(R > 0)
      continue;
    R = sd_bus_wait(Bus, (uint64_t) -1);
    if(R < 0)
      throw std::runtime_error(strerror(-R));
  }
}

void Object::runFor(unsigned MilliSec) {
  assert(Bus);
  unsigned Dur = 0;
  while(Dur < MilliSec) {
    int R = sd_bus_process(Bus, NULL);
    if(R < 0)
      throw std::runtime_error(strerror(-R));
    if(R > 0)
      continue;
    R = sd_bus_wait(Bus, (uint64_t) 1000);
    if(R < 0)
      throw std::runtime_error(strerror(-R));
    Dur += 1;
  }
}

void Object::addVirtualTable(sd_bus_vtable *Table) {
  assert(Table);
  int R = sd_bus_add_object_vtable(Bus,
                                   &Slot,
                                   Path,
                                   Destination,
                                   Table,
                                   NULL);
  if(R < 0)
    throw std::runtime_error(strerror(-R));

  R = sd_bus_request_name(Bus, Interface, 0);
  if(R < 0)
    throw std::runtime_error(strerror(-R));
}
