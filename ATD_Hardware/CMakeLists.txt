CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../ATD_Microscopy/Microscope Systems/Resources/cmake_modules/")
INCLUDE(${CMAKE_SOURCE_DIR}/cmake_modules/GET_RESOURCE_FILES.cmake)

SET(CMAKE_CONFIGURATION_TYPES "Debug;VerboseDebug;Developmental;Release" CACHE STRING "" FORCE)
    
# Set Developmental build type to have the safe linker flags as debug
SET(CMAKE_CXX_FLAGS_DEVELOPMENTAL "${CMAKE_CXX_FLAGS_DEBUG} /DDEVELOPMENTAL_FEATURES /DFORTIFY" CACHE STRING "" FORCE)
SET(CMAKE_C_FLAGS_DEVELOPMENTAL ${CMAKE_CXX_FLAGS_DEVELOPMENTAL} CACHE STRING "" FORCE)
SET(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENTAL ${CMAKE_EXE_LINKER_FLAGS_DEBUG} CACHE STRING "" FORCE)

# Set VerboseDebug build type to have the safe linker flags as debug
SET(CMAKE_CXX_FLAGS_VERBOSEDEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DVERBOSE_DEBUG /DFORTIFY" CACHE STRING "" FORCE)
SET(CMAKE_C_FLAGS_VERBOSEDEBUG ${CMAKE_CXX_FLAGS_VERBOSEDEBUG} CACHE STRING "" FORCE)
SET(CMAKE_EXE_LINKER_FLAGS_VERBOSEDEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG} CACHE STRING "" FORCE)

    
# The name of our project is "Hardware".  CMakeLists files in this project can
# refer to the root source directory of the project as ${Hardware_SOURCE_DIR}and
# to the root binary directory of the project as ${Hardware_BINARY_DIR}.
PROJECT(Hardware)

SET(MICROSCOPY_TOPLEVEL_SOURCE_DIR ${Hardware_SOURCE_DIR})
SET(MICROSCOPY_RUNTIME_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
SET(CMAKE_DEBUG_POSTFIX "_d")

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/bin CACHE PATH "Output directory for executable"  FORCE )

# Look for the FreeImage library
FIND_PACKAGE(FreeImage REQUIRED) 

FIND_PATH (GCI_DLL_PATH StringUtils.lib FORCE "C://dlls")
FIND_PATH (PYTHON_PATH python.exe FORCE "C://Python25")
FIND_PATH (LIBRARIES_DIR_PATH UIModule ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../ATD_Libraries)
FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR})
FIND_PATH (CVI_DIR_PATH cvi.exe FORCE "C://Program\ Files//National\ Instruments//CVI80")
FIND_FILE (CONFIG_FILE_PATH config.ini.in [NO_DEFAULT_PATH])
			
FIND_PACKAGE(ImageViewer REQUIRED) 
FIND_PACKAGE(IcsViewer REQUIRED) 
FIND_PACKAGE(GciLibraries REQUIRED) 
FIND_PACKAGE(I2CModule REQUIRED)
FIND_PACKAGE(CviToolbox REQUIRED) 			
FIND_PACKAGE(MicroscopeModules REQUIRED) 	

SET(HARDWARE_DEVICE_SRCS 		${HARDWARE_DIR_PATH}/microscope/trunk/HardwareDevice.c
                                ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/SingleRangeHardwareDevice/trunk/single_range_hardware_device.c
                                ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/SingleRangeHardwareDevice/trunk/single_range_hardware_device_ui.c
                                )

