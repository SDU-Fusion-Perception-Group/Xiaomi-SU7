# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/b0weny/robot_px4_core/build/_deps/rplidar_sdk-src"
  "/home/b0weny/robot_px4_core/build/_deps/rplidar_sdk-build"
  "/home/b0weny/robot_px4_core/build/_deps/rplidar_sdk-subbuild/rplidar_sdk-populate-prefix"
  "/home/b0weny/robot_px4_core/build/_deps/rplidar_sdk-subbuild/rplidar_sdk-populate-prefix/tmp"
  "/home/b0weny/robot_px4_core/build/_deps/rplidar_sdk-subbuild/rplidar_sdk-populate-prefix/src/rplidar_sdk-populate-stamp"
  "/home/b0weny/robot_px4_core/build/_deps/rplidar_sdk-subbuild/rplidar_sdk-populate-prefix/src"
  "/home/b0weny/robot_px4_core/build/_deps/rplidar_sdk-subbuild/rplidar_sdk-populate-prefix/src/rplidar_sdk-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/b0weny/robot_px4_core/build/_deps/rplidar_sdk-subbuild/rplidar_sdk-populate-prefix/src/rplidar_sdk-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/b0weny/robot_px4_core/build/_deps/rplidar_sdk-subbuild/rplidar_sdk-populate-prefix/src/rplidar_sdk-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
