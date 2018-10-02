//===- DBus.h ---------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Handles communication with the DBus.
//===----------------------------------------------------------------------===//

#ifndef BUZZ_DBUS_H
#define BUZZ_DBUS_H

#include <cstdint>
#include <systemd/sd-bus.h>

namespace buzz {
namespace dbus {

struct Object {
  sd_bus *Bus;
  const char *Destination;
  const char *Path;
  const char *Interface;

  Object(sd_bus *bus,
         const char *destination,
         const char *path,
         const char *interface)
      : Bus(bus),
        Destination(destination),
        Path(path),
        Interface(interface) {}
};

class TrivialProperty {
  Object *BusObject;
  const char *Member;
  char Type;

public:
  TrivialProperty(Object *busobject, const char *member, char type)
      : BusObject(busobject), Member(member), Type(type) {}

  std::uint64_t get();
};


} // dbus
} // buzz

#endif // BUZZ_DBUS_H
