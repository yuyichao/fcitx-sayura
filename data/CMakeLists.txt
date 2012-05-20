fcitx_add_addon_conf_file(fcitx-sayura.conf)
fcitx_add_inputmethod_conf_file(sayura.conf)
install(FILES sayura.png DESTINATION share/fcitx/skin/default/)
install(FILES sayura.png DESTINATION share/icons/hicolor/32x32/apps/
  RENAME fcitx-sayura.png)
