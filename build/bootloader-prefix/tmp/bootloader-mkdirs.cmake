# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Esp32_521/frameworks/esp-idf-v5.2.1/components/bootloader/subproject"
  "C:/Esp32_521/workspace_521/TTGO_ota/build/bootloader"
  "C:/Esp32_521/workspace_521/TTGO_ota/build/bootloader-prefix"
  "C:/Esp32_521/workspace_521/TTGO_ota/build/bootloader-prefix/tmp"
  "C:/Esp32_521/workspace_521/TTGO_ota/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Esp32_521/workspace_521/TTGO_ota/build/bootloader-prefix/src"
  "C:/Esp32_521/workspace_521/TTGO_ota/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Esp32_521/workspace_521/TTGO_ota/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Esp32_521/workspace_521/TTGO_ota/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
