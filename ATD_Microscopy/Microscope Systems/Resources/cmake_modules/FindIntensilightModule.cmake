INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(INTENSILIGHT_SRCS 	${HARDWARE_DIR_PATH}/Intensilight/trunk/intensilight.c
						${HARDWARE_DIR_PATH}/Intensilight/trunk/intensilight.h
						${HARDWARE_DIR_PATH}/Intensilight/trunk/intensilight_ui.c
						${HARDWARE_DIR_PATH}/Intensilight/trunk/intensilight_ui.h
                        ${HARDWARE_DIR_PATH}/Intensilight/trunk/intensilight_ui.uir
)

INCLUDE_DIRECTORIES(
					${HARDWARE_DIR_PATH}/Intensilight/trunk/				
					)
   
ADD_DEFINITIONS(-D BUILD_MODULE_INTENSILIGHT)
   
SOURCE_GROUP("Hardware\\Intensilight" FILES ${INTENSILIGHT_SRCS})