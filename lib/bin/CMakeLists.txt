add_executable(dbus-buzzlight "dbus-buzzlight.c")
target_link_libraries(dbus-buzzlight
  systemd
  pthread)

add_executable(buzzlight
  "buzzlight.cpp"
  "${BUZZ_LIB_DIR}/Backlight.cpp"
  "${BUZZ_LIB_DIR}/CommandLineParser.cpp"
  "${BUZZ_LIB_DIR}/DBus.cpp"
  "${BUZZ_LIB_DIR}/OptionMap.cpp")
target_link_libraries(buzzlight
  systemd)
