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
#include <thread>
#include <chrono>
#include <systemd/sd-bus.h>

#include <iostream>

using namespace buzz;

template <typename T>
static T min(T a, T b) {
  return a < b ? a : b;
}

template <typename T>
static T max(T a, T b) {
  return a > b ? a : b;
}

std::int32_t BacklightDaemon::getBrightness() const {
  try {
    return FileManager::readBrightnessValue();
  }
  catch (std::exception &Exception) {}
  return -1;
}

void BacklightDaemon::setBrightness(std::int32_t Value) {
  int V = min(max(0, Value), MaxBrightness);
  try {
    FileManager::writeBrightnessValue(V);
  }
  catch (std::exception &Exception) {}
}

bool BacklightDaemon::getIsEnabled() const {
  return getBrightness() != 0;
}

struct Frame {
  using Duration = std::chrono::duration<std::uint32_t, std::ratio<1, 60>>;

  template <typename Dur>
  static int count(const Dur &D) {
    return std::chrono::duration_cast<Duration>(D).count();
  }
};

static void smoothSet(std::int32_t From,
                      std::int32_t To,
                      std::uint32_t MilliSec) {
  int Frames = Frame::count(std::chrono::milliseconds(MilliSec));
  auto Thread = std::thread(
      [From, To, Frames]{
        float Delta = static_cast<float>(To - From) /
                          static_cast<float>(Frames);
        for(int I = 0; I < Frames; I++) {
          FileManager::writeBrightnessValue(
              From + static_cast<std::int32_t>(Delta * I));
          std::this_thread::sleep_for(Frame::Duration(1));
        }
        FileManager::writeBrightnessValue(To);
      });
  Thread.join();
}

bool BacklightDaemon::toggleBacklight(std::uint32_t MilliSec) const {
  bool Enabled = getIsEnabled();
  std::int32_t V = Enabled ? 0 : StoredBrightness;
  try {
    smoothSet(getBrightness(), V, MilliSec);
  }
  catch (std::exception &Exception) {}
  return !Enabled;
}
