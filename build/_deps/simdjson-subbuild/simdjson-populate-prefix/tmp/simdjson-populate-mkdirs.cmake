# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/BIT/hft_system/build/_deps/simdjson-src")
  file(MAKE_DIRECTORY "C:/Users/BIT/hft_system/build/_deps/simdjson-src")
endif()
file(MAKE_DIRECTORY
  "C:/Users/BIT/hft_system/build/_deps/simdjson-build"
  "C:/Users/BIT/hft_system/build/_deps/simdjson-subbuild/simdjson-populate-prefix"
  "C:/Users/BIT/hft_system/build/_deps/simdjson-subbuild/simdjson-populate-prefix/tmp"
  "C:/Users/BIT/hft_system/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src/simdjson-populate-stamp"
  "C:/Users/BIT/hft_system/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src"
  "C:/Users/BIT/hft_system/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src/simdjson-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/BIT/hft_system/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src/simdjson-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/BIT/hft_system/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src/simdjson-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
