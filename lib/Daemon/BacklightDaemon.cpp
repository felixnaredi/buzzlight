//===- BacklightDaemon.cpp --------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Implement Backlight daemon.
//===----------------------------------------------------------------------===//

#include "FileManager.h"
#include "buzz/Daemon/BacklightDaemon.h"
#include "buzz/DBus/Object.h"
#include <cstdint>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <string>
#include <stdexcept>
#include <systemd/sd-bus.h>

using namespace buzz;

static dbus::Object BaseObject(dbus::DefaultBus::System,
                               "git.felixnaredi.buzzlight",
                               "/git/felixnaredi/buzzlight",
                               "git.felixnaredi.buzzlight");

std::int32_t BacklightDaemon::getBrightness() const {
  try {
    return FileManager::readBrightnessValue();
  }
  catch (std::exception &Exception) {}
  return -1;
}

void BacklightDaemon::run() {
  sd_bus *Bus = DBusObject.Bus;
  assert(Bus);
  while(1) {
    int R = sd_bus_process(Bus, NULL);
    if(R < 0)
      throw std::runtime_error(strerror(-R));
    if(R > 0)
      continue;
    R = sd_bus_wait(Bus, (uint64_t) -1);
    if(R < 0)
      throw std::runtime_error(strerror(-R));
  }
}

struct Singleton {
  static BacklightDaemon *Daemon;

  static std::int32_t getMaxBrightness() {
    assert(Daemon);
    return Daemon->getMaxBrightness();
  }

  static std::int32_t getBrightness() {
    assert(Daemon);
    return Daemon->getBrightness();
  }

  static void setBrightness(std::int32_t Value) {
    assert(Daemon);
    return Daemon->setBrightness(Value);
  }

  static std::int32_t getStoredBrightness() {
    assert(Daemon);
    return Daemon->getStoredBrightness();
  }

  static int getIsReady() {
    assert(Daemon);
    return Daemon->getIsReady();
  }

  static int getIsEnabled() {
    assert(Daemon);
    return Daemon->getIsEnabled();
  }
};

BacklightDaemon *Singleton::Daemon = nullptr;

struct Property {
#define BUZZ_PROPERTY_GETTER(Name, Signature, Func)             \
  static int Name(sd_bus *Bus,                                  \
                  const char *Path,                             \
                  const char *Interface,                        \
                  const char *Property,                         \
                  sd_bus_message *Reply,                        \
                  void *UserData,                               \
                  sd_bus_error *Error) {                        \
    assert(Bus);                                                \
    assert(Reply);                                              \
    return sd_bus_message_append(Reply, Signature, Func());     \
  }

  BUZZ_PROPERTY_GETTER(getMaxBrightness,
                       "i",
                       Singleton::getMaxBrightness);
  BUZZ_PROPERTY_GETTER(getBrightness,
                       "i",
                       Singleton::getBrightness);
  BUZZ_PROPERTY_GETTER(getStoredBrightness,
                       "i",
                       Singleton::getStoredBrightness);
  BUZZ_PROPERTY_GETTER(getIsReady,
                       "b",
                       Singleton::getIsReady);
  BUZZ_PROPERTY_GETTER(getIsEnabled,
                       "b",
                       Singleton::getIsEnabled);

#undef BUZZ_PROEPRTY_GETTER
};

BacklightDaemon::BacklightDaemon()
    : DBusObject(BaseObject),
      MaxBrightness(FileManager::readMaxBrightnessValue()),
      StoredBrightness(FileManager::readBrightnessValue()),
      Ready(true) {

  Enabled = StoredBrightness > 0;

  sd_bus_vtable Temp[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_PROPERTY("MaxBrightness",
                    "i",
                    Property::getMaxBrightness,
                    0,
                    SD_BUS_VTABLE_PROPERTY_CONST),
    SD_BUS_PROPERTY("Brightness",
                    "i",
                    Property::getBrightness,
                    0,
                    0),
    SD_BUS_PROPERTY("StoredBrightness",
                    "i",
                    Property::getStoredBrightness,
                    0,
                    0),
    SD_BUS_PROPERTY("IsReady",
                    "b",
                    Property::getIsReady,
                    0,
                    0),
    SD_BUS_PROPERTY("IsEnabled",
                    "b",
                    Property::getIsEnabled,
                    0,
                    0),
    SD_BUS_VTABLE_END,
  };
  VirtualTable = static_cast<sd_bus_vtable *>(std::malloc(sizeof(Temp)));
  assert(VirtualTable);
  std::memcpy(VirtualTable, Temp, sizeof(Temp));

  sd_bus_slot *Slot;
  int R = sd_bus_add_object_vtable(DBusObject.Bus,
                                   &Slot,
                                   DBusObject.Path,
                                   DBusObject.Destination,
                                   VirtualTable,
                                   NULL);
  if(R < 0)
    throw std::runtime_error(strerror(-R));

  R = sd_bus_request_name(DBusObject.Bus, DBusObject.Interface, 0);
  if(R < 0)
    throw std::runtime_error(strerror(-R));

  Singleton::Daemon = this;
}
