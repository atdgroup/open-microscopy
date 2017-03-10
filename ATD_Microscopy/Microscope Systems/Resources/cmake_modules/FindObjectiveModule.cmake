INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(OBJ_SRCS 		${HARDWARE_DIR_PATH}/Objectives/trunk/objectives.c
					${HARDWARE_DIR_PATH}/Objectives/trunk/objectives.h
					${HARDWARE_DIR_PATH}/Objectives/trunk/objectives_private.h
					${HARDWARE_DIR_PATH}/Objectives/trunk/ObjectivesUI.c
					${HARDWARE_DIR_PATH}/Objectives/trunk/ObjectivesUI.h
					${HARDWARE_DIR_PATH}/Objectives/trunk/ObjectivesUI.uir
                    
					${HARDWARE_DIR_PATH}/Objectives/trunk/ATD_Objectives_Dummy/ATD_Objectives_Dummy.c
					${HARDWARE_DIR_PATH}/Objectives/trunk/ATD_Objectives_Dummy/ATD_Objectives_Dummy.h
                    
                    ${HARDWARE_DIR_PATH}/Objectives/trunk/OfflineImager_Objectives/OfflineImager_Objectives.c
					${HARDWARE_DIR_PATH}/Objectives/trunk/OfflineImager_Objectives/OfflineImager_Objectives.h
)

IF (USE_90I_COMPONENTS)
SET(OBJ_SRCS ${OBJ_SRCS}

                ${HARDWARE_DIR_PATH}/Objectives/trunk/Nikon_Objectives_90i/90i_objectives.c
				${HARDWARE_DIR_PATH}/Objectives/trunk/Nikon_Objectives_90i/90i_objectives.h
)
ENDIF (USE_90I_COMPONENTS)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/Objectives/trunk)
                    
SOURCE_GROUP("Hardware\\Objective" FILES ${OBJ_SRCS})