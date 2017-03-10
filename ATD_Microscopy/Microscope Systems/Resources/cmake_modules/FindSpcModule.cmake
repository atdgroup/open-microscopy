INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(SPC_SRCS 		${HARDWARE_DIR_PATH}/SPC/trunk/spc.c
					${HARDWARE_DIR_PATH}/SPC/trunk/spc.h
					${HARDWARE_DIR_PATH}/SPC/trunk/spc_ui.c
					${HARDWARE_DIR_PATH}/SPC/trunk/spc_ui.h
					${HARDWARE_DIR_PATH}/SPC/trunk/spc_ui.uir
                    ${HARDWARE_DIR_PATH}/SPC/trunk/spc_scope.c
                    ${HARDWARE_DIR_PATH}/SPC/trunk/spc_count_rates.c
                    ${HARDWARE_DIR_PATH}/SPC/trunk/spc_parameters.c
                    ${HARDWARE_DIR_PATH}/SPC/trunk/spc_stagescan.c
                    
                    ${HARDWARE_DIR_PATH}/SPC/trunk/flim.c
                    ${HARDWARE_DIR_PATH}/SPC/trunk/flim.h
                    ${HARDWARE_DIR_PATH}/SPC/trunk/flim_ui.h
                    ${HARDWARE_DIR_PATH}/SPC/trunk/flim_ui.uir   
)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/SPC/trunk/)
                  
ADD_DEFINITIONS(-D BUILD_MODULE_SPC)
                  
SOURCE_GROUP("Hardware\\Spc" FILES ${SPC_SRCS})