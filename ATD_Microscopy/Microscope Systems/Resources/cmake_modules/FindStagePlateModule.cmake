INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH StagePlate ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(STAGE_PLATE_SRCS 		${HARDWARE_DIR_PATH}/StagePlate/trunk/StagePlateUI.uir
                            ${HARDWARE_DIR_PATH}/StagePlate/trunk/StagePlateUI.h
                            ${HARDWARE_DIR_PATH}/StagePlate/trunk/StagePlateUI.c
                            ${HARDWARE_DIR_PATH}/StagePlate/trunk/StagePlate.h
                            ${HARDWARE_DIR_PATH}/StagePlate/trunk/StagePlate.c
                            ${HARDWARE_DIR_PATH}/StagePlate/trunk/StagePlate_Drawing.c
							${HARDWARE_DIR_PATH}/StagePlate/trunk/StagePlate_PointSorting.c
)
            
INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/StagePlate/trunk)
                    
SOURCE_GROUP("Hardware\\StagePlate" FILES ${STAGE_PLATE_SRCS})
