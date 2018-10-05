//===- FileManager.h --------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Handles the specific files that controlls the backlight.
//===----------------------------------------------------------------------===//

#ifndef BUZZ_DAEMON_FILEMANAGER_H
#define BUZZ_DAEMON_FILEMANAGER_H

#include <cstdint>

namespace buzz {

class FileManager {
  static const char *BrightnessPath;
  static const char *MaxBrightnessPath;

  static std::int32_t readValue(const char *Path);
  static void writeValue(const char *Path, std::int32_t Value);

public:
  static std::int32_t readBrightnessValue() {
    return readValue(BrightnessPath);
  }

  static void writeBrightnessValue(std::int32_t Value) {
    writeValue(BrightnessPath, Value);
  }

  static std::int32_t readMaxBrightnessValue() {
    return readValue(MaxBrightnessPath);
  }
};

}

#endif // BUZZ_DAEMON_FILEMANAGER_H
