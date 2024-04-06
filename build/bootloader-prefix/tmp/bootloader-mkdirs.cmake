# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/Esp32/frameworks/esp-idf-v5.2.1/components/bootloader/subproject"
  "D:/Esp32/workspace521/TTGO_OTA/build/bootloader"
  "D:/Esp32/workspace521/TTGO_OTA/build/bootloader-prefix"
  "D:/Esp32/workspace521/TTGO_OTA/build/bootloader-prefix/tmp"
  "D:/Esp32/workspace521/TTGO_OTA/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Esp32/workspace521/TTGO_OTA/build/bootloader-prefix/src"
  "D:/Esp32/workspace521/TTGO_OTA/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Esp32/workspace521/TTGO_OTA/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Esp32/workspace521/TTGO_OTA/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
