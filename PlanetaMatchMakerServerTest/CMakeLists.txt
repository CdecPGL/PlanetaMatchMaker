cmake_minimum_required (VERSION 3.21.3)

file(GLOB_RECURSE source_files "*.cpp")
add_executable (PlanetaMatchMakerServerTest ${source_files})

# Boost Library
find_package(Boost 1.77.0 REQUIRED COMPONENTS unit_test_framework json)
# to avoid warnings, use SYSTEM (-isystem option of gcc)
target_include_directories(PlanetaMatchMakerServerTest SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(PlanetaMatchMakerServerTest ${Boost_LIBRARIES})

target_link_libraries(PlanetaMatchMakerServerTest PlanetaMatchMakerServerLib)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# -D_WIN32_WINNT=0x0601: To avoid error of Boost Library 1.77 for MSVC
	# _CRT_SECURE_NO_WARNINGS: To avoid error for getenv, etc...
  add_definitions(-D_WIN32_WINNT=0x0601 _CRT_SECURE_NO_WARNINGS)
endif()

# Test
add_test(
  NAME PlanetaMatchMakerServerTest
  COMMAND $<TARGET_FILE:PlanetaMatchMakerServerTest>
)
