INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (MS_MODULES_DIR_PATH RegionScan ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../Microscope\ Modules NO_DEFAULT_PATH)

IF (MS_MODULES_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Microscope Modules")
ENDIF (MS_MODULES_DIR_PATH-NOTFOUND)

MAKE_WINDOWS_PATH(MS_MODULES_DIR_PATH)

SET(MS_MODULES		${MS_MODULES_DIR_PATH}/Object\ Finding/cell_finding_ui.h
					${MS_MODULES_DIR_PATH}/Object\ Finding/celllinklist.h
					${MS_MODULES_DIR_PATH}/Object\ Finding/cellmap_ui.h
					${MS_MODULES_DIR_PATH}/Object\ Finding/cell_finding_ui.uir
					${MS_MODULES_DIR_PATH}/Object\ Finding/cell_finding_ui.h
                    ${MS_MODULES_DIR_PATH}/Object\ Finding/cell.c
					${MS_MODULES_DIR_PATH}/Object\ Finding/cellmap.c
					${MS_MODULES_DIR_PATH}/Object\ Finding/cell_finding.c
                    ${MS_MODULES_DIR_PATH}/Object\ Finding/cell.h 
                    ${MS_MODULES_DIR_PATH}/Object\ Finding/cell_finding.h
                    ${MS_MODULES_DIR_PATH}/Object\ Finding/cell_info_ui.h
                    ${MS_MODULES_DIR_PATH}/Object\ Finding/cellmap.h
                    ${MS_MODULES_DIR_PATH}/Object\ Finding/celllinklist.c

                    ${MS_MODULES_DIR_PATH}/RegionScan/trunk/RegionScan.c
                    ${MS_MODULES_DIR_PATH}/RegionScan/trunk/RegionScan.h
                    ${MS_MODULES_DIR_PATH}/RegionScan/trunk/RegionScan_ui.h
                    ${MS_MODULES_DIR_PATH}/RegionScan/trunk/RegionScan_ui.uir
					
					${MS_MODULES_DIR_PATH}/WellPlateDefiner/trunk/WellPlateDefiner.c
                    ${MS_MODULES_DIR_PATH}/WellPlateDefiner/trunk/WellPlateDefiner.h
                    ${MS_MODULES_DIR_PATH}/WellPlateDefiner/trunk/WellPlateDefiner_ui.h
                    ${MS_MODULES_DIR_PATH}/WellPlateDefiner/trunk/WellPlateDefiner_ui.uir
					
					${MS_MODULES_DIR_PATH}/Background\ Correction/trunk/background_correction.c
                    ${MS_MODULES_DIR_PATH}/Background\ Correction/trunk/background_correction.h
                    ${MS_MODULES_DIR_PATH}/Background\ Correction/trunk/background_correction_ui.h
                    ${MS_MODULES_DIR_PATH}/Background\ Correction/trunk/background_correction_ui.uir
					
					${MS_MODULES_DIR_PATH}/Optical\ Calibration/trunk/optical_calibration.c
                    ${MS_MODULES_DIR_PATH}/Optical\ Calibration/trunk/optical_calibration.h
                    ${MS_MODULES_DIR_PATH}/Optical\ Calibration/trunk/optical_calibration_ui.c
                    ${MS_MODULES_DIR_PATH}/Optical\ Calibration/trunk/optical_calibration_uir.h
                    ${MS_MODULES_DIR_PATH}/Optical\ Calibration/trunk/optical_calibration_uir.uir
					
					${MS_MODULES_DIR_PATH}/StageScan/trunk/StageScan.c
                    ${MS_MODULES_DIR_PATH}/StageScan/trunk/StageScan.h
                    ${MS_MODULES_DIR_PATH}/StageScan/trunk/StageScan_ui.h
                    ${MS_MODULES_DIR_PATH}/StageScan/trunk/StageScan_ui.uir
					${MS_MODULES_DIR_PATH}/StageScan/trunk/xy_scan.bmp
                    ${MS_MODULES_DIR_PATH}/StageScan/trunk/yx_scan.bmp
                    
					${MS_MODULES_DIR_PATH}/Focus/trunk/focus.c
                    ${MS_MODULES_DIR_PATH}/Focus/trunk/focus.h
                    ${MS_MODULES_DIR_PATH}/Focus/trunk/focus_ui.h
                    ${MS_MODULES_DIR_PATH}/Focus/trunk/focus_ui.uir
					
					${MS_MODULES_DIR_PATH}/Python\ Automation/trunk/AutomationEditor.c
                    ${MS_MODULES_DIR_PATH}/Python\ Automation/trunk/AutomationEditor.h
                    ${MS_MODULES_DIR_PATH}/Python\ Automation/trunk/PythonEditor.h 
                    ${MS_MODULES_DIR_PATH}/Python\ Automation/trunk/PythonEditor.uir 
                    ${MS_MODULES_DIR_PATH}/Python\ Automation/trunk/gci_python_wrappers.c
                    ${MS_MODULES_DIR_PATH}/Python\ Automation/trunk/gci_python_wrappers.h
					
					${MS_MODULES_DIR_PATH}/SWAutoFocus/AutoFocus.c
                    ${MS_MODULES_DIR_PATH}/SWAutoFocus/AutoFocus.h
                    ${MS_MODULES_DIR_PATH}/SWAutoFocus/AutoFocus_ui.h
                    ${MS_MODULES_DIR_PATH}/SWAutoFocus/AutoFocus_ui.uir

                    ${MS_MODULES_DIR_PATH}/Mosaic/trunk/mosaic.c
                    ${MS_MODULES_DIR_PATH}/Mosaic/trunk/mosaic.h

                    ${MS_MODULES_DIR_PATH}/RegionOfInterest/trunk/RegionOfInterest.c
                    ${MS_MODULES_DIR_PATH}/RegionOfInterest/trunk/RegionOfInterest.h
                    ${MS_MODULES_DIR_PATH}/RegionOfInterest/trunk/RegionOfInterest_ui.h
                    ${MS_MODULES_DIR_PATH}/RegionOfInterest/trunk/RegionOfInterest_ui.uir

#                    ${MS_MODULES_DIR_PATH}/Object\ Finding/comets.c
#                    ${MS_MODULES_DIR_PATH}/Object\ Finding/comets.h
#                    ${MS_MODULES_DIR_PATH}/Object\ Finding/comets_ui.h
#                    ${MS_MODULES_DIR_PATH}/Object\ Finding/comets_ui.uir

                    ${MS_MODULES_DIR_PATH}/Timelapse/trunk/TimeLapse.c
                    ${MS_MODULES_DIR_PATH}/Timelapse/trunk/TimeLapse.h
					${MS_MODULES_DIR_PATH}/Timelapse/trunk/TimeLapse_ui.c
                    ${MS_MODULES_DIR_PATH}/Timelapse/trunk/TimeLapse_ui.h
                    ${MS_MODULES_DIR_PATH}/Timelapse/trunk/TimeLapse_ui.uir
					${MS_MODULES_DIR_PATH}/Timelapse/trunk/WizardUI.uir
					${MS_MODULES_DIR_PATH}/Timelapse/trunk/WizardUI.h
					${MS_MODULES_DIR_PATH}/Timelapse/trunk/Wizard.c
					${MS_MODULES_DIR_PATH}/Timelapse/trunk/Wizard.h
                    ${MS_MODULES_DIR_PATH}/Timelapse/trunk/TimeLapse-PointWizardImportUI.uir
                    ${MS_MODULES_DIR_PATH}/Timelapse/trunk/TimeLapse-PointWizardImportUI.h
                    ${MS_MODULES_DIR_PATH}/Timelapse/trunk/Timelapse-PointWizardImport.c
					${MS_MODULES_DIR_PATH}/RealTime\ Overview/trunk/realtime_overview.c
                    ${MS_MODULES_DIR_PATH}/RealTime\ Overview/trunk/realtime_overview.h
)

INCLUDE_DIRECTORIES(

					${MS_MODULES_DIR_PATH}/Object\ Finding/
                    ${MS_MODULES_DIR_PATH}/RegionScan/trunk/
                    ${MS_MODULES_DIR_PATH}/WellPlateDefiner/trunk/
                    ${MS_MODULES_DIR_PATH}/Background\ Correction/trunk/
                    ${MS_MODULES_DIR_PATH}/Optical\ Calibration/trunk/
                    ${MS_MODULES_DIR_PATH}/StageScan/trunk/
                    ${MS_MODULES_DIR_PATH}/Focus/trunk/
                    ${MS_MODULES_DIR_PATH}/Python\ Automation/trunk/
                    ${MS_MODULES_DIR_PATH}/SWAutoFocus/
                    ${MS_MODULES_DIR_PATH}/RegionOfInterest/trunk/
                    ${MS_MODULES_DIR_PATH}/Timelapse/trunk/
					${MS_MODULES_DIR_PATH}/Mosaic/trunk/
					${MS_MODULES_DIR_PATH}/RealTime\ Overview/trunk
					)
                    
SOURCE_GROUP("Microscope Modules" FILES ${MS_MODULES})