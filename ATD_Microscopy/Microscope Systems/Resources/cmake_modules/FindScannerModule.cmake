INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(SCANNER_SRCS 	${HARDWARE_DIR_PATH}/scanner/trunk/scanner.c
					${HARDWARE_DIR_PATH}/scanner/trunk/scanner.h
					${HARDWARE_DIR_PATH}/scanner/trunk/ScannerUI.c
					${HARDWARE_DIR_PATH}/scanner/trunk/ScannerUI.h
					${HARDWARE_DIR_PATH}/scanner/trunk/ScannerUI.uir
					${HARDWARE_DIR_PATH}/scanner/trunk/ATD_Scanner_Dummy/ATD_Scanner_Dummy.c
					${HARDWARE_DIR_PATH}/scanner/trunk/ATD_Scanner_Dummy/ATD_Scanner_Dummy.h
                    
                    ${HARDWARE_DIR_PATH}/scanner/trunk/ATD_Scanner_A/ATD_Scanner_A.c
					${HARDWARE_DIR_PATH}/scanner/trunk/ATD_Scanner_A/ATD_Scanner_A.h
)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/Scanner/trunk)
                    
ADD_DEFINITIONS(-D BUILD_MODULE_SCANNER)
                    
SOURCE_GROUP("Hardware\\Scanner" FILES ${SCANNER_SRCS})