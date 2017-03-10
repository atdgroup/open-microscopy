INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(I2C_SRCS 		${HARDWARE_DIR_PATH}/ATD_UsbInterface/trunk/ATD_UsbInterface_A.c
					${HARDWARE_DIR_PATH}/ATD_UsbInterface/trunk/ATD_UsbInterface_A.h
                    ${HARDWARE_DIR_PATH}/FTDI/trunk/FTDI_Utils.c
					${HARDWARE_DIR_PATH}/FTDI/trunk/FTDI_Utils.h
)



INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/ATD_UsbInterface/trunk
                    ${HARDWARE_DIR_PATH}/FTDI/trunk/)
                    
SOURCE_GROUP("Hardware\\I2C" FILES ${I2C_SRCS})