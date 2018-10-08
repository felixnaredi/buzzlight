//===- Property.cpp ---------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Implements Property
//===----------------------------------------------------------------------===//

#include "buzz/DBus/Property.h"
#include "buzz/DBus/DBusObject.h"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <systemd/sd-bus.h>

using namespace buzz;
using namespace dbus;

template <typename T>
static T getTrivial(const DBusObject *Obj, const char *Member, char Sig) {
  std::uint64_t V = 0;
  int R = sd_bus_get_property_trivial(Obj->Bus,
                                      Obj->Destination,
                                      Obj->Path,
                                      Obj->Interface,
                                      Member,
                                      NULL,
                                      Sig,
                                      &V);
  if(R < 0)
    throw std::runtime_error(strerror(-R));
  return T(V);
}

#define BUZZ_TRIVIAL_GETTER(T, Sig)              \
  template <>                                    \
  T Property<T>::get() const {                   \
    return getTrivial<T>(Object, Member, Sig);   \
  }

BUZZ_TRIVIAL_GETTER(std::uint8_t, 'y')
BUZZ_TRIVIAL_GETTER(bool, 'b')
BUZZ_TRIVIAL_GETTER(std::int16_t, 'n')
BUZZ_TRIVIAL_GETTER(std::uint16_t, 'q')
BUZZ_TRIVIAL_GETTER(std::int32_t, 'i')
BUZZ_TRIVIAL_GETTER(std::uint32_t, 'u')
BUZZ_TRIVIAL_GETTER(std::int64_t, 'x')
BUZZ_TRIVIAL_GETTER(std::uint64_t, 't')
BUZZ_TRIVIAL_GETTER(double, 'd')

#undef BUZZ_TRIVIAL_GETTER

template <typename T>
static void setTrivial(const DBusObject *Obj,
                       const char *Member,
                       char Sig,
                       T Value) {
  char Buffer[2] = {0, 0};
  Buffer[0] = Sig;
  int R = sd_bus_set_property(Obj->Bus,
                              Obj->Destination,
                              Obj->Path,
                              Obj->Interface,
                              Member,
                              NULL,
                              Buffer,
                              Value);
  if(R < 0)
    throw std::runtime_error(strerror(-R));
}

#define BUZZ_TRIVIAL_SETTER(T, Sig)           \
  template <>                                 \
  void WritableProperty<T>::set(const T &V) { \
    setTrivial(Object, Member, Sig, V);       \
  }

BUZZ_TRIVIAL_SETTER(std::uint8_t, 'y')
BUZZ_TRIVIAL_SETTER(bool, 'b')
BUZZ_TRIVIAL_SETTER(std::int16_t, 'n')
BUZZ_TRIVIAL_SETTER(std::uint16_t, 'q')
BUZZ_TRIVIAL_SETTER(std::int32_t, 'i')
BUZZ_TRIVIAL_SETTER(std::uint32_t, 'u')
BUZZ_TRIVIAL_SETTER(std::int64_t, 'x')
BUZZ_TRIVIAL_SETTER(std::uint64_t, 't')
BUZZ_TRIVIAL_SETTER(double, 'd')

#undef BUZZ_TRIVIAL_SETTER
