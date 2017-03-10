INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(OPT_PATH_SRCS 	${HARDWARE_DIR_PATH}/OpticalPath/trunk/OpticalPath.c
					${HARDWARE_DIR_PATH}/OpticalPath/trunk/OpticalPath.h
					${HARDWARE_DIR_PATH}/OpticalPath/trunk/OpticalPathUI.c
					${HARDWARE_DIR_PATH}/OpticalPath/trunk/OpticalPathUI.h
					${HARDWARE_DIR_PATH}/OpticalPath/trunk/OpticalPathUI.uir
					${HARDWARE_DIR_PATH}/OpticalPath/trunk/ATD_OpticalPath_Dummy/ATD_OpticalPath_Dummy.c
					${HARDWARE_DIR_PATH}/OpticalPath/trunk/ATD_OpticalPath_Dummy/ATD_OpticalPath_Dummy.h
                    
                    ${HARDWARE_DIR_PATH}/OpticalPath/trunk/ATD_OpticalPath_A/ATD_OpticalPath_A.c
					${HARDWARE_DIR_PATH}/OpticalPath/trunk/ATD_OpticalPath_A/ATD_OpticalPath_A.h
					
					${HARDWARE_DIR_PATH}/OpticalPath/trunk/ATD_OpticalPath_B/ATD_OpticalPath_B.c
					${HARDWARE_DIR_PATH}/OpticalPath/trunk/ATD_OpticalPath_B/ATD_OpticalPath_B.h
                    
					${HARDWARE_DIR_PATH}/StepperMotor/trunk/ATD_StepperMotor_A/ATD_StepperMotor_A.c
					${HARDWARE_DIR_PATH}/StepperMotor/trunk/ATD_StepperMotor_A/ATD_StepperMotor_A.h
					${HARDWARE_DIR_PATH}/StepperMotor/trunk/ATD_StepperMotor_A/ATD_StepperMotorUI_A.c
					${HARDWARE_DIR_PATH}/StepperMotor/trunk/ATD_StepperMotor_A/ATD_StepperMotorUI_A.h
					${HARDWARE_DIR_PATH}/StepperMotor/trunk/ATD_StepperMotor_A/ATD_StepperMotorUI_A.uir
)

IF (USE_90I_COMPONENTS)
SET(OPT_PATH_SRCS ${OPT_PATH_SRCS}

                ${HARDWARE_DIR_PATH}/OpticalPath/trunk/Nikon_OpticalPath_90i/Nikon_OpticalPath_90i.c
				${HARDWARE_DIR_PATH}/OpticalPath/trunk/Nikon_OpticalPath_90i/Nikon_OpticalPath_90i.h
)
ENDIF (USE_90I_COMPONENTS)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/OpticalPath/trunk/
                    ${HARDWARE_DIR_PATH}/StepperMotor/trunk)
                    
SOURCE_GROUP("Hardware\\OpticalPath" FILES ${OPT_PATH_SRCS})