INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(STAGE_SRCS 		${HARDWARE_DIR_PATH}/stage/trunk/stage/stage.c
					${HARDWARE_DIR_PATH}/stage/trunk/stage/stage.h
					${HARDWARE_DIR_PATH}/stage/trunk/stage/stage_ui.c
					${HARDWARE_DIR_PATH}/stage/trunk/stage/stage_ui.h
					${HARDWARE_DIR_PATH}/stage/trunk/stage/stage_ui.uir
					${HARDWARE_DIR_PATH}/stage/trunk/dummy/dummy_stage.c
					${HARDWARE_DIR_PATH}/stage/trunk/dummy/dummy_stage.h
					${HARDWARE_DIR_PATH}/stage/trunk/RS232CorvusXY/RS232Corvus_Communication.c
					${HARDWARE_DIR_PATH}/stage/trunk/RS232CorvusXY/RS232Corvus_Communication.h
					${HARDWARE_DIR_PATH}/stage/trunk/RS232CorvusXY/RS232CorvusXY.c
					${HARDWARE_DIR_PATH}/stage/trunk/RS232CorvusXY/RS232CorvusXY.h
					${HARDWARE_DIR_PATH}/stage/trunk/RS232CorvusXY/RS232CorvusXY_Callbacks.c
					${HARDWARE_DIR_PATH}/stage/trunk/RS232CorvusXY/RS232CorvusXY_UserInterface.h
					${HARDWARE_DIR_PATH}/stage/trunk/RS232CorvusXY/RS232CorvusXY_UserInterface.uir
)

IF (USE_STAGE_LSTEP)
SET(STAGE_SRCS ${STAGE_SRCS}

                ${HARDWARE_DIR_PATH}/stage/trunk/LStep/LStep4.h
				${HARDWARE_DIR_PATH}/stage/trunk/LStep/LStepXY.c
				${HARDWARE_DIR_PATH}/stage/trunk/LStep/LStepXY.h
				${HARDWARE_DIR_PATH}/stage/trunk/LStep/LStep_Callbacks.c
				${HARDWARE_DIR_PATH}/stage/trunk/LStep/LStep_UserInterface.h
				${HARDWARE_DIR_PATH}/stage/trunk/LStep/LStep_UserInterface.uir
)
ENDIF (USE_STAGE_LSTEP)

            
INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/Stage/trunk)
                    
SOURCE_GROUP("Hardware\\Stage" FILES ${STAGE_SRCS})
