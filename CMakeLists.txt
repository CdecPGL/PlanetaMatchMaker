﻿cmake_minimum_required (VERSION 3.21.3)
enable_testing()

project ("PlanetaMatchMaker" CXX)

# Force to build only static library
set(BUILD_SHARED_LIBS OFF)
# Link dependencies statically by default
# If you want to link dependencies with shared libraries, add "-DSTATIC_LINK_DEPENDENCIES=OFF" as cmake option.
option (STATIC_LINK_DEPENDENCIES "Link dependencies statically." ON)

# Set build type
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Macros
macro(install_symlink filepath sympath)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${filepath} ${sympath})")
    install(CODE "message(\"-- Created symlink: ${sympath} -> ${filepath}\")")
endmacro(install_symlink)

# C++ configurations
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler configurations
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:utf-8")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
elseif((CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
  message(FATAL_ERROR "gcc is not supported. Add \"-DCMAKE_CXX_COMPILER=clang++\" as cmake option.")
elseif((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -fpermissive")

  if (STATIC_LINK_DEPENDENCIES)
  	# to avoid linker error "undefined reference to `__atomic_load'"
    find_library(Atomic "libatomic.a" HINTS /usr/lib/gcc/x86_64-linux-gnu/12/ REQUIRED)
    link_libraries(${Atomic})
  else()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -latomic")
  endif()
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
else()
  message(FATAL_ERROR "Not supported compiler. (${CMAKE_CXX_COMPILER_ID})")
endif()

# vckpg
if(Windows)
  if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
  else()
    message(FATAL_ERROR "vcpkg is required.")
  endif()
endif()

# Subprojects
add_subdirectory ("PlanetaMatchMakerServer")
add_subdirectory ("PlanetaMatchMakerServerTest")
