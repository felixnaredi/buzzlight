//===- Backlight.cpp --------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Implements Backlight.h.
//===----------------------------------------------------------------------===//

#include "buzz/Backlight.h"
#include "buzz/DBus/DBusObject.h"
#include "buzz/DBus/Property.h"
#include <cstdint>
#include <systemd/sd-bus.h>

using namespace buzz;

extern dbus::DBusObject BacklightObject;

Backlight::Backlight()
    : Object(&BacklightObject),
      Brightness(&BacklightObject, "Brightness"),
      MaxBrightness(&BacklightObject, "MaxBrightness"),
      IsEnabled(&BacklightObject, "IsEnabled"),
      IsReady(&BacklightObject, "IsReady") {}

void Backlight::toggleBacklight(std::uint32_t MilliSec) {
  sd_bus_message *Reply;
  int R = sd_bus_call_method(Object->Bus,
                             Object->Destination,
                             Object->Path,
                             Object->Interface,
                             "ToggleBacklight",
                             NULL,
                             &Reply,
                             "u",
                             MilliSec);
  if(R < 0)
    throw std::runtime_error(strerror(-R));
}

void Backlight::setBrightnessSmooth(std::int32_t Value,
                                    std::uint32_t MilliSec) {
  sd_bus_message *Reply;
  int R = sd_bus_call_method(Object->Bus,
                             Object->Destination,
                             Object->Path,
                             Object->Interface,
                             "SetBrightnessSmooth",
                             NULL,
                             &Reply,
                             "uu",
                             Value,
                             MilliSec);
  if(R < 0)
    throw std::runtime_error(strerror(-R));
}
