//===- Object.h -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Description goes here
//===----------------------------------------------------------------------===//

#ifndef BUZZ_DBUS_OBJECT_H
#define BUZZ_DBUS_OBJECT_H

#include <memory>
#include <systemd/sd-bus.h>

namespace buzz {
namespace dbus {

enum class DefaultBus {
  System,
  User,
};

struct Object {
  sd_bus *Bus;
  sd_bus_slot *Slot;
  const char *Destination;
  const char *Path;
  const char *Interface;

  Object(sd_bus *bus,
         sd_bus_slot *Slot,
         const char *destination,
         const char *path,
         const char *interface)
      : Bus(bus), Destination(destination), Path(path), Interface(interface) {}

  Object(DefaultBus K,
         const char *destination,
         const char *path,
         const char *interface);

  Object(dbus::Object *Obj)
      : Object(Obj->Bus,
               Obj->Slot,
               Obj->Destination,
               Obj->Path,
               Obj->Interface) {}

  ~Object();

  void run();
  void runFor(unsigned MilliSec);

  void addVirtualTable(sd_bus_vtable *Table);

  virtual std::unique_ptr<sd_bus_vtable[]> spawnVirtualTable() {
    return nullptr;
  }
};

} // namespace: dbus
} // namespace: buzz

#endif // BUZZ_DBUS_OBJECT_H
