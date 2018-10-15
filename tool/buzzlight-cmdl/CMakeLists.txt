add_executable(
  buzzlight-cmdl
  main.cpp
  CommandLineParser.cpp
  "${BUZZ_LIB_DIR}/BacklightBase.cpp"
  "${BUZZ_LIB_DIR}/Backlight.cpp"
  "${BUZZ_LIB_DIR}/DBus/DBusObject.cpp"
  "${BUZZ_LIB_DIR}/DBus/Property.cpp")

target_link_libraries(
  buzzlight-cmdl
  systemd
  pthread)
