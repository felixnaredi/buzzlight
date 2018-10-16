add_executable(
  buzzlight-cmdl
  main.cpp
  CommandLineParser.cpp
  "${BUZZ_LIBRARY_DIR}/BacklightBase.cpp"
  "${BUZZ_LIBRARY_DIR}/Backlight.cpp"
  "${BUZZ_LIBRARY_DIR}/DBus/DBusObject.cpp"
  "${BUZZ_LIBRARY_DIR}/DBus/Property.cpp")

target_link_libraries(
  buzzlight-cmdl
  systemd
  pthread)

install(TARGETS buzzlight-cmdl
  RUNTIME DESTINATION bin)
