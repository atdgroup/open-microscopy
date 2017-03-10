INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(LASERPOWERMONITOR_SRCS
                ${HARDWARE_DIR_PATH}/LaserPowerMonitor/trunk/LaserPowerMonitor.h
                ${HARDWARE_DIR_PATH}/LaserPowerMonitor/trunk/LaserPowerMonitor.c
				${HARDWARE_DIR_PATH}/LaserPowerMonitor/trunk/ATD_LaserPowerMonitor_A.h
				${HARDWARE_DIR_PATH}/LaserPowerMonitor/trunk/ATD_LaserPowerMonitor_A.c
                ${HARDWARE_DIR_PATH}/LaserPowerMonitor/trunk/ATD_LaserPowerMonitor_A_UI.uir
				${HARDWARE_DIR_PATH}/LaserPowerMonitor/trunk/ATD_LaserPowerMonitor_LOGP.h
				${HARDWARE_DIR_PATH}/LaserPowerMonitor/trunk/ATD_LaserPowerMonitor_LOGP.c
                ${HARDWARE_DIR_PATH}/LaserPowerMonitor/trunk/ATD_LaserPowerMonitor_LOGP_UI.uir
)

INCLUDE_DIRECTORIES(${HARDWARE_DIR_PATH}/LaserPowerMonitor/trunk/)
                  
ADD_DEFINITIONS(-D BUILD_MODULE_LASER_POWER_MONITOR)
                    
SOURCE_GROUP("Hardware\\LaserPowerMonitor" FILES ${LASERPOWERMONITOR_SRCS})