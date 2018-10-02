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

namespace buzz {

class Backlight {
  static dbus::Object BacklightDBusObject;
public:
  Backlight();

  dbus::TrivialProperty Brightness;
  dbus::TrivialProperty MaxBrightness;
  dbus::TrivialProperty Ready;
  dbus::TrivialProperty BacklightEnabled;
};

};

#endif // BUZZ_BACKLIGHT_H
