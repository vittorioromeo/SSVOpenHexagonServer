cmake_minimum_required(VERSION 2.8)

project(SSVOpenHexagonServer)

# Set options
set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
set(SFML_STATIC_LIBS FALSE CACHE BOOL "Choose whether SFML is linked statically or not.")
if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/;${CMAKE_MODULE_PATH}")

# Make sure that FindSFML.cmake searches for the static libraries
if(SFML_STATIC_LIBS)
	set(SFML_STATIC_LIBRARIES TRUE)
endif()

# Find SFML
find_package(SFML 2 COMPONENTS system network)

# Output an error if SFML wasn't found
if(SFML_FOUND)
	include_directories(${SFML_INCLUDE_DIR})
else()
	set(SFML_ROOT "" CACHE PATH "SFML top-level directory")
	message("\n-> SFML directory not found. Set SFML_ROOT to SFML's top-level path (containing \"include\" and \"lib\" directories).")
	message("-> Make sure the SFML libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()

# Add the source files
set(SRC_DIR "src")
set(INC_DIR "src")

set(
	SOURCES
	${INC_DIR}/${PROJECT_NAME}/MySQLSession/MySQLSession.hpp
	${INC_DIR}/${PROJECT_NAME}/PacketHandler/PacketHandler.hpp
	${SRC_DIR}/${PROJECT_NAME}/MySQLSession/MySQLSession.cpp
	${SRC_DIR}/${PROJECT_NAME}/PacketHandler/PacketHandler.cpp
	${SRC_DIR}/${PROJECT_NAME}/main.cpp
)

# Tell CMake to build a executable 
add_executable(${PROJECT_NAME} ${SOURCES})

# Link SFML
target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})

# On Windows install required audio libraries
if(WIN32)
	install(FILES ${SFML_ROOT}/bin/openal32.dll ${SFML_ROOT}/bin/libsndfile-1.dll
			DESTINATION .)
endif()