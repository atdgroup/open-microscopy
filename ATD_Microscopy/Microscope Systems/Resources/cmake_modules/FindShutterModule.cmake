INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(SHUTTER_SRCS 	${HARDWARE_DIR_PATH}/shutter/trunk/shutter.c
					${HARDWARE_DIR_PATH}/shutter/trunk/shutter.h
					${HARDWARE_DIR_PATH}/shutter/trunk/ShutterUI.h
					${HARDWARE_DIR_PATH}/shutter/trunk/ShutterUI.uir
					${HARDWARE_DIR_PATH}/shutter/trunk/ATD_Shutter_Dummy/ATD_Shutter_Dummy.c
					${HARDWARE_DIR_PATH}/shutter/trunk/ATD_Shutter_Dummy/ATD_Shutter_Dummy.h
                    
                    ${HARDWARE_DIR_PATH}/shutter/trunk/ATD_Shutter_A/ATD_Shutter_A.c
					${HARDWARE_DIR_PATH}/shutter/trunk/ATD_Shutter_A/ATD_Shutter_A.h
                    
                    ${HARDWARE_DIR_PATH}/shutter/trunk/OfflineImager_Shutter/OfflineImager_Shutter.c
					${HARDWARE_DIR_PATH}/shutter/trunk/OfflineImager_Shutter/OfflineImager_Shutter.h
)

IF (USE_90I_COMPONENTS)
SET(SHUTTER_SRCS ${SHUTTER_SRCS}

                ${HARDWARE_DIR_PATH}/Shutter/trunk/Nikon_Shutter_90i/Nikon_Shutter_90i.c
				${HARDWARE_DIR_PATH}/Shutter/trunk/Nikon_Shutter_90i/Nikon_Shutter_90i.h
)
ENDIF (USE_90I_COMPONENTS)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/Shutter/trunk ${HARDWARE_DIR_PATH}/Shutter/trunk/Nikon_Shutter_90i)
                    
SOURCE_GROUP("Hardware\\Shutter" FILES ${SHUTTER_SRCS})
