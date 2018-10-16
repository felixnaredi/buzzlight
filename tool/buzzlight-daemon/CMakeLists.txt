add_executable(
  buzzlight-daemon
  "main.cpp"
  "BacklightDaemon.cpp"
  "BacklightDaemonObject.cpp"
  "FileManager.cpp"
  "${BUZZ_LIBRARY_DIR}/BacklightBase.cpp"
  "${BUZZ_LIBRARY_DIR}/DBus/DBusObject.cpp")

target_link_libraries(
  buzzlight-daemon
  pthread
  systemd)

install(TARGETS buzzlight-daemon
  RUNTIME DESTINATION bin)

install(FILES git.felixnaredi.buzzlight.conf
  DESTINATION /etc/dbus-1/system.d)
