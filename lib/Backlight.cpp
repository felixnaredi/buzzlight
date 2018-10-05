//===- Backlight.cpp --------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Implements backlight.
//===----------------------------------------------------------------------===//

#include "buzz/Backlight.h"
#include "buzz/DBus.h"
#include <cstdint>
#include <stdexcept>
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
    Brightness(&BacklightDBusObject, "Brightness", 'i'),
    MaxBrightness(&BacklightDBusObject, "MaxBrightness", 'u'),
    Ready(&BacklightDBusObject, "Ready", 'b'),
    BacklightEnabled(&BacklightDBusObject, "BacklightEnabled", 'b') {}

void Backlight::setBrightnessSmooth(std::int32_t Value,
                                    std::uint32_t US,
                                    bool Store) {

  if(!Ready.get())
    return;
  dbus::Object *Obj = &Backlight::BacklightDBusObject;
  sd_bus_message *Reply;
  int R = sd_bus_call_method(Obj->Bus,
                             Obj->Destination,
                             Obj->Path,
                             Obj->Interface,
                             "SetBrightnessSmooth",
                             NULL,
                             &Reply,
                             "iub",
                             Value,
                             US,
                             Store);
  if(R < 0)
    throw std::runtime_error(strerror(-R));
}
