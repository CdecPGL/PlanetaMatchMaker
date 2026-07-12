cmake_minimum_required (VERSION 3.21.3)

# Main executable
add_executable (
	PlanetaMatchMakerServer
	"source/main/main.cpp"
	"source/main/log_initializer.cpp"
)
target_link_libraries(PlanetaMatchMakerServer PRIVATE PlanetaMatchMakerServerLib)

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
