cmake_minimum_required (VERSION 3.8)

# Sources in this project
set(main_source "source/main/main.cpp")
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
find_package(Boost 1.70.0 REQUIRED COMPONENTS thread coroutine date_time filesystem log)
target_include_directories(PlanetaMatchMakerServerLib SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(PlanetaMatchMakerServerLib ${Boost_LIBRARIES})

# Avoid to error of Boost Library 1.70 for MSVC
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_definitions(-D_WIN32_WINNT=0x0601)
endif()

# Main executable
add_executable (PlanetaMatchMakerServer ${main_source})
target_link_libraries(PlanetaMatchMakerServer PlanetaMatchMakerServerLib)
# Install binary
install (TARGETS PlanetaMatchMakerServer DESTINATION bin)
# Create symbolic link to binary
install_symlink ("${CMAKE_INSTALL_PREFIX}/bin/PlanetaMatchMakerServer" "${CMAKE_INSTALL_PREFIX}/bin/pmms")

# Copy setting file
if((CMAKE_SYSTEM_NAME STREQUAL "Windows") OR (CMAKE_SYSTEM_NAME STREQUAL "MSYS"))
	install (FILES "setting.json" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ DESTINATION "$ENV{HOMEDRIVE}$ENV{HOMEPATH}/.pmms")
else()
	install (FILES "setting.json" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ DESTINATION "$ENV{HOME}/.pmms")
endif()
