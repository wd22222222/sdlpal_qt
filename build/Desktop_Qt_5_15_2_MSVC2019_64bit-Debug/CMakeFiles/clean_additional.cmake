# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\main_lib_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\main_lib_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\s_lib_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\s_lib_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\sdlpal_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\sdlpal_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\sdlpal_qt_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\sdlpal_qt_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\sound_lib_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\sound_lib_autogen.dir\\ParseCache.txt"
  "main_lib_autogen"
  "s_lib_autogen"
  "sdlpal_autogen"
  "sdlpal_qt_autogen"
  "sound_lib_autogen"
  )
endif()
