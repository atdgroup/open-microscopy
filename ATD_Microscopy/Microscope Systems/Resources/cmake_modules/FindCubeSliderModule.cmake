INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(CUBE_SRCS 		${HARDWARE_DIR_PATH}/CubeSlider/trunk/CubeSlider.c
					${HARDWARE_DIR_PATH}/CubeSlider/trunk/CubeSlider.h
					${HARDWARE_DIR_PATH}/CubeSlider/trunk/CubeSliderUI.c
					${HARDWARE_DIR_PATH}/CubeSlider/trunk/CubeSliderUI.h
					${HARDWARE_DIR_PATH}/CubeSlider/trunk/CubeSliderUI.uir
					${HARDWARE_DIR_PATH}/CubeSlider/trunk/ATD_CubeSlider_A/ATD_CubeSlider_A.c
					${HARDWARE_DIR_PATH}/CubeSlider/trunk/ATD_CubeSlider_A/ATD_CubeSlider_A.h
					${HARDWARE_DIR_PATH}/CubeSlider/trunk/ATD_CubeSlider_Dummy/ATD_CubeSlider_Dummy.c
					${HARDWARE_DIR_PATH}/CubeSlider/trunk/ATD_CubeSlider_Dummy/ATD_CubeSlider_Dummy.h
)

IF (USE_90I_COMPONENTS)
SET(CUBE_SRCS ${CUBE_SRCS}

                ${HARDWARE_DIR_PATH}/CubeSlider/trunk/Nikon_CubeSlider_90i/Nikon_CubeSlider_90i.c
				${HARDWARE_DIR_PATH}/CubeSlider/trunk/Nikon_CubeSlider_90i/Nikon_CubeSlider_90i.h
)
ENDIF (USE_90I_COMPONENTS)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/CubeSlider/trunk)
                    
SOURCE_GROUP("Hardware\\Cubes" FILES ${CUBE_SRCS})