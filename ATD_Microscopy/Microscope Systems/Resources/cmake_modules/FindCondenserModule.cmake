INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(CONDENSER_SRCS 	${HARDWARE_DIR_PATH}/Condensers/trunk/condensers.c
					${HARDWARE_DIR_PATH}/Condensers/trunk/condensers.h
					${HARDWARE_DIR_PATH}/Condensers/trunk/condensers_ui.c
					${HARDWARE_DIR_PATH}/Condensers/trunk/condensers_ui.h
					${HARDWARE_DIR_PATH}/Condensers/trunk/condensers_ui.uir		
					${HARDWARE_DIR_PATH}/Condensers/trunk/manual_condensers.c
					${HARDWARE_DIR_PATH}/Condensers/trunk/manual_condensers.h					
)

IF (USE_90I_COMPONENTS)
SET(CONDENSER_SRCS ${CONDENSER_SRCS}

                ${HARDWARE_DIR_PATH}/Condensers/trunk/90i_condensers.c
				${HARDWARE_DIR_PATH}/Condensers/trunk/90i_condensers.h
)
ENDIF (USE_90I_COMPONENTS)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/Condensers/trunk/)
            
ADD_DEFINITIONS(-D BUILD_MODULE_CONDENSER)
            
SOURCE_GROUP("Hardware\\Condenser" FILES ${CONDENSER_SRCS})