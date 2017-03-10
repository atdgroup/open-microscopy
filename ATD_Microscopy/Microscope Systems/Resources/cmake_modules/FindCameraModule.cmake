INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(CAMERA_SRCS 	${HARDWARE_DIR_PATH}/Camera/trunk/camera/gci_camera.c
					${HARDWARE_DIR_PATH}/Camera/trunk/camera/gci_camera.h
					${HARDWARE_DIR_PATH}/Camera/trunk/camera/gci_camera_autoexposure.c
					${HARDWARE_DIR_PATH}/Camera/trunk/camera/gci_camera_callbacks.h
					${HARDWARE_DIR_PATH}/Camera/trunk/camera/gci_camera_ui.c	
					${HARDWARE_DIR_PATH}/Camera/trunk/uir_files/gci_camera_ui.uir	
					${HARDWARE_DIR_PATH}/Camera/trunk/uir_files/gci_camera_ui.h	
                    
					${HARDWARE_DIR_PATH}/Camera/trunk/dummy/gci_dummy_camera.c
					${HARDWARE_DIR_PATH}/Camera/trunk/dummy/gci_dummy_camera.h
					${HARDWARE_DIR_PATH}/Camera/trunk/dummy/gci_dummy_camera_ui.c
                    ${HARDWARE_DIR_PATH}/Camera/trunk/uir_files/gci_dummy_camera_ui.uir
)


IF (USE_CAMERA_DCAM)
SET(CAMERA_SRCS ${CAMERA_SRCS}

                ${HARDWARE_DIR_PATH}/Camera/trunk/dcam/gci_dcam_camera.c
				${HARDWARE_DIR_PATH}/Camera/trunk/dcam/gci_dcam_camera.h
				${HARDWARE_DIR_PATH}/Camera/trunk/dcam/gci_dcam_camera_lowlevel.c
				${HARDWARE_DIR_PATH}/Camera/trunk/dcam/gci_dcam_camera_lowlevel.h
				${HARDWARE_DIR_PATH}/Camera/trunk/dcam/gci_dcam_camera_settings.c
				${HARDWARE_DIR_PATH}/Camera/trunk/dcam/gci_dcam_camera_settings.h
				${HARDWARE_DIR_PATH}/Camera/trunk/dcam/gci_dcam_camera_properties.c
				${HARDWARE_DIR_PATH}/Camera/trunk/dcam/gci_dcam_camera_ui.c
				${HARDWARE_DIR_PATH}/Camera/trunk/dcam/gci_dcam_camera_ui.h
				${HARDWARE_DIR_PATH}/Camera/trunk/dcam/gci_dcam_camera_ui.uir
)
ENDIF (USE_CAMERA_DCAM)

IF (USE_CAMERA_JVC)
SET(CAMERA_SRCS ${CAMERA_SRCS}

                ${HARDWARE_DIR_PATH}/Camera/trunk/uir_files/gci_camera_jvc_ui.h
					${HARDWARE_DIR_PATH}/Camera/trunk/uir_files/gci_camera_jvc_ui.uir
					${HARDWARE_DIR_PATH}/Camera/trunk/jvc/gci_camera_jvc_ui.c
					${HARDWARE_DIR_PATH}/Camera/trunk/jvc/gci_jvc_camera.c
					${HARDWARE_DIR_PATH}/Camera/trunk/jvc/gci_jvc_camera.h
					${HARDWARE_DIR_PATH}/Camera/trunk/jvc/KYF75.c
					${HARDWARE_DIR_PATH}/Camera/trunk/jvc/KYF75.h
)
ENDIF (USE_CAMERA_JVC)


IF (USE_CAMERA_UPIX1311)
SET(CAMERA_SRCS ${CAMERA_SRCS}

				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC1311/gci_upix_uc1311_camera.c
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC1311/gci_upix_uc1311_camera.h
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC1311/gci_upix_uc1311_camera_lowlevel.c
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC1311/gci_upix_uc1311_camera_lowlevel.h
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC1311/gci_upix_uc1311_camera_ui.c
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC1311/gci_upix_uc1311_camera_ui.h
)

INCLUDE_DIRECTORIES(
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC1311/
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC1311/UC1311-SDK/
)

ENDIF (USE_CAMERA_UPIX1311)

IF (USE_CAMERA_UPIX3010)
SET(CAMERA_SRCS ${CAMERA_SRCS}

				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC3010/gci_upix_uc3010_camera.c
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC3010/gci_upix_uc3010_camera.h
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC3010/gci_upix_uc3010_camera_lowlevel.c
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC3010/gci_upix_uc3010_camera_lowlevel.h
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC3010/gci_upix_uc3010_camera_ui.c
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC3010/gci_upix_uc3010_camera_ui.h
                ${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC3010/gci_upix_uc3010_camera_ui.uir
)

INCLUDE_DIRECTORIES(
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC3010/
				${HARDWARE_DIR_PATH}/Camera/trunk/Upix-UC3010/UC3010-SDK/
)

ENDIF (USE_CAMERA_UPIX3010)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/Camera/trunk)
                    
SOURCE_GROUP("Hardware\\Camera" FILES ${CAMERA_SRCS})