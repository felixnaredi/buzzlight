//===- Frontend.cpp ---------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Description goes here
//===----------------------------------------------------------------------===//

#include "buzz/Backlight.h"
#include <thread>
#include <chrono>
#include <iostream>

using namespace buzz;

void printProperties(const Backlight &Bl) {
  std::cout << std::boolalpha
            << "MaxBrightness: " << Bl.MaxBrightness.get() << ", "
            << "Brightness: " << Bl.Brightness.get() << ", "
            << "IsEnabled: " << Bl.IsEnabled.get() << ", "
            << "IsReady: " << Bl.IsReady.get() << "\n";
}

int main(int argc, char **argv) {
  Backlight Bl;
  printProperties(Bl);

  Bl.setBrightnessSmooth(50000, 1000);
  printProperties(Bl);

  Bl.toggleBacklight(500);
  printProperties(Bl);
  Bl.toggleBacklight(500);
  printProperties(Bl);

  Bl.Brightness.set(500000);
  printProperties(Bl);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  Bl.Brightness.set(-5);
  printProperties(Bl);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  Bl.setBrightnessSmooth(20000, 1000);
  printProperties(Bl);

  return 0;
}
