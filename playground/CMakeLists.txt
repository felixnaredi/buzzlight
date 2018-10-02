
add_executable(CMP
  "CMP.cpp"
  "../lib/CommandLineParser.cpp"
  "../lib/OptionMap.cpp")

add_executable(DBusGetter
  "DBusGetter.cpp"
  "../lib/DBus.cpp"
  "../lib/Backlight.cpp")

target_link_libraries(DBusGetter
  systemd)
