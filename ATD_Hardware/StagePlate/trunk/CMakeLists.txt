SET(STAGE_PLATE_SRCS 	${HARDWARE_DIR_PATH}/StagePlate/trunk/StagePlateUI.uir
                        ${HARDWARE_DIR_PATH}/StagePlate/trunk/StagePlateUI.h
                        ${HARDWARE_DIR_PATH}/StagePlate/trunk/StagePlateUI.c
                        ${HARDWARE_DIR_PATH}/StagePlate/trunk/StagePlate.h
                        ${HARDWARE_DIR_PATH}/StagePlate/trunk/StagePlate.c
)

# Make sure the compiler can find include files from our library.
INCLUDE_DIRECTORIES(
					${HARDWARE_DIR_PATH}/StagePlate/trunk/
					${HARDWARE_DIR_PATH}/StagePlate/trunk/ATD_CubeSlider_Dummy/
					${HARDWARE_DIR_PATH}/StagePlate/trunk/ATD_CubeSlider_A/
					#${HARDWARE_DIR_PATH}/CubeSlider/trunk/Nikon_CubeSlider_90i/
					)


ADD_EXECUTABLE (StagePlate WIN32 ${STAGE_PLATE_SRCS} main.c)
TARGET_LINK_LIBRARIES (StagePlate hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)