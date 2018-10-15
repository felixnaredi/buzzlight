add_executable(buzzlight-daemon
  "main.cpp"
  "BacklightDaemon.cpp"
  "BacklightDaemonObject.cpp"
  "FileManager.cpp"
  "${BUZZ_LIB_DIR}/BacklightBase.cpp"
  "${BUZZ_LIB_DIR}/DBus/DBusObject.cpp")

target_link_libraries(buzzlight-daemon
  pthread
  systemd)
