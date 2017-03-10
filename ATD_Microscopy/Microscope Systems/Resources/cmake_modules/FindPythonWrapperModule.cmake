INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(HW_PY_SRCS 		${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_python_wrappers.c
					${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_python_wrappers.h
                    
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_camera_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_cube_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_timelapse_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_stage_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_shutter_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_opticalpath_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_spc_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_lamp_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_scanner_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_zdrive_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_regionscan_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_cellfinding_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_background_correction_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_sw_autofocus_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_dialog_wrappers.c
                    ${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/microscope_batchcounter_wrappers.c
)


INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/HardwarePy/trunk/Hardware/Microscope/)
                    
SOURCE_GROUP("Hardware\\PythonWrappers" FILES ${HW_PY_SRCS})