add_executable(daemon-buzzlight
  "${BUZZ_LIB_DIR}/Daemon/main.cpp"
  "${BUZZ_LIB_DIR}/Daemon/BacklightDaemon.cpp"
  "${BUZZ_LIB_DIR}/Daemon/BacklightDaemonObject.cpp"
  "${BUZZ_LIB_DIR}/Daemon/FileManager.cpp"
  "${BUZZ_LIB_DIR}/DBus/Object.cpp")

target_link_libraries(daemon-buzzlight
  pthread
  systemd)
