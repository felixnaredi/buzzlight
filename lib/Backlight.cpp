//===- Backlight.cpp --------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Implements backlight.
//===----------------------------------------------------------------------===//

#include "buzz/Backlight.h"
#include "buzz/DBus.h"
#include <systemd/sd-bus.h>

using namespace buzz;

sd_bus *systemBus() {
  sd_bus *bus = NULL;
  int R = sd_bus_default_system(&bus);
  if(R < 0)
    return nullptr;
  return bus;
}

dbus::Object Backlight::BacklightDBusObject(systemBus(),
                                            "git.felixnaredi.buzzlight",
                                            "/git/felixnaredi/buzzlight",
                                            "git.felixnaredi.buzzlight");

Backlight::Backlight() :
    Brightness(&BacklightDBusObject, "Brightness", 'u'),
    MaxBrightness(&BacklightDBusObject, "MaxBrightness", 'u'),
    Ready(&BacklightDBusObject, "Ready", 'b'),
    BacklightEnabled(&BacklightDBusObject, "BacklightEnabled", 'b') {}
