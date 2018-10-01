add_executable(dbus-buzzlight "DBus/buzzlight.c")
target_link_libraries(dbus-buzzlight
  systemd
  pthread)
