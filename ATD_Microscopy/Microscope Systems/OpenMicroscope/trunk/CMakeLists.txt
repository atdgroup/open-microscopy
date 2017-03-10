CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(${CMAKE_SOURCE_DIR}/../../Resources/cmake_modules/GET_RESOURCE_FILES.cmake)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../Resources/cmake_modules/")

SET(CMAKE_CONFIGURATION_TYPES "Debug;VerboseDebug;Developmental;Release" CACHE STRING "" FORCE)

# Set Developmental build type to have the safe linker flags as debug
SET(CMAKE_CXX_FLAGS_DEVELOPMENTAL "${CMAKE_CXX_FLAGS_DEBUG} /DDEVELOPMENTAL_FEATURES /DFORTIFY" CACHE STRING "" FORCE)
SET(CMAKE_C_FLAGS_DEVELOPMENTAL ${CMAKE_CXX_FLAGS_DEVELOPMENTAL} CACHE STRING "" FORCE)
SET(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENTAL ${CMAKE_EXE_LINKER_FLAGS_DEBUG} CACHE STRING "" FORCE)

# Set VerboseDebug build type to have the safe linker flags as debug
SET(CMAKE_CXX_FLAGS_VERBOSEDEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DVERBOSE_DEBUG /DFORTIFY" CACHE STRING "" FORCE)
SET(CMAKE_C_FLAGS_VERBOSEDEBUG ${CMAKE_CXX_FLAGS_VERBOSEDEBUG} CACHE STRING "" FORCE)
SET(CMAKE_EXE_LINKER_FLAGS_VERBOSEDEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG} CACHE STRING "" FORCE)

PROJECT(OpenMicroscope)
SET(PROJECT_NAME "OpenMicroscope")

SET(MICROSCOPY_BUILD_XML_RPC_SERVER FALSE CACHE BOOL "Enable xml-rpc server")
SET(MICROSCOPY_BUILD_WEBSERVER FALSE CACHE BOOL "Enable embedded webserver")

