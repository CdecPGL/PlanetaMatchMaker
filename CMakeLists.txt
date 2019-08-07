cmake_minimum_required (VERSION 3.8)

project ("PlanetaMatchMaker" CXX)

# C++ configurations
set(CMAKE_CXX_STANDARD 17)
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
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
elseif((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -lpthread -latomic")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # for code compability of MSVC 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdelayed-template-parsing")
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
