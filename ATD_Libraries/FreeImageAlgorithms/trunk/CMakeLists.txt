cmake_minimum_required(VERSION 2.6)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

SET(CMAKE_CONFIGURATION_TYPES "Debug;VerboseDebug;Release" CACHE STRING "" FORCE)

# Set VerboseDebug build type to have the safe linker flags as debug
SET(CMAKE_CXX_FLAGS_VERBOSEDEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DVERBOSE_DEBUG /DFORTIFY /DGENERATE_DEBUG_IMAGES" CACHE STRING "" FORCE)
SET(CMAKE_C_FLAGS_VERBOSEDEBUG ${CMAKE_CXX_FLAGS_VERBOSEDEBUG} CACHE STRING "" FORCE)
SET(CMAKE_EXE_LINKER_FLAGS_VERBOSEDEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG} CACHE STRING "" FORCE)
SET(CMAKE_SHARED_LINKER_FLAGS_VERBOSEDEBUG ${CMAKE_SHARED_LINKER_FLAGS_DEBUG} CACHE STRING "" FORCE)

# The name of our project is "FreeImageAlgorithms".  CMakeLists files in this project can
# refer to the root source directory of the project as ${FreeImageAlgorithms_SOURCE_DIR} and
# to the root binary directory of the project as ${FreeImageAlgorithms_BINARY_DIR}.
PROJECT(FreeImageAlgorithms)

SUBDIRS(src Tests)

SET(CMAKE_DEBUG_POSTFIX "_d")

ADD_DEFINITIONS(-D _CRT_SECURE_NO_WARNINGS)

# Look for the FreeImage library
FIND_PACKAGE(FreeImage REQUIRED) 

# Make sure the compiler can find include files from our library.
INCLUDE_DIRECTORIES(include src/agg/include Tests ${FREEIMAGE_INCLUDE_PATH})

SET (LIBRARY_OUTPUT_PATH ${FreeImageAlgorithms_BINARY_DIR}/bin)
SET (EXECUTABLE_OUTPUT_PATH ${FreeImageAlgorithms_BINARY_DIR}/bin)

CONFIGURE_FILE(${FreeImageAlgorithms_SOURCE_DIR}/Constants.h.template ${FreeImageAlgorithms_SOURCE_DIR}/include/Constants.h)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})

IF(WIN32)
SET(CMAKE_CXX_STANDARD_LIBRARIES "user32.lib gdi32.lib")
ENDIF(WIN32)

MESSAGE( STATUS "EXECUTABLE_OUTPUT_PATH: " ${EXECUTABLE_OUTPUT_PATH} )
