INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (CVI_DIR_PATH cvi.exe FORCE "C://Program\ Files\ (x86)//National\ Instruments//CVI80")

IF (CVI_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find cvi")
ENDIF (CVI_DIR_PATH-NOTFOUND)

SET(CVI_TOOLLIB_SRCS 
					 ${CVI_DIR_PATH}/toolslib/toolbox/toolbox.c
					 ${CVI_DIR_PATH}/toolslib/custctrl/easytab.c
					 ${CVI_DIR_PATH}/toolslib/custctrl/easytab.c
					 ${CVI_DIR_PATH}/toolslib/custctrl/pathctrl.c
					 ${CVI_DIR_PATH}/toolslib/custctrl/pwctrl.c
                     ${CVI_DIR_PATH}/toolslib/custctrl/radioGroup.c
)

INCLUDE_DIRECTORIES(${CVI_TOOLBOX_INCLUDE_PATH}
                    ${CVI_DIR_PATH}/include
					${CVI_DIR_PATH}/toolslib/toolbox
					${CVI_DIR_PATH}/toolslib/custctrl
					${CVI_DIR_PATH}/toolslib/reportgen
					${CVI_DIR_PATH}/toolslib/printing
					${CVI_DIR_PATH}/toolslib/localui
					${CVI_DIR_PATH}/toolslib/datasock
					${CVI_DIR_PATH}/toolslib/cvirtsup
					${CVI_DIR_PATH}/toolslib/activex
					${CVI_DIR_PATH}/toolslib/activex/excel)
                    
SOURCE_GROUP(Toolslib FILES ${CVI_TOOLLIB_SRCS})