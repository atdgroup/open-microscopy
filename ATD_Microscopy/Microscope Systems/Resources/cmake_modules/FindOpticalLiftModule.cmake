INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(OPT_LIFT 		${HARDWARE_DIR_PATH}/OpticalLift/trunk/OpticalLift.c
					${HARDWARE_DIR_PATH}/OpticalLift/trunk/OpticalLift.h
					${HARDWARE_DIR_PATH}/OpticalLift/trunk/OpticalLiftUI.h
					${HARDWARE_DIR_PATH}/OpticalLift/trunk/OpticalLiftUI.uir
					${HARDWARE_DIR_PATH}/OpticalLift/trunk/ATD_OpticalLift_Dummy/ATD_OpticalLift_Dummy.c
					${HARDWARE_DIR_PATH}/OpticalLift/trunk/ATD_OpticalLift_Dummy/ATD_OpticalLift_Dummy.h
                    
					${HARDWARE_DIR_PATH}/OpticalLift/trunk/ATD_OpticalLift_A/ATD_OpticalLift_A.c
					${HARDWARE_DIR_PATH}/OpticalLift/trunk/ATD_OpticalLift_A/ATD_OpticalLift_A.h
					${HARDWARE_DIR_PATH}/OpticalLift/trunk/ATD_OpticalLift_A/ATD_OpticalLift_A_UI.h
					${HARDWARE_DIR_PATH}/OpticalLift/trunk/ATD_OpticalLift_A/ATD_OpticalLift_A_UI.uir
)

INCLUDE_DIRECTORIES(
					${HARDWARE_DIR_PATH}/OpticalLift/trunk		
					)
                    
SOURCE_GROUP("Hardware\\OpticalLift" FILES ${OPT_LIFT})