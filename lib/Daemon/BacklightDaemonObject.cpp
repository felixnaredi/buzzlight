//===- BacklightDaemonObject.cpp --------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Base object implementation.
//===----------------------------------------------------------------------===//

#include "FileManager.h"
#include "buzz/DBus/DBusObject.h"
#include "buzz/BacklightDaemon.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <systemd/sd-bus.h>

using namespace buzz;

extern dbus::DBusObject BacklightObject;

struct Singleton {
  static BacklightDaemon *Daemon;

  static std::int32_t propertyGetMaxBrightness() {
    assert(Daemon);
    return Daemon->getMaxBrightness();
  }

  static std::int32_t propertyGetBrightness() {
    assert(Daemon);
    return Daemon->getBrightness();
  }

  static void propertySetBrightness(std::int32_t Value) {
    assert(Daemon);
    return Daemon->setBrightness(Value);
  }

  static std::int32_t propertyGetStoredBrightness() {
    assert(Daemon);
    return Daemon->getStoredBrightness();
  }

  static int propertyGetIsReady() {
    assert(Daemon);
    return Daemon->getIsReady();
  }

  static int propertyGetIsEnabled() {
    assert(Daemon);
    return Daemon->getIsEnabled();
  }

  static int methodToggleBacklight(std::uint32_t MilliSec) {
    assert(Daemon);
    return Daemon->toggleBacklight(MilliSec);
  }

  static void methodSetBrightnessSmooth(std::int32_t Value,
                                       std::uint32_t MilliSec) {
    assert(Daemon);
    Daemon->setBrightnessSmooth(Value, MilliSec);
  }
};

BacklightDaemon *Singleton::Daemon = nullptr;

struct Property {

#define BUZZ_PROPERTY_BASIC_GETTER(Name, Signature, Func)               \
  static int Name(sd_bus *Bus,                                          \
                  const char *Path,                                     \
                  const char *Interface,                                \
                  const char *Property,                                 \
                  sd_bus_message *Reply,                                \
                  void *UserData,                                       \
                  sd_bus_error *Error) {                                \
    assert(Bus);                                                        \
    assert(Reply);                                                      \
    auto Value = Func();                                                \
    return sd_bus_message_append_basic(Reply, Signature, &Value);       \
  }

#define BUZZ_PROPERTY_BASIC_SETTER(Name, Signature, Func)               \
  static int Name(sd_bus *Bus,                                          \
                  const char *Path,                                     \
                  const char *Interface,                                \
                  const char *Property,                                 \
                  sd_bus_message *Value,                                \
                  void *UserData,                                       \
                  sd_bus_error *Error) {                                \
    assert(Bus);                                                        \
    assert(Value);                                                      \
    std::size_t Ret;                                                    \
    int R = sd_bus_message_read_basic(Value, Signature, &Ret);          \
    if(R < 0)                                                           \
      return R;                                                         \
    Func(Ret);                                                          \
    return 0;                                                           \
  }

  BUZZ_PROPERTY_BASIC_GETTER(getMaxBrightness,
                             'i',
                             Singleton::propertyGetMaxBrightness);
  BUZZ_PROPERTY_BASIC_GETTER(getBrightness,
                             'i',
                             Singleton::propertyGetBrightness);
  BUZZ_PROPERTY_BASIC_SETTER(setBrightness,
                             'i',
                             Singleton::propertySetBrightness);
  BUZZ_PROPERTY_BASIC_GETTER(getStoredBrightness,
                             'i',
                             Singleton::propertyGetStoredBrightness);
  BUZZ_PROPERTY_BASIC_GETTER(getIsReady,
                             'b',
                             Singleton::propertyGetIsReady);
  BUZZ_PROPERTY_BASIC_GETTER(getIsEnabled,
                             'b',
                             Singleton::propertyGetIsEnabled);

#undef BUZZ_PROPERTY_BASIC_SETTER
#undef BUZZ_PROPERTY_BASIC_GETTER
};

struct Method {

  static int toggleBacklight(sd_bus_message *Message,
                             void *UserData,
                             sd_bus_error *Error) {
    assert(Message);

    std::uint32_t MilliSec;
    int R = sd_bus_message_read_basic(Message, 'u', &MilliSec);
    if(R < 0)
      throw std::runtime_error(strerror(-R));

    int Ret = Singleton::methodToggleBacklight(MilliSec);
    return sd_bus_reply_method_return(Message, "b", Ret);
  }

  static int setBrightnessSmooth(sd_bus_message *Message,
                                 void *UserData,
                                 sd_bus_error *Error) {
    assert(Message);

    std::int32_t Value;
    std::uint32_t MilliSec;
    int R = sd_bus_message_read(Message, "uu", &Value, &MilliSec);
    if(R < 0)
      throw std::runtime_error(strerror(-R));

    Singleton::methodSetBrightnessSmooth(Value, MilliSec);
    return sd_bus_reply_method_return(Message, "");
  }

};

std::unique_ptr<sd_bus_vtable[]> BacklightDaemon::spawnVirtualTable() {
  sd_bus_vtable Table[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_PROPERTY("MaxBrightness",
                    "i",
                    Property::getMaxBrightness,
                    0,
                    SD_BUS_VTABLE_PROPERTY_CONST),
    SD_BUS_WRITABLE_PROPERTY("Brightness",
                             "i",
                             Property::getBrightness,
                             Property::setBrightness,
                             0,
                             SD_BUS_VTABLE_UNPRIVILEGED),
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
    SD_BUS_METHOD("ToggleBacklight",
                  "u",
                  "b",
                  Method::toggleBacklight,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("SetBrightnessSmooth",
                  "uu",
                  "",
                  Method::setBrightnessSmooth,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
  };
  std::unique_ptr<sd_bus_vtable[]> Ptr = std::unique_ptr<sd_bus_vtable[]>(
      static_cast<sd_bus_vtable*>(std::malloc(sizeof(Table))));
  assert(Ptr);
  std::memcpy(Ptr.get(), Table, sizeof(Table));

  return Ptr;
}

BacklightDaemon::BacklightDaemon()
    : dbus::DBusObject(&BacklightObject),
      MaxBrightness(FileManager::readMaxBrightnessValue()),
      StoredBrightness(FileManager::readBrightnessValue()),
      Ready(true) {

  Singleton::Daemon = this;
}
