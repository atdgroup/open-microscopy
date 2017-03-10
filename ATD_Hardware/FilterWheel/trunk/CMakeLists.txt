SET(FILTERWHEEL_SRCS 	${HARDWARE_DIR_PATH}/FilterWheel/trunk/FilterWheelUI.uir
						${HARDWARE_DIR_PATH}/FilterWheel/trunk/FilterWheelUI.h
						${HARDWARE_DIR_PATH}/FilterWheel/trunk/FilterWheelUI.c
						${HARDWARE_DIR_PATH}/FilterWheel/trunk/FilterWheel.h
						${HARDWARE_DIR_PATH}/FilterWheel/trunk/FilterWheel.c
						${HARDWARE_DIR_PATH}/FilterWheel/trunk/ATD_FilterWheel_Dummy/ATD_FilterWheel_Dummy.h
						${HARDWARE_DIR_PATH}/FilterWheel/trunk/ATD_FilterWheel_Dummy/ATD_FilterWheel_Dummy.c
						${HARDWARE_DIR_PATH}/FilterWheel/trunk/ATD_FilterWheel_Thorlabs/ATD_FilterWheel_Thorlabs.c
						${HARDWARE_DIR_PATH}/FilterWheel/trunk/ATD_FilterWheel_Thorlabs/ATD_FilterWheel_Thorlabs.h
)

# Make sure the compiler can find include files from our library.
INCLUDE_DIRECTORIES(
					${HARDWARE_DIR_PATH}/FilterWheel/trunk/
					${HARDWARE_DIR_PATH}/FilterWheel/trunk/ATD_FilterWheel_Dummy/
					${HARDWARE_DIR_PATH}/FilterWheel/trunk/ATD_FilterWheel_Thorlabs/
					)


ADD_EXECUTABLE (FilterWheel WIN32 ${FILTERWHEEL_SRCS} main.c)
TARGET_LINK_LIBRARIES (FilterWheel hardware-dependancies-static ${FI_LIBS} ${CVI_LIBS} ${GCI_LIBS} setupapi.lib)