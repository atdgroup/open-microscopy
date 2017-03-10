INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(BATCH_COUNTER_SRCS 		${HARDWARE_DIR_PATH}/BatchCounter/trunk/ATD_BatchCounter_A1.c
							${HARDWARE_DIR_PATH}/BatchCounter/trunk/ATD_BatchCounter_A1.h
							${HARDWARE_DIR_PATH}/BatchCounter/trunk/ATD_BatchCounter_A1_UI.h
							${HARDWARE_DIR_PATH}/BatchCounter/trunk/ATD_BatchCounter_A1_UI.uir
							
)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/BatchCounter/trunk)
                    
ADD_DEFINITIONS(-D BUILD_MODULE_BATCHCOUNTER)
                
SOURCE_GROUP("Hardware\\BatchCounter" FILES ${BATCH_COUNTER_SRCS})