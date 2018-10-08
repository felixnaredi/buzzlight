//===- DBusObject.h ---------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Description goes here
//===----------------------------------------------------------------------===//

#ifndef BUZZ_DBUS_DBUSOBJECT_H
#define BUZZ_DBUS_DBUSOBJECT_H

#include <memory>
#include <systemd/sd-bus.h>

namespace buzz {
namespace dbus {

enum class DefaultBus {
  System,
  User,
};

struct DBusObject {
  sd_bus *Bus;
  sd_bus_slot *Slot;
  const char *Destination;
  const char *Path;
  const char *Interface;

  DBusObject(sd_bus *bus,
         sd_bus_slot *slot,
         const char *destination,
         const char *path,
         const char *interface)
      : Bus(bus),
        Slot(slot),
        Destination(destination),
        Path(path),
        Interface(interface) {}

  DBusObject(DefaultBus K,
         const char *destination,
         const char *path,
         const char *interface);

  DBusObject(dbus::DBusObject *Obj)
      : DBusObject(Obj->Bus,
                   Obj->Slot,
                   Obj->Destination,
                   Obj->Path,
                   Obj->Interface) {}

  ~DBusObject();

  void run();
  void runFor(unsigned MilliSec);

  void addVirtualTable(sd_bus_vtable *Table);

  virtual std::unique_ptr<sd_bus_vtable[]> spawnVirtualTable() {
    return nullptr;
  }
};

} // namespace: dbus
} // namespace: buzz

#endif // BUZZ_DBUS_DBUSOBJECT_H
