INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(ROBOT_SRCS 		${HARDWARE_DIR_PATH}/RobotInterface/trunk/RobotInterface.c
					${HARDWARE_DIR_PATH}/RobotInterface/trunk/RobotInterface.h
					${HARDWARE_DIR_PATH}/RobotInterface/trunk/RobotInterfaceUI.h
					${HARDWARE_DIR_PATH}/RobotInterface/trunk/RobotInterfaceUI.uir
					${HARDWARE_DIR_PATH}/RobotInterface/trunk/ATD_RobotInterface_Manual/ATD_RobotInterface_Manual.c
					${HARDWARE_DIR_PATH}/RobotInterface/trunk/ATD_RobotInterface_Manual/ATD_RobotInterface_Manual.h
					${HARDWARE_DIR_PATH}/RobotInterface/trunk/ATD_RobotInterface_Manual/ATD_RobotInterface_Manual_uir.uir
					${HARDWARE_DIR_PATH}/RobotInterface/trunk/ATD_RobotInterface_Manual/ATD_RobotInterface_Manual_uir.h
					${HARDWARE_DIR_PATH}/RobotInterface/trunk/ATD_RobotInterface_Piped/ATD_RobotInterface_Piped.c
					${HARDWARE_DIR_PATH}/RobotInterface/trunk/ATD_RobotInterface_Piped/ATD_RobotInterface_Piped.h
					${HARDWARE_DIR_PATH}/RobotInterface/trunk/ATD_RobotInterface_Piped/ATD_RobotInterface_Piped_uir.uir
					${HARDWARE_DIR_PATH}/RobotInterface/trunk/ATD_RobotInterface_Piped/ATD_RobotInterface_Piped_uir.h
)

INCLUDE_DIRECTORIES(
					${HARDWARE_DIR_PATH}/RobotInterface/trunk/	
					)

ADD_DEFINITIONS(-D BUILD_ROBOT_INTERFACE)
                    
SOURCE_GROUP("Hardware\\RobotInterface" FILES ${ROBOT_SRCS})
