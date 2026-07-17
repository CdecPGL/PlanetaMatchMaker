cmake_minimum_required (VERSION 3.21.3)

file(GLOB_RECURSE source_files CONFIGURE_DEPENDS "source/*.cpp")
add_library (PlanetaMatchMakerServerLib STATIC ${source_files})

if (STATIC_LINK_DEPENDENCIES)
	if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		message(FATAL_ERROR "Static link dependencies is only supported for GNU and Clang")
	endif()

	# Link boost libraries statically
	set(Boost_USE_STATIC_LIBS ON)
else()
	# to avoid linker error "undefined reference to `std::filesystem::__cxx11::**"
	# http://nanapro.org/en-us/forum/index.php?u=/topic/266/ggcompiling-with-std-filesystem
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_link_libraries(PlanetaMatchMakerServerLib PRIVATE stdc++fs)
	endif()
endif()

target_include_directories(
	PlanetaMatchMakerServerLib
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/library"
		"${CMAKE_CURRENT_SOURCE_DIR}/source"
)

# Boost Library
find_package(Boost 1.77.0 CONFIG REQUIRED COMPONENTS thread coroutine date_time filesystem log json)
target_include_directories(PlanetaMatchMakerServerLib SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(PlanetaMatchMakerServerLib PUBLIC ${Boost_LIBRARIES})

# OpenSSL Library
if (STATIC_LINK_DEPENDENCIES)
	set(OPENSSL_USE_STATIC_LIBS ON)
endif()
find_package(OpenSSL 3.0 REQUIRED)
target_link_libraries(PlanetaMatchMakerServerLib PUBLIC OpenSSL::SSL OpenSSL::Crypto)

# Compiler settings
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# -D_WIN32_WINNT=0x0A00: To avoid error of Boost Library 1.89 for MSVC
	# _CRT_SECURE_NO_WARNINGS: To avoid error for getenv
	target_compile_definitions(PlanetaMatchMakerServerLib PUBLIC _WIN32_WINNT=0x0A00 _CRT_SECURE_NO_WARNINGS)
elseif((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
	if (ENABLE_COVERAGE)
		target_compile_options(PlanetaMatchMakerServerLib PUBLIC --coverage)
		target_link_options(PlanetaMatchMakerServerLib PUBLIC --coverage)
	endif()
endif()
