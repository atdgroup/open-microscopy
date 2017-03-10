INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(SINGLE_RANGE_HW_DEVICE_SRCS 	
			${HARDWARE_DIR_PATH}/SingleRangeHardwareDevice/trunk/single_range_hardware_device.c
            ${HARDWARE_DIR_PATH}/SingleRangeHardwareDevice/trunk/single_range_hardware_device.h
            ${HARDWARE_DIR_PATH}/SingleRangeHardwareDevice/trunk/single_range_hardware_device_ui.c
            ${HARDWARE_DIR_PATH}/SingleRangeHardwareDevice/trunk/single_range_hardware_device_ui.h
            ${HARDWARE_DIR_PATH}/SingleRangeHardwareDevice/trunk/single_range_hardware_device_ui.uir
)

INCLUDE_DIRECTORIES(
					${HARDWARE_DIR_PATH}/SingleRangeHardwareDevice/trunk/				
					)
          
ADD_DEFINITIONS(-D BUILD_MODULE_SINGLE_RANGE_HW_DEVICE)
          
SOURCE_GROUP("Hardware\\SingleRangeHardwareDevice" FILES ${SINGLE_RANGE_HW_DEVICE_SRCS})