//===- FileManager.cpp ------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Description goes here
//===----------------------------------------------------------------------===//

#include "FileManager.h"
#include <cstring>
#include <cstdint>
#include <cassert>
#include <string>
#include <fcntl.h>
#include <unistd.h>

using namespace buzz;

const char *FileManager::BrightnessPath =
    "/sys/class/backlight/intel_backlight/brightness";
const char *FileManager::MaxBrightnessPath =
    "/sys/class/backlight/intel_backlight/max_brightness";

std::int32_t FileManager::readValue(const char *Path) {
  int FD = open(Path, O_RDONLY);
  assert(FD > -1);
  char Buf[32];
  read(FD, Buf, sizeof(Buf));
  close(FD);

  std::size_t Len;
  int Val = std::stoi(Buf, &Len);
  assert(Len == std::strlen(static_cast<char *>(Buf)) || Val > -1);
  return Val;
}

void FileManager::writeValue(const char *Path, std::int32_t Value) {
  int FD = open(Path, O_WRONLY);
  assert(FD > -1);
  ftruncate(FD, 0);

  char Buf[32];
  snprintf(Buf, sizeof(Buf), "%d\n", Value);
  int N = write(FD, Buf, std::strlen(static_cast<char *>(Buf)));
  close(FD);
  assert(N > 0);
}
