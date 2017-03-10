cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

# The name of our project is "FreeImageAlgorithms".  CMakeLists files in this project can
# refer to the root source directory of the project as ${FreeImageIcs_SOURCE_DIR} and
# to the root binary directory of the project as ${FreeImageIcs_BINARY_DIR}.
PROJECT(FreeImageIcs)

SUBDIRS(src zlib Tests)

SET(CMAKE_DEBUG_POSTFIX "_d")

# Look for the FreeImage library
FIND_PACKAGE(FreeImage REQUIRED) 
FIND_PACKAGE(FreeImageAlgorithms REQUIRED)

# Make sure the compiler can find include files from our library.
INCLUDE_DIRECTORIES(include src 
							zlib
							Tests
							libics-1.5.1
							${FREEIMAGE_INCLUDE_PATH}
                            ${FREEIMAGEALGORITHMS_INCLUDE_PATH}
)

SET (LIBRARY_OUTPUT_PATH ${FreeImageIcs_BINARY_DIR}/bin)
SET (EXECUTABLE_OUTPUT_PATH ${FreeImageIcs_BINARY_DIR}/bin)

CONFIGURE_FILE(${FreeImageIcs_SOURCE_DIR}/Tests/Constants.h.template ${FreeImageIcs_SOURCE_DIR}/Tests/Constants.h)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH} ${FREEIMAGEALGORITHMS_LIBRARY})

IF(WIN32)
SET(CMAKE_CXX_STANDARD_LIBRARIES "user32.lib gdi32.lib")
ENDIF(WIN32)

MESSAGE( STATUS "EXECUTABLE_OUTPUT_PATH: " ${EXECUTABLE_OUTPUT_PATH} )

ADD_DEFINITIONS(-DFREEIMAGE_EXPORTS -DICS_ZLIB -D_CRT_SECURE_NO_WARNINGS) # -DHAVE_STRINGS_STRCASECMP

SET(SRCS zlib/adler32.c
	     zlib/compress.c
	     zlib/crc32.c
	     zlib/deflate.c
	     zlib/example.c
	     zlib/gzio.c
	     zlib/infback.c
	     zlib/inffast.c
	     zlib/inflate.c
	     zlib/inftrees.c
	     zlib/trees.c
	     zlib/uncompr.c
	     zlib/zutil.c
		 libics-1.5.1/libics_binary.c
		 libics-1.5.1/libics_data.c
		 libics-1.5.1/libics_gzip.c
		 libics-1.5.1/libics_history.c
		 libics-1.5.1/libics_preview.c
		 libics-1.5.1/libics_read.c
		 libics-1.5.1/libics_sensor.c
		 libics-1.5.1/libics_top.c
		 libics-1.5.1/libics_util.c
		 libics-1.5.1/libics_write.c
		 src/FreeImageIcs_IO.cpp
		 src/FreeImageIcs_MetaData.cpp
		 src/FreeImageIcs_Private.cpp
		 exports.def
)

SET(PUBLIC_HEADERS
         libics-1.5.1/libics.h
         include/FreeImageIcs_IO.h
         include/FreeImageIcs_MetaData.h
)

LINK_DIRECTORIES(${GCI_DLL_PATH})

ADD_LIBRARY (freeimageics SHARED ${SRCS})

# Link the executable to the FreeImage library.
TARGET_LINK_LIBRARIES (freeimageics ${FREEIMAGEALGORITHMS_LIBRARY} ${FREEIMAGE_LIBRARY})

IF (UNIX)
  INSTALL(TARGETS freeimageics RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

  SET(CMAKE_INSTALL_PREFIX /usr/local                                           CACHE PATH "Install prefix")
  SET(BIN_INSTALL_DIR      ${CMAKE_INSTALL_PREFIX}/bin                          CACHE PATH "Binary install dir")
  SET(LIB_INSTALL_DIR      ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}             CACHE PATH "Library install dir")
  SET(LIBEXEC_INSTALL_DIR  ${CMAKE_INSTALL_PREFIX}/libexec                      CACHE PATH "Private executable install dir")
  SET(INCLUDE_INSTALL_DIR  ${CMAKE_INSTALL_PREFIX}/include/FreeImageIcs         CACHE PATH "Include install dir")
  SET(SYSCONF_INSTALL_DIR  ${CMAKE_INSTALL_PREFIX}/etc                          CACHE PATH "Configuration install dir")
  SET(SHARE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/share                        CACHE PATH "Data install dir")

  INSTALL (FILES ${PUBLIC_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR})

ENDIF (UNIX)
