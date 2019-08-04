cmake_minimum_required (VERSION 3.8)

file(GLOB_RECURSE source_files "*.cpp")
add_executable (PlanetaMatchMakerServerTest ${source_files})

# Boost Library
find_package(Boost REQUIRED)
file(GLOB_RECURSE boost_library_paths "${Boost_LIBRARY_DIRS}/*")
target_include_directories(PlanetaMatchMakerServerTest PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(PlanetaMatchMakerServerTest ${boost_library_paths})

target_link_libraries(PlanetaMatchMakerServerTest PlanetaMatchMakerServerLib)
