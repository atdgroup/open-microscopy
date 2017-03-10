INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(PS_SRCS			${HARDWARE_DIR_PATH}/Power\ Switch/trunk/PowerSwitch.c
					${HARDWARE_DIR_PATH}/Power\ Switch/trunk/PowerSwitch.h
					${HARDWARE_DIR_PATH}/Power\ Switch/trunk/ATD_PowerSwitch_A/ATD_PowerSwitch_A.c
					${HARDWARE_DIR_PATH}/Power\ Switch/trunk/ATD_PowerSwitch_A/ATD_PowerSwitch_A.h
					${HARDWARE_DIR_PATH}/Power\ Switch/trunk/ATD_PowerSwitch_A/ATD_PowerSwitch_A_UI.h
					${HARDWARE_DIR_PATH}/Power\ Switch/trunk/ATD_PowerSwitch_A/ATD_PowerSwitch_A_UI.uir
                    
                    ${HARDWARE_DIR_PATH}/Power\ Switch/trunk/ATD_PowerSwitch_B/ATD_PowerSwitch_B.c
					${HARDWARE_DIR_PATH}/Power\ Switch/trunk/ATD_PowerSwitch_B/ATD_PowerSwitch_B.h
					${HARDWARE_DIR_PATH}/Power\ Switch/trunk/ATD_PowerSwitch_B/ATD_PowerSwitch_B_UI.h
					${HARDWARE_DIR_PATH}/Power\ Switch/trunk/ATD_PowerSwitch_B/ATD_PowerSwitch_B_UI.uir
                    
                    ${HARDWARE_DIR_PATH}/Power\ Switch/trunk/OfflineImager_PowerSwitch/OfflineImager_PowerSwitch.c
					${HARDWARE_DIR_PATH}/Power\ Switch/trunk/OfflineImager_PowerSwitch/OfflineImager_PowerSwitch.h
					${HARDWARE_DIR_PATH}/Power\ Switch/trunk/OfflineImager_PowerSwitch/OfflineImager_PowerSwitch_UI.h
					${HARDWARE_DIR_PATH}/Power\ Switch/trunk/OfflineImager_PowerSwitch/OfflineImager_PowerSwitch_UI.uir
)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/Power\ Switch/trunk)
                    
SOURCE_GROUP("Hardware\\Power" FILES ${PS_SRCS})
