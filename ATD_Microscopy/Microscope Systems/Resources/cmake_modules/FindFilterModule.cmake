INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(FILTER_SRCS 	
			${HARDWARE_DIR_PATH}/FilterSet/trunk/FilterSet.c
            ${HARDWARE_DIR_PATH}/FilterSet/trunk/FilterSet.h
            ${HARDWARE_DIR_PATH}/FilterSet/trunk/FilterSetUI.c
            ${HARDWARE_DIR_PATH}/FilterSet/trunk/FilterSetUI.h
            ${HARDWARE_DIR_PATH}/FilterSet/trunk/FilterSetUI.uir
                   
            ${HARDWARE_DIR_PATH}/FilterSet/trunk/FilterSet_Dummy/FilterSet_Dummy.c
            ${HARDWARE_DIR_PATH}/FilterSet/trunk/FilterSet_Dummy/FilterSet_Dummy.h
)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/FilterSet/trunk/)
      
ADD_DEFINITIONS(-D BUILD_MODULE_FILTER)
	  
SOURCE_GROUP("Hardware\\Filters" FILES ${FILTER_SRCS})