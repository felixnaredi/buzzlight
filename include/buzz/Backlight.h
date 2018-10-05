//===- Backlight.h ----------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Communicates with DBus object to control backlight.
//===----------------------------------------------------------------------===//

#ifndef BUZZ_BACKLIGHT_H
#define BUZZ_BACKLIGHT_H

#include "buzz/DBus.h"
#include <cstdint>

namespace buzz {

class Backlight {
  static dbus::Object BacklightDBusObject;
public:
  Backlight();

  dbus::TrivialProperty Brightness;
  dbus::TrivialProperty MaxBrightness;
  dbus::TrivialProperty Ready;
  dbus::TrivialProperty BacklightEnabled;

  void setBrightnessSmooth(std::int32_t Value, std::uint32_t US, bool Store);
};

}

#endif // BUZZ_BACKLIGHT_H
