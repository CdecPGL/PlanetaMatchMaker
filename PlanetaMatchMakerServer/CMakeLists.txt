cmake_minimum_required (VERSION 3.21.3)

# Sources in this project
set(main_source "source/main/main.cpp")
file(GLOB_RECURSE source_files "*.cpp")
list(REMOVE_ITEM source_files ${main_source})
add_library (PlanetaMatchMakerServerLib ${source_files})

if (STATIC_LINK_DEPENDENCIES)
	if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		message(FATAL_ERROR "Static link dependencies is only supported for GNU and Clang")
	endif()

	# Link libraries statically
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
	# Link executables statically
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
	# Link boost libraries statically
	set(Boost_USE_STATIC_LIBS ON)
else()
	# to avoid linker error "undefined reference to `std::filesystem::__cxx11::**"
	# http://nanapro.org/en-us/forum/index.php?u=/topic/266/ggcompiling-with-std-filesystem
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_link_libraries(PlanetaMatchMakerServerLib stdc++fs)
	endif()
endif()

# Headers in this project
set (includes "library" "source")
target_include_directories(PlanetaMatchMakerServerLib PUBLIC ${includes})

# Boost Library
find_package(Boost 1.77.0 REQUIRED COMPONENTS thread coroutine date_time filesystem log json)
target_include_directories(PlanetaMatchMakerServerLib SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(PlanetaMatchMakerServerLib ${Boost_LIBRARIES})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# -D_WIN32_WINNT=0x0601: To avoid error of Boost Library 1.77 for MSVC
	# _CRT_SECURE_NO_WARNINGS: To avoid error for getenv
  add_definitions(-D_WIN32_WINNT=0x0601 _CRT_SECURE_NO_WARNINGS)
elseif((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
endif()

# Main executable
add_executable (PlanetaMatchMakerServer ${main_source})
target_link_libraries(PlanetaMatchMakerServer PlanetaMatchMakerServerLib -static)
# Install binary
install (TARGETS PlanetaMatchMakerServer DESTINATION bin)
# Create symbolic link to binary
install_symlink ("${CMAKE_INSTALL_PREFIX}/bin/PlanetaMatchMakerServer" "${CMAKE_INSTALL_PREFIX}/bin/pmms")

# Copy setting file
if((CMAKE_SYSTEM_NAME STREQUAL "Windows") OR (CMAKE_SYSTEM_NAME STREQUAL "MSYS"))
	install (FILES "setting.json" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ DESTINATION "C:/pmms/")
else()
	install (FILES "setting.json" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ DESTINATION "/etc/pmms/")
endif()