SET(MICROSCOPY_TOPLEVEL_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
SET(MICROSCOPY_RUNTIME_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin")
SET(CMAKE_DEBUG_POSTFIX "_d")

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/bin" CACHE PATH "Output directory for executable"  FORCE )

FIND_PACKAGE(FreeImage REQUIRED) 
FIND_PACKAGE(IcsViewer REQUIRED) 
FIND_PACKAGE(GciLibraries REQUIRED) 
FIND_PACKAGE(MicroscopeModules REQUIRED) 

FIND_PATH (GCI_DLL_PATH StringUtils.lib FORCE "../../../../ATD_Dlls/trunk")
FIND_PATH (PYTHON_PATH python.exe FORCE "C://Python27")
FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)
FIND_PATH (MS_MICROSCOPY_DIR_PATH Resources ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../Microscope\ Systems NO_DEFAULT_PATH)
FIND_PATH (CVI_DIR_PATH cvi.exe FORCE "C://Program\ Files//National\ Instruments//CVI80")

MAKE_WINDOWS_PATH(MICROSCOPY_TOPLEVEL_SOURCE_DIR)
MAKE_WINDOWS_PATH(MICROSCOPY_RUNTIME_OUTPUT_DIR)
MAKE_WINDOWS_PATH(HARDWARE_DIR_PATH)
MAKE_WINDOWS_PATH(MS_MICROSCOPY_DIR_PATH)

#SET(USE_CAMERA_UPIX3010 TRUE)

CONFIGURE_FILE(${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/MainApplication/Config.h.in
			   ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/MainApplication/Config.h ESCAPE_QUOTES)
			   
CONFIGURE_FILE(${MICROSCOPY_RUNTIME_OUTPUT_DIR}/config.ini.in
			   ${MICROSCOPY_RUNTIME_OUTPUT_DIR}/config.ini ESCAPE_QUOTES)
               
CONFIGURE_FILE(${MS_MICROSCOPY_DIR_PATH}/Resources/microscope.rc.in
			   ${MICROSCOPY_RUNTIME_OUTPUT_DIR}/microscope.rc ESCAPE_QUOTES)
			   
FIND_PACKAGE(PmtModule REQUIRED) 
FIND_PACKAGE(CubeSliderModule REQUIRED)               
FIND_PACKAGE(PythonWrapperModule REQUIRED)  
FIND_PACKAGE(CameraModule REQUIRED) 
FIND_PACKAGE(CviToolbox REQUIRED) 
FIND_PACKAGE(StageModule REQUIRED)
FIND_PACKAGE(StagePlateModule REQUIRED)
FIND_PACKAGE(ObjectiveModule REQUIRED)
FIND_PACKAGE(LampModule REQUIRED)
FIND_PACKAGE(OpticalPathModule REQUIRED)
FIND_PACKAGE(ScannerModule REQUIRED)
FIND_PACKAGE(ShutterModule REQUIRED)
FIND_PACKAGE(ZdriveModule REQUIRED)
FIND_PACKAGE(PowerModule REQUIRED)
FIND_PACKAGE(FilterModule REQUIRED)
FIND_PACKAGE(LaserModule REQUIRED)
FIND_PACKAGE(I2CModule REQUIRED)
FIND_PACKAGE(LaserPowerMonitorModule REQUIRED)

SET(MS_SRCS 		${HARDWARE_DIR_PATH}/microscope/trunk/Microscope.c
					${HARDWARE_DIR_PATH}/microscope/trunk/Microscope.h
                    ${HARDWARE_DIR_PATH}/microscope/trunk/MicroscopeUI.uir
					${HARDWARE_DIR_PATH}/microscope/trunk/MicroscopeCommon.c
					${HARDWARE_DIR_PATH}/microscope/trunk/MicroscopeCommon.h
					${HARDWARE_DIR_PATH}/microscope/trunk/MicroscopeCommonUI.c
					${HARDWARE_DIR_PATH}/microscope/trunk/HardwareTypes.h
                    ${HARDWARE_DIR_PATH}/microscope/trunk/HardwareDevice.h
                    ${HARDWARE_DIR_PATH}/microscope/trunk/HardwareDevice.c
					${HARDWARE_DIR_PATH}/microscope/trunk/OpticalCalibrationDevice.h
                    ${HARDWARE_DIR_PATH}/microscope/trunk/OpticalCalibrationDevice.c
					${HARDWARE_DIR_PATH}/microscope/trunk/OpenMicroscope.c
					${HARDWARE_DIR_PATH}/microscope/trunk/OpenMicroscope.h
					${HARDWARE_DIR_PATH}/microscope/trunk/OpenMicroscopeUI.c
					${HARDWARE_DIR_PATH}/microscope/trunk/OpenMicroscopeUI.h
					${HARDWARE_DIR_PATH}/microscope/trunk/OpenMicroscopeUI.uir
)


SET(MICROSCOPY_SRCS ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/MainApplication/Microscopy.c
					${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/MainApplication/Microscopy.h
					${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/MainApplication/Microscopy_ui.h
					${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/MainApplication/Microscopy_ui.uir
					${MICROSCOPY_RUNTIME_OUTPUT_DIR}/microscope.rc
					${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/MainApplication/Config.h
)				

SET(WEBSERVER_SRCS  ${LIBRARIES_DIR_PATH}/mongoose/mongoose.c
                    ${LIBRARIES_DIR_PATH}/mongoose/mongoose.h
)


SET(PROTOTYPE_AUTOFOCUS_SRCS ${MS_MODULES_DIR_PATH}/PrototypeAutoFocus/PrototypeAutoFocus.c
							 ${MS_MODULES_DIR_PATH}/PrototypeAutoFocus/PrototypeAutoFocus.h
							 ${MS_MODULES_DIR_PATH}/PrototypeAutoFocus/PrototypeAutoFocus_ui.h
							 ${MS_MODULES_DIR_PATH}/PrototypeAutoFocus/PrototypeAutoFocus_ui.uir
)
		
	

SOURCE_GROUP(XmlRpc FILES ${XMLRPC_SRCS})
SOURCE_GROUP(Microscope FILES ${MS_SRCS})
SOURCE_GROUP(Microscopy FILES ${MICROSCOPY_SRCS})
SOURCE_GROUP(Webserver FILES ${WEBSERVER_SRCS})
SOURCE_GROUP(PrototypeAutofocus FILES ${PROTOTYPE_AUTOFOCUS_SRCS})
SOURCE_GROUP(Distortion FILES ${DISTORTION_SRCS})

# Make sure the compiler can find include files from our library.
INCLUDE_DIRECTORIES(
                    ${CMAKE_CURRENT_BINARY_DIR} 
					${FREEIMAGE_INCLUDE_PATH}
	
					${PYTHON_PATH}/include
					${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/MainApplication
                    ${LIBRARIES_DIR_PATH}/mongoose
					${HARDWARE_DIR_PATH}
					${HARDWARE_DIR_PATH}/microscope/trunk
					)
					
LINK_DIRECTORIES(${GCI_DLL_PATH}
				 ${PYTHON_PATH}/libs
				 ${CVI_DIR_PATH}/extlib
				 )            
                 
ADD_DEFINITIONS(-D _CRT_SECURE_NO_WARNINGS -D MICROSCOPE_DUMMY -D MICROSCOPE_PYTHON_AUTOMATION -D VS_COMPILER -D IMAGEVIEWER_IMPORTS -D IMAGEVIEWER_STDCALL
				-D ONE_THREAD_PER_CALLBACK -D OPEN_IN_NEW_WINDOW -D BUILT_RESOURCE_TABLE
				-D WIN32_LEAN_AND_MEAN -D DONT_PROFILE -D THREADED_CAM_AQ 
				-D LOCATIONS_DEFINED -D COBJMACROS -D CINTERFACE -D NO_HARDWARE_PRESENT -D FTDI_NO_VIRTUAL_COMPORT)
		
IF(MICROSCOPY_BUILD_WEBSERVER)
  ADD_DEFINITIONS(-D USE_WEB_SERVER)
ENDIF(MICROSCOPY_BUILD_WEBSERVER)
	
IF(MICROSCOPY_BUILD_XML_RPC_SERVER)
  ADD_DEFINITIONS(-D USE_XML_RPC)
ENDIF(MICROSCOPY_BUILD_XML_RPC_SERVER)

SET(ALL_SRCS ${MICROSCOPY_SRCS}
			 ${CVI_TOOLLIB_SRCS}
			 ${LIBRARY_SRCS}
			 ${XMLRPC_SRCS}
			 ${ICSVIEWER_SRCS}
			 ${CAMERA_SRCS} 
             ${LAMP_SRCS}
			 ${STAGE_SRCS} 
			 ${OPT_PATH_SRCS}
			 ${OBJ_SRCS}
			 ${I2C_SRCS}
			 ${CUBE_SRCS}
			 ${SCANNER_SRCS}
             ${SHUTTER_SRCS}
			 ${SPC_SRCS}
			 ${ZDRIVE_SRCS}
			 ${TEMP_MON}
			 ${PS_SRCS}
			 ${HW_PY_SRCS}
			 ${MS_SRCS}
			 ${MS_MODULES}
             ${WEBSERVER_SRCS}
             ${PMT_SRCS}
             ${FILTER_SRCS}
             ${LASER_SRCS}
             ${STAGE_PLATE_SRCS}
             ${LASERPOWERMONITOR_SRCS}
             )

             
## First we add the executable that generates the table of resources
ADD_EXECUTABLE(BuildResourceTable ${LIBRARIES_DIR_PATH}/BuildResourceTable/BuildResourceTable.c)
 
GET_RESOURCE_FILES("${ALL_SRCS}" RESOURCE_FILE_PATHS)

FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Resources.dat ${RESOURCE_FILE_PATHS})

## Add the command to generate the source code
ADD_CUSTOM_COMMAND (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ResourceFiles.h
  COMMAND BuildResourceTable "${CMAKE_CURRENT_BINARY_DIR}/Resources.dat" "${CMAKE_CURRENT_BINARY_DIR}/ResourceFiles.h"
  DEPENDS BuildResourceTable
  )
                      
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${GCI_DLL_PATH})
                      
ADD_DEFINITIONS(-D BUILT_RESOURCE_TABLE)
               
MESSAGE( STATUS "GET_RESOURCE_FILES: " ${RESOURCE_FILE_PATHS})
MESSAGE( STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})
         
ADD_EXECUTABLE (OpenMicroscope WIN32 ${ALL_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/ResourceFiles.h)
                          
SET(CMAKE_CXX_STANDARD_LIBRARIES "user32.lib gdi32.lib ole32.lib shell32.lib vfw32.lib advapi32.lib comctl32.lib")
SET(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libc.lib\")

TARGET_LINK_LIBRARIES (OpenMicroscope ${FREEIMAGE_LIBRARY} cviauto.lib cvintwrk.lib cvirt.lib cvisupp.lib cviwmain.lib analysis.lib setupapi.lib
							 freeimagealgorithms.lib freeimageics.lib StringUtils.lib ImageViewer.lib dbghelp.lib Ws2_32.lib)

