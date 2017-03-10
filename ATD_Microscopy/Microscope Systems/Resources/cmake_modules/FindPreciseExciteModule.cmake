INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(PRECISE_EXCITE_SRCS 	
			${HARDWARE_DIR_PATH}/PreciseExcite/trunk/precisExcite.c
            ${HARDWARE_DIR_PATH}/PreciseExcite/trunk/precisExcite.h
            ${HARDWARE_DIR_PATH}/PreciseExcite/trunk/precisExcite_ui.c
            ${HARDWARE_DIR_PATH}/PreciseExcite/trunk/precisExcite_ui.h
            ${HARDWARE_DIR_PATH}/PreciseExcite/trunk/precisExcite_ui.uir
)

INCLUDE_DIRECTORIES(
					${HARDWARE_DIR_PATH}/PreciseExcite/trunk/				
					)
   
ADD_DEFINITIONS(-D BUILD_MODULE_PRECISE_EXCITE)
   
SOURCE_GROUP("Hardware\\PreciseExcite" FILES ${PRECISE_EXCITE_SRCS})