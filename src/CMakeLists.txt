include_directories(${FCITX4_FCITX_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_CONFIG_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_UTILS_INCLUDE_DIRS})
include_directories(${PROJECT_BINARY_DIR})

set(fcitx_sayura_sources
  eim.c
  )

fcitx_add_addon(fcitx-sayura ${fcitx_sayura_sources})
