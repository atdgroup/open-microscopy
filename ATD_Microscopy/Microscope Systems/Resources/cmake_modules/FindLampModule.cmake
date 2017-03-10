INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(LAMP_SRCS 		${HARDWARE_DIR_PATH}/Lamp/trunk/lamp.c
					${HARDWARE_DIR_PATH}/Lamp/trunk/lamp.h
                    ${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_Dummy/ATD_Lamp_Dummy.h
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_Dummy/ATD_Lamp_Dummy.c
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_Dummy/ATD_Lamp_Dummy_UI.h
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_Dummy/ATD_Lamp_Dummy_UI.uir
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_Dummy/ATD_Lamp_Dummy_UI.c
                    
                    ${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_A/ATD_LedLamp_A.c
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_A/ATD_LedLamp_A.h
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_A/ATD_LedLampUI_A.c
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_A/ATD_LedLampUI_A.h
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_A/ATD_LedLampUI_A.uir
                    
                    ${HARDWARE_DIR_PATH}/Lamp/trunk/OfflineImager/OfflineImager_Lamp.c
					${HARDWARE_DIR_PATH}/Lamp/trunk/OfflineImager/OfflineImager_Lamp.h
					${HARDWARE_DIR_PATH}/Lamp/trunk/OfflineImager/OfflineImager_LampUI.c
					${HARDWARE_DIR_PATH}/Lamp/trunk/OfflineImager/OfflineImager_LampUI.h
					${HARDWARE_DIR_PATH}/Lamp/trunk/OfflineImager/OfflineImager_LampUI.uir
                    
                    ${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_B/ATD_LedLamp_B.c
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_B/ATD_LedLamp_B.h
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_B/ATD_LedLampUI_B.h
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLamp_B/ATD_LedLampUI_B.uir
                    
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLighting_A/ATD_LedLighting_A.c
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLighting_A/ATD_LedLighting_A.h
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLighting_A/ATD_LedLightingUI_A.c
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLighting_A/ATD_LedLightingUI_A.h
					${HARDWARE_DIR_PATH}/Lamp/trunk/ATD_LedLighting_A/ATD_LedLightingUI_A.uir
)

IF (USE_90I_COMPONENTS)
SET(LAMP_SRCS ${LAMP_SRCS}

                ${HARDWARE_DIR_PATH}/Lamp/trunk/Nikon_Lamp_90i/Nikon_Lamp_90i.c
				${HARDWARE_DIR_PATH}/Lamp/trunk/Nikon_Lamp_90i/Nikon_Lamp_90i.h
				${HARDWARE_DIR_PATH}/Lamp/trunk/Nikon_Lamp_90i/90i_lamp_ui.uir
				${HARDWARE_DIR_PATH}/Lamp/trunk/Nikon_Lamp_90i/90i_lamp_ui.h
)
ENDIF (USE_90I_COMPONENTS)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/Lamp/trunk/)
                    
SOURCE_GROUP("Hardware\\Lamp" FILES ${LAMP_SRCS})