INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(PMT_SRCS 	
			${HARDWARE_DIR_PATH}/PmtSet/trunk/PmtSet.c
            ${HARDWARE_DIR_PATH}/PmtSet/trunk/PmtSet.h
            ${HARDWARE_DIR_PATH}/PmtSet/trunk/PmtSetUI.c
            ${HARDWARE_DIR_PATH}/PmtSet/trunk/PmtSetUI.h
            ${HARDWARE_DIR_PATH}/PmtSet/trunk/PmtSetUI.uir
                   
            ${HARDWARE_DIR_PATH}/PmtSet/trunk/PmtSet_Dummy/PmtSet_Dummy.c
            ${HARDWARE_DIR_PATH}/PmtSet/trunk/PmtSet_Dummy/PmtSet_Dummy.h
)

INCLUDE_DIRECTORIES(
					${HARDWARE_DIR_PATH}/PmtSet/trunk/
					${HARDWARE_DIR_PATH}/PmtSet/trunk/PmtSet_Dummy				
					)
              
ADD_DEFINITIONS(-D BUILD_MODULE_PMT)
			  
SOURCE_GROUP("Hardware\\PMT" FILES ${PMT_SRCS})