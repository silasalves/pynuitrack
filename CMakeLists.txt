cmake_minimum_required(VERSION 2.8.3)

add_definitions(-std=c++11)

# REQUIRED
set(NUITRACK_SDK_PATH add_nuitrack_sdk_path_here)

## OPTIONAL: uncomment these variables and change them in case you are using
## a custom installation of Boost.
# set(BOOST_INCLUDE_PATH add_boost_include_dir_here)
# set(BOOST_STAGE_PATH add_boost_stage_dir_here)

if(UNIX)
	IF (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
		set(PLATFORM_DIR linux_arm)
	ELSEIF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "amd64")
		set(PLATFORM_DIR linux64)
	ENDIF()
elseif(WIN32)
	IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(PLATFORM_DIR win32)
	ELSEIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(PLATFORM_DIR win64)
	ENDIF()
endif()

FIND_PACKAGE(PythonInterp)

if(BOOST_INCLUDE_PATH AND BOOST_STAGE_PATH)
  message(STATUS "Using custom Boost C++ library.")
  set(Boost_LIBRARIES
    ${BOOST_STAGE_PATH}/libboost_python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}.so
    ${BOOST_STAGE_PATH}/libboost_numpy${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}.so
  )
  set(Boost_INCLUDE_DIRS ${BOOST_INCLUDE_PATH})
else()
  FIND_PACKAGE(Boost COMPONENTS python numpy REQUIRED)
endif()
FIND_PACKAGE(PythonInterp ${PYTHON_VERSION_MAJOR} REQUIRED)
FIND_PACKAGE(PythonLibs ${PYTHON_VERSION_MAJOR} REQUIRED)

message(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_EXECUTABLE = ${PYTHON_EXECUTABLE}")
message(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")
message(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${NUITRACK_SDK_PATH}/Nuitrack/include
  ${NUITRACK_SDK_PATH}/Nuitrack/include/middleware
)

link_directories(
  ${NUITRACK_SDK_PATH}/Nuitrack/lib/${PLATFORM_DIR}
)

LINK_LIBRARIES(
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARIES}
  nuitrack
)

PYTHON_ADD_MODULE(pynuitrack src/pynuitrack.cpp)
