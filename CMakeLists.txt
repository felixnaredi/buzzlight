cmake_minimum_required(VERSION 3.0)

add_executable(buzzlight-dbus "buzzlight-dbus.c")
target_link_libraries(buzzlight-dbus
  systemd)

add_executable(buzzlight "buzzlight.c")
target_link_libraries(buzzlight
  systemd)
