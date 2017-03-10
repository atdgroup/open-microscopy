INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(COARSE_ZDRIVE_SRCS 	${HARDWARE_DIR_PATH}/CoarseZDrive/trunk/ATD_CoarseZDrive_A.c
					    ${HARDWARE_DIR_PATH}/CoarseZDrive/trunk/ATD_CoarseZDrive_A.h
					    ${HARDWARE_DIR_PATH}/CoarseZDrive/trunk/ATD_CoarseZDrive_A_UI.h
					    ${HARDWARE_DIR_PATH}/CoarseZDrive/trunk/ATD_CoarseZDrive_A_UI.uir
)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/CoarseZDrive/trunk/)
                    
SOURCE_GROUP("Hardware\\CoarseZDrive" FILES ${COARSE_ZDRIVE_SRCS})