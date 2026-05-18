# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "debug")
  file(REMOVE_RECURSE
  "output.map"
  "C:\\Users\\hp\\.vscode\\MoiBox\\program.bin"
  )
endif()