# Make sure the compiler can find include files from our library.
INCLUDE_DIRECTORIES(
					${MICROSCOPY_TOPLEVEL_SOURCE_DIR}
					${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/microscope/trunk
                    ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/SingleRangeHardwareDevice/trunk
					${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/ATD_UsbInterface/trunk/
                    ${MS_MODULES_DIR_PATH}/Focus/trunk/
					${FREEIMAGE_INCLUDE_PATH}
					${CVI_TOOLBOX_INCLUDE_PATH}
					${PYTHON_PATH}/include
					)
					
LINK_DIRECTORIES(${GCI_DLL_PATH}
				 ${HARDWARE_DIR_PATH}/Stage/trunk/LStep
				 ${PYTHON_PATH}/libs
				 ${CVI_DIR_PATH}/extlib
				 )

MAKE_WINDOWS_PATH(MICROSCOPY_TOPLEVEL_SOURCE_DIR)
MAKE_WINDOWS_PATH(MICROSCOPY_RUNTIME_OUTPUT_DIR)
MAKE_WINDOWS_PATH(CONFIG_FILE_PATH)
MAKE_WINDOWS_PATH(HARDWARE_DIR_PATH)
MAKE_WINDOWS_PATH(MS_MODULES_DIR_PATH)

if(NOT EXISTS ${CONFIG_FILE_PATH_ESCAPED})
	MESSAGE(FATAL_ERROR "Config file not found")
ENDIF (NOT EXISTS ${CONFIG_FILE_PATH_ESCAPED})

CONFIGURE_FILE(${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/Config.h.in
			   ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/Config.h ESCAPE_QUOTES)
			   
CONFIGURE_FILE(${CONFIG_FILE_PATH_ESCAPED}
			   ${MICROSCOPY_RUNTIME_OUTPUT_DIR}/config.ini ESCAPE_QUOTES)
			   
MESSAGE( STATUS "CONFIG.INI.IN PATH: " ${CONFIG_FILE_PATH} )
				 

MESSAGE( STATUS "MICROSCOPY_RUNTIME_OUTPUT_DIR: " ${MICROSCOPY_RUNTIME_OUTPUT_DIR} )
MESSAGE( STATUS "CVI_DIR_PATH: " ${CVI_DIR_PATH} )

ADD_DEFINITIONS(-D VS_COMPILER -D WIN32_LEAN_AND_MEAN -D ONE_THREAD_PER_CALLBACK 
                -DNO_PROFILING -D THREADED_CAM_AQ -DLOCATIONS_DEFINED -DNO_ICSVIEWER_DRAG_DROP -D FTDI_NO_VIRTUAL_COMPORT)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${GCI_DLL_PATH})
                      
ADD_DEFINITIONS(-D BUILT_RESOURCE_TABLE)

SET(CVI_LIBS cviauto.lib cvintwrk.lib cvirt.lib cvisupp.lib cviwmain.lib analysis.lib )
SET(GCI_LIBS ${FREEIMAGE_LIBRARY} freeimagealgorithms.lib freeimageics.lib StringUtils.lib ImageViewer.lib)

SET(CMAKE_CXX_STANDARD_LIBRARIES "user32.lib gdi32.lib ole32.lib shell32.lib vfw32.lib advapi32.lib comctl32.lib")
SET(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libc.lib\")

SET(${HARDWARE_RESOURCE_SRCS} "")
 
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsPowerSwitch.txt)
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsCubeSlider.txt)
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsOpticalPath.txt)
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsShutter.txt)
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsScanner.txt)
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsLaser.txt)
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsCondenser.txt)
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsLaserPowerMonitor.txt)
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsCamera.txt)
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsCoarseZDrive.txt)
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsZDrive.txt)
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsFastLineAndI2CTest.txt)
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsLamp.txt)

# Surrey Stuff
#INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsMotorMikes.txt)
#INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsOpticalList.txt)
#INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsTemperaturMonitor.txt)
#INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsBatchCounter.txt)
#INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsBeamScanner.txt)

# Manual STuff
#INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsPmtSet.txt)
#INCLUDE(${CMAKE_SOURCE_DIR}/CMakeListsFilterSet.txt)

SET(ALL_COMMON_SRCS 
                    ${I2C_SRCS} 
                    ${CVI_TOOLLIB_SRCS}
                    ${LIBRARY_SRCS}
                    ${IMAGEVIEWER_SRCS}
                    ${ICSVIEWER_SRCS}
                    ${HARDWARE_DEVICE_SRCS}      
                )
                
                
########################  Build resource array for uir, and other resources
                     
## First we add the executable that generates the table of resources
ADD_EXECUTABLE(BuildResourceTable ${LIBRARIES_DIR_PATH}/BuildResourceTable/BuildResourceTable.c)
 
GET_RESOURCE_FILES("${ALL_COMMON_SRCS}" RESOURCE_FILE_PATHS)

FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Resources.dat ${RESOURCE_FILE_PATHS})

## Add the command to generate the source code
ADD_CUSTOM_COMMAND (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ResourceFiles.h
  COMMAND BuildResourceTable "${CMAKE_CURRENT_BINARY_DIR}/Resources.dat" "${CMAKE_CURRENT_BINARY_DIR}/ResourceFiles.h"
  DEPENDS BuildResourceTable
  )
                             
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
                      
ADD_DEFINITIONS(-D BUILT_RESOURCE_TABLE)
     
SET_SOURCE_FILES_PROPERTIES(hardware-dependancies-static PROPERTIES OBJECT_DEPENDS BuildResourceTable)
                
ADD_LIBRARY(hardware-dependancies-static STATIC
                                                ${ALL_COMMON_SRCS} 
                                                ${CMAKE_CURRENT_BINARY_DIR}/ResourceFiles.h
)
     
####################################################################
                
                
                
#LIST(APPEND HARDWARE_RESOURCE_SRCS ${ALL_COMMON_SRCS})
                   
