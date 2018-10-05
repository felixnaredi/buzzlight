//===- Object.h -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Description goes here
//===----------------------------------------------------------------------===//

#ifndef BUZZ_DBUS_OBJECT_H
#define BUZZ_DBUS_OBJECT_H

#include <systemd/sd-bus.h>

namespace buzz {
namespace dbus {

enum class DefaultBus {
  System,
  User,
};

struct Object {
  sd_bus *Bus;
  const char *Destination;
  const char *Path;
  const char *Interface;

  Object(sd_bus *bus,
         const char *destination,
         const char *path,
         const char *interface)
      : Bus(bus), Destination(destination), Path(path), Interface(interface) {}

  Object(DefaultBus K,
         const char *destination,
         const char *path,
         const char *interface);
};

} // namespace: dbus
} // namespace: buzz

#endif // BUZZ_DBUS_OBJECT_H
