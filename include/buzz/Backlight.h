//===- Backlight.h ----------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Frontend program communicating with the daemon.
//===----------------------------------------------------------------------===//

#ifndef BUZZ_BACKLIGHT_H
#define BUZZ_BACKLIGHT_H

#include "buzz/DBus/DBusObject.h"
#include "buzz/DBus/Property.h"
#include <cstdint>

namespace buzz {

class Backlight {
  dbus::DBusObject *Object;

public:
  Backlight();

  dbus::WritableProperty<std::int32_t> Brightness;
  dbus::Property<std::int32_t> MaxBrightness;
  dbus::Property<bool> IsEnabled;
  dbus::Property<bool> IsReady;

  void toggleBacklight(std::uint32_t MilliSec);
  void setBrightnessSmooth(std::int32_t Value, std::uint32_t MilliSec);
};

} // buzz

#endif // BUZZ_BACKLIGHT_H
