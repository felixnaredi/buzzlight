//===- Property.h -----------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Property getters and setters.
//===----------------------------------------------------------------------===//

#ifndef BUZZ_DBUS_PROPERTY_H
#define BUZZ_DBUS_PROPERTY_H

#include "buzz/DBus/DBusObject.h"

namespace buzz {
namespace dbus {

template <typename T>
class Property {
protected:
  DBusObject *Object;
  const char *Member;

public:
  Property(DBusObject *object, const char *member)
      : Object(object), Member(member) {}

  T get() const;
};

template <typename T>
class WritableProperty : public Property<T> {
public:
  WritableProperty(DBusObject *object, const char *member)
      : Property<T>(object, member) {}

  void set(const T &Value);
};

} // dbus
} // buzz

#endif // BUZZ_DBUS_PROPERTY_H
