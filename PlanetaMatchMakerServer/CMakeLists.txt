cmake_minimum_required (VERSION 3.8)

# Sources in this project
set(main_source "source/main.cpp")
file(GLOB_RECURSE source_files "*.cpp")
list(REMOVE_ITEM source_files ${main_source})
add_library (PlanetaMatchMakerServerLib ${source_files})

# to avoid linker error "undefined reference to `std::filesystem::__cxx11::**"
# http://nanapro.org/en-us/forum/index.php?u=/topic/266/ggcompiling-with-std-filesystem
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_link_libraries(PlanetaMatchMakerServerLib stdc++fs)
endif()

# Headers in this project
set (includes "library" "source")
target_include_directories(PlanetaMatchMakerServerLib PUBLIC ${includes})

# Boost Library
find_package(Boost REQUIRED)
file(GLOB_RECURSE boost_library_paths "${Boost_LIBRARY_DIRS}/*boost*")
# to avoid warnings, use SYSTEM (-isystem option of gcc)
target_include_directories(PlanetaMatchMakerServerLib SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(PlanetaMatchMakerServerLib ${boost_library_paths})

# Avoid to error of Boost Library 1.70 for MSVC
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_definitions(-D_WIN32_WINNT=0x0601)
endif()

# Main executable
add_executable (PlanetaMatchMakerServer ${main_source})
target_link_libraries(PlanetaMatchMakerServer PlanetaMatchMakerServerLib)
# Install bunary
install (TARGETS PlanetaMatchMakerServer DESTINATION bin)
# Create symbolic link to binary
install_symlink ("${CMAKE_INSTALL_PREFIX}/bin/PlanetaMatchMakerServer" "${CMAKE_INSTALL_PREFIX}/bin/pmms")

# Copy setting file
if((CMAKE_SYSTEM_NAME STREQUAL "Windows") OR (CMAKE_SYSTEM_NAME STREQUAL "MSYS"))
	install (FILES "setting.json" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ DESTINATION "$ENV{HOMEDRIVE}$ENV{HOMEPATH}/.pmms")
else()
	install (FILES "setting.json" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ DESTINATION "$ENV{HOME}/.pmms")
endif()

# for debug
#message(STATUS "*** dump start cmake variables ***")
#get_cmake_property(_variableNames VARIABLES)
#foreach(_variableName ${_variableNames})
#        message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
#message(STATUS "*** dump end ***")
