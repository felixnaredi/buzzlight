//===- DBusGetter.cpp -------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Description goes here
//===----------------------------------------------------------------------===//

#include "buzz/Backlight.h"
#include <iostream>

using namespace buzz;

int main(int argc, char **argv)
{
  Backlight Bl;
  std::cout << std::boolalpha
            << "Brightness: " << Bl.Brightness.get() << ", "
            << "MaxBrightness: " << Bl.MaxBrightness.get() << ", "
            << "Ready: " << static_cast<bool>(Bl.Ready.get()) << ", "
            << "BacklightEnables: " << static_cast<bool>(
                Bl.BacklightEnabled.get())
            << "\n";
  Bl.Brightness.set(35000);
  return 0;
}
