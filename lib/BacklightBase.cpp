//===- BacklightBase.cpp ----------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Base for backlight frontend and daemon.
//===----------------------------------------------------------------------===//

#include "buzz/DBus/DBusObject.h"

using namespace buzz;

dbus::DBusObject BacklightObject(dbus::DefaultBus::System,
                                 "git.felixnaredi.buzzlight",
                                 "/git/felixnaredi/buzzlight",
                                 "git.felixnaredi.buzzlight");
