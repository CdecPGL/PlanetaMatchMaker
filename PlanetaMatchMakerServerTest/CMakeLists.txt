cmake_minimum_required (VERSION 3.8)

file(GLOB_RECURSE source_files "*.cpp")
add_executable (PlanetaMatchMakerServerTest ${source_files})

# Boost Library
find_package(Boost REQUIRED COMPONENTS unit_test_framework)
file(GLOB_RECURSE boost_library_paths "${Boost_LIBRARY_DIRS}/*boost*")
# to avoid warnings, use SYSTEM (-isystem option of gcc)
target_include_directories(PlanetaMatchMakerServerTest SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(PlanetaMatchMakerServerTest ${Boost_LIBRARIES})

target_link_libraries(PlanetaMatchMakerServerTest PlanetaMatchMakerServerLib)

# Test
add_test(
  NAME PlanetaMatchMakerServerTest
  COMMAND $<TARGET_FILE:PlanetaMatchMakerServerTest>
)
