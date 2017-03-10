INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(ANALYZER_SRCS 	${HARDWARE_DIR_PATH}/Analyzer/trunk/analyzer.c
					${HARDWARE_DIR_PATH}/Analyzer/trunk/analyzer.h
					${HARDWARE_DIR_PATH}/Analyzer/trunk/analyzer_ui.c
					${HARDWARE_DIR_PATH}/Analyzer/trunk/analyzer_ui.h
					${HARDWARE_DIR_PATH}/Analyzer/trunk/analyzer_ui.uir
					
					
)

IF (USE_90I_COMPONENTS)
SET(ANALYZER_SRCS ${ANALYZER_SRCS}

                ${HARDWARE_DIR_PATH}/Analyzer/trunk/90i_analyzer.c
				${HARDWARE_DIR_PATH}/Analyzer/trunk/90i_analyzer.h
)
ENDIF (USE_90I_COMPONENTS)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/Analyzer/trunk/)
            
ADD_DEFINITIONS(-D BUILD_MODULE_ANALYZER)
            
SOURCE_GROUP("Hardware\\Analyzer" FILES ${ANALYZER_SRCS})