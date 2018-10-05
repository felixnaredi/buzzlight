//===- BacklightDaemon.h ----------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Set up for the dbus daemon.
//===----------------------------------------------------------------------===//

#ifndef BUZZ_DAEMON_SETUP_H
#define BUZZ_DAEMON_SETUP_H

#include "buzz/DBus/Object.h"
#include <cstdint>
#include <vector>
#include <systemd/sd-bus.h>

namespace buzz {

class BacklightDaemon {
  dbus::Object DBusObject;
  sd_bus_vtable *VirtualTable;

  const std::int32_t MaxBrightness;
  std::int32_t StoredBrightness;
  bool Ready;
  bool Enabled;

public:
  BacklightDaemon();

  dbus::Object *getObject() {
    return &DBusObject;
  }

  void run();

  std::int32_t getBrightness() const;

  std::int32_t getMaxBrightness() const {
    return MaxBrightness;
  }

  std::int32_t getStoredBrightness() const {
    return StoredBrightness;
  }

  bool getIsReady() const {
    return Ready;
  }

  bool getIsEnabled() const {
    return Enabled;
  }
};

}

#endif // BUZZ_DAEMON_SETUP_H
