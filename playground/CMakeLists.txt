add_executable(Frontend
  "Frontend.cpp"
  "${BUZZ_LIB_DIR}/DBus/DBusObject.cpp"
  "${BUZZ_LIB_DIR}/DBus/Property.cpp"
  "${BUZZ_LIB_DIR}/BacklightBase.cpp"
  "${BUZZ_LIB_DIR}/Backlight.cpp")

target_link_libraries(Frontend
  pthread
  systemd)