#GET_RESOURCE_FILES("${HARDWARE_RESOURCE_SRCS}" RESOURCE_FILE_PATHS)

#MESSAGE( STATUS "GET_RESOURCE_FILES: " ${RESOURCE_FILE_PATHS})

## First we add the executable that generates the table of resources
#ADD_EXECUTABLE(BuildResourceTable ${LIBRARIES_DIR_PATH}/BuildResourceTable/BuildResourceTable.c)
 
## Add the command to generate the source code
#ADD_CUSTOM_COMMAND (
#  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ResourceFiles.h
#  COMMAND BuildResourceTable "${CMAKE_CURRENT_BINARY_DIR}/ResourceFiles.h" ${RESOURCE_FILE_PATHS}
#  DEPENDS BuildResourceTable
#)
#
#SET_SOURCE_FILES_PROPERTIES(hardware-dependancies-static PROPERTIES OBJECT_DEPENDS BuildResourceTable)
#                
#ADD_LIBRARY(hardware-dependancies-static STATIC
#                                                ${ALL_COMMON_SRCS} 
#                                                ${CMAKE_CURRENT_BINARY_DIR}/ResourceFiles.h
#)

TARGET_LINK_LIBRARIES (hardware-dependancies-static comctl32.lib setupapi.lib )

ADD_EXECUTABLE (PowerSwitch WIN32 ${PS_SRCS})
TARGET_LINK_LIBRARIES (PowerSwitch hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)

ADD_EXECUTABLE (CubeSlider WIN32 ${CUBE_SRCS})
TARGET_LINK_LIBRARIES (CubeSlider hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)
        
ADD_EXECUTABLE (OpticalPath WIN32 ${OP_SRCS})
TARGET_LINK_LIBRARIES (OpticalPath hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)
        
ADD_EXECUTABLE (Shutter WIN32 ${SHUTTER_SRCS})
TARGET_LINK_LIBRARIES (Shutter hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)
         
ADD_EXECUTABLE (Scanner WIN32 ${SCANNER_SRCS})
TARGET_LINK_LIBRARIES (Scanner hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)

ADD_EXECUTABLE (Laser WIN32 ${LASER_SRCS})
TARGET_LINK_LIBRARIES (Laser hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)

ADD_EXECUTABLE (Laser WIN32 ${CONDENSER_SRCS})
TARGET_LINK_LIBRARIES (Condenser hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)

ADD_EXECUTABLE (LaserPowerMonitor WIN32 ${LASERPOWERMONITOR_SRCS})
TARGET_LINK_LIBRARIES (LaserPowerMonitor hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)

ADD_EXECUTABLE (Camera WIN32 ${CAMERA_SRCS})
TARGET_LINK_LIBRARIES (Camera hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} dcamapi.lib setupapi.lib uc3010.lib)

ADD_EXECUTABLE (CoarseZDrive WIN32 ${COARSEZDRIVE_SRCS})
TARGET_LINK_LIBRARIES (CoarseZDrive hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)

ADD_EXECUTABLE (ZDrive WIN32 ${ZDRIVE_SRCS})
TARGET_LINK_LIBRARIES (ZDrive hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib LStep4.lib)

ADD_EXECUTABLE (FastLineAndI2CTest WIN32 ${FASTLINE_AND_I2C_TEST_SRCS})
TARGET_LINK_LIBRARIES (FastLineAndI2CTest hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)

ADD_EXECUTABLE (Lamp WIN32 ${LAMP_SRCS})
TARGET_LINK_LIBRARIES (Lamp hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)


#ADD_EXECUTABLE (BatchCounter WIN32 ${BATCHCOUNTER_SRCS})
#TARGET_LINK_LIBRARIES (BatchCounter hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)

#ADD_EXECUTABLE (TemperatureMonitor WIN32 ${TEMP_SRCS})
#TARGET_LINK_LIBRARIES (TemperatureMonitor hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)

#ADD_EXECUTABLE (OpticalLift WIN32 ${OL_SRCS})
#TARGET_LINK_LIBRARIES (OpticalLift hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)

#ADD_EXECUTABLE (MotorMike WIN32 ${MOTORMIKE_SRCS})
#TARGET_LINK_LIBRARIES (MotorMike hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)

#ADD_EXECUTABLE (PmtSet WIN32 ${PMT_SRCS})
#TARGET_LINK_LIBRARIES (PmtSet hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)

#ADD_EXECUTABLE (FilterSet WIN32 ${FILTERSET_SRCS})
#TARGET_LINK_LIBRARIES (FilterSet hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)
 
#ADD_EXECUTABLE (BeamScanner WIN32 ${BS_SRCS})
#TARGET_LINK_LIBRARIES (BeamScanner hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)