INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(ZDRIVE_SRCS 	${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ZDrive.c
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ZDrive.h
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ZDriveUI.h
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ZDriveUI.uir
                    
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ATD_ZDrive_Dummy/ATD_ZDrive_Dummy.c
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ATD_ZDrive_Dummy/ATD_ZDrive_Dummy.c
                    
                    ${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ATD_ZDrive_A/ATD_ZDrive_A.c
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ATD_ZDrive_A/ATD_ZDrive_A.h
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ATD_ZDrive_A/ATD_ZDriveAutoFocus_A.c
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ATD_ZDrive_A/ATD_ZDriveAutoFocus_A.h
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ATD_ZDrive_A/ATD_ZDriveAutoFocus_A.uir
					
                    ${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ATD_ZDrive_B/ATD_ZDrive_B.c
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ATD_ZDrive_B/ATD_ZDriveSetup_B.c
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ATD_ZDrive_B/ATD_ZDrive_B.h
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ATD_ZDrive_B/ATD_ZDriveAutoFocus_B.c
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ATD_ZDrive_B/ATD_ZDriveAutoFocus_B.h
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/ATD_ZDrive_B/ATD_ZDriveAutoFocus_B.uir			
                    
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/PRIOR_ZDrive_NanoScanZ/prior_piezo_focus.c
					${HARDWARE_DIR_PATH}/Z\ Drive/trunk/PRIOR_ZDrive_NanoScanZ/prior_piezo_focus.h
)

IF (USE_90I_COMPONENTS)
SET(ZDRIVE_SRCS ${ZDRIVE_SRCS}

                ${HARDWARE_DIR_PATH}/Z\ Drive/trunk/90i_ZDrive/90i_z_drive.c
				${HARDWARE_DIR_PATH}/Z\ Drive/trunk/90i_ZDrive/90i_z_drive.h
)
ENDIF (USE_90I_COMPONENTS)

IF (USE_STAGE_LSTEP)
SET(ZDRIVE_SRCS ${ZDRIVE_SRCS}

            ${HARDWARE_DIR_PATH}/Z\ Drive/trunk/Marzhauser_ZDrive_LStep/Marzhauser_ZDrive_LStep.h
			${HARDWARE_DIR_PATH}/Z\ Drive/trunk/Marzhauser_ZDrive_LStep/Marzhauser_ZDrive_LStep.c
)
ENDIF (USE_STAGE_LSTEP)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/Z\ Drive/trunk/)
                    
SOURCE_GROUP("Hardware\\ZDrive" FILES ${ZDRIVE_SRCS})