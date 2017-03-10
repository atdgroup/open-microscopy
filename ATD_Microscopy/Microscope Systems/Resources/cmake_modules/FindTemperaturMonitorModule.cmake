INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (HARDWARE_DIR_PATH Shutter ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Hardware NO_DEFAULT_PATH)

IF (HARDWARE_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Hardware")
ENDIF (HARDWARE_DIR_PATH-NOTFOUND)

SET(TEMP_MON 		${HARDWARE_DIR_PATH}/TemperatureMonitor/trunk/TemperatureMonitor.c
					${HARDWARE_DIR_PATH}/TemperatureMonitor/trunk/TemperatureMonitor.h
					${HARDWARE_DIR_PATH}/TemperatureMonitor/trunk/TemperatureMonitorUI.h
					${HARDWARE_DIR_PATH}/TemperatureMonitor/trunk/TemperatureMonitorUI.uir
					${HARDWARE_DIR_PATH}/TemperatureMonitor/trunk/ATD_TemperatureMonitor_Dummy/ATD_TemperatureMonitor_Dummy.c
					${HARDWARE_DIR_PATH}/TemperatureMonitor/trunk/ATD_TemperatureMonitor_Dummy/ATD_TemperatureMonitor_Dummy.h
                    
					${HARDWARE_DIR_PATH}/TemperatureMonitor/trunk/ATD_TemperatureMonitor_A/ATD_TemperatureMonitor_A.c
					${HARDWARE_DIR_PATH}/TemperatureMonitor/trunk/ATD_TemperatureMonitor_A/ATD_TemperatureMonitor_A.h
					${HARDWARE_DIR_PATH}/TemperatureMonitor/trunk/ATD_TemperatureMonitor_A/ATD_TemperatureMonitor_A_UI.h
		
)

INCLUDE_DIRECTORIES(
					${HARDWARE_DIR_PATH}/TemperatureMonitor/trunk/	
					)
                    
SOURCE_GROUP("Hardware\\TemperatureMonitor" FILES ${TEMP_MON})
