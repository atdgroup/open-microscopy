INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(LASER_SRCS 	
                        ${HARDWARE_DIR_PATH}/Laser/trunk/laser.c
                        ${HARDWARE_DIR_PATH}/Laser/trunk/laser.h
                        ${HARDWARE_DIR_PATH}/Laser/trunk/white_light_laser.c
						${HARDWARE_DIR_PATH}/Laser/trunk/white_light_laser.h
						${HARDWARE_DIR_PATH}/Laser/trunk/white_light_laser_ui.h
						${HARDWARE_DIR_PATH}/Laser/trunk/white_light_laser_ui.uir
                        ${HARDWARE_DIR_PATH}/Laser/trunk/dummy_laser.c
                        ${HARDWARE_DIR_PATH}/Laser/trunk/dummy_laser.h
)	

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/Laser/trunk)
                
ADD_DEFINITIONS(-D BUILD_MODULE_LASER)
				
SOURCE_GROUP("Hardware\\Laser" FILES ${LASER_SRCS})