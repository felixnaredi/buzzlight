//===- BacklightDaemon.h ----------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Set up for the dbus daemon.
//===----------------------------------------------------------------------===//

#ifndef BUZZ_BACKLIGHTDAEMON_H
#define BUZZ_BACKLIGHTDAEMON_H

#include "buzz/DBus/DBusObject.h"
#include <cstdint>
#include <memory>
#include <systemd/sd-bus.h>

namespace buzz {

class BacklightDaemon : public dbus::DBusObject {
  const std::int32_t MaxBrightness;
  std::int32_t StoredBrightness;
  bool Ready;

public:
  BacklightDaemon();

  virtual std::unique_ptr<sd_bus_vtable[]> spawnVirtualTable();

  std::int32_t getBrightness() const;
  void setBrightness(std::int32_t Value);

  std::int32_t getMaxBrightness() const {
    return MaxBrightness;
  }

  std::int32_t getStoredBrightness() const {
    return StoredBrightness;
  }

  bool getIsReady() const {
    return Ready;
  }

  bool getIsEnabled() const;

  bool toggleBacklight(std::uint32_t MilliSec) const;
  void setBrightnessSmooth(std::int32_t Value, std::uint32_t MilliSec);
};

}

#endif // BUZZ_BACKLIGHTDAEMON_H
