cmake_minimum_required (VERSION 3.21.3)

file(GLOB_RECURSE source_files "*.cpp")
add_executable (PlanetaMatchMakerServerTest ${source_files})

# Boost Library
find_package(Boost 1.77.0 REQUIRED COMPONENTS unit_test_framework json)
# to avoid warnings, use SYSTEM (-isystem option of clang)
target_include_directories(PlanetaMatchMakerServerTest SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(PlanetaMatchMakerServerTest ${Boost_LIBRARIES})


# Additional compiler configurations
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # Suppress unused-result warning because ignoring return value of functions is common case in test code
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4834")
elseif((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
  # Suppress unused-result warning because ignoring return value of functions is common case in test code
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result")
endif()

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
