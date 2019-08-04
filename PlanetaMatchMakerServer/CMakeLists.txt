cmake_minimum_required (VERSION 3.8)

# Sources in the project
include_directories("library")
include_directories("source")
file(GLOB_RECURSE source_files "*.cpp")
add_executable (PlanetaMatchMakerServer ${source_files})

# vckpg
if(Windows)
	if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	else()
	  message(FATAL_ERROR "vcpkg is required.")
	endif()
endif()

# Boost Library
find_package(Boost REQUIRED)
file(GLOB_RECURSE boost_library_paths "${Boost_LIBRARY_DIRS}/*")
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(PlanetaMatchMakerServer ${boost_library_paths})

# Avoid to error of Boost Library 1.70 for MSVC
if (MSVC)
  add_definitions(-D_WIN32_WINNT=0x0601)
endif()

install (TARGETS "PlanetaMatchMakerServer" RUNTIME DESTINATION bin)

# for debug
#message(STATUS "*** dump start cmake variables ***")
#get_cmake_property(_variableNames VARIABLES)
#foreach(_variableName ${_variableNames})
#        message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
#message(STATUS "*** dump end ***")
