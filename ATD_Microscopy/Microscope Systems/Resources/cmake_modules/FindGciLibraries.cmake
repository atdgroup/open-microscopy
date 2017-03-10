INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (LIBRARIES_DIR_PATH UIModule ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Libraries NO_DEFAULT_PATH)

IF (LIBRARIES_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Gci Libraries")
ENDIF (LIBRARIES_DIR_PATH-NOTFOUND)

MAKE_WINDOWS_PATH(LIBRARIES_DIR_PATH)

SET(LIBRARY_SRCS 	${LIBRARIES_DIR_PATH}/Cvi\ Stubs/stubs.c
			        ${LIBRARIES_DIR_PATH}/DeviceListConfigModule/trunk/device_list.c
					${LIBRARIES_DIR_PATH}/DeviceListConfigModule/trunk/device_list.h
					${LIBRARIES_DIR_PATH}/DeviceListConfigModule/trunk/device_list_private.c
					${LIBRARIES_DIR_PATH}/DeviceListConfigModule/trunk/device_list_private.h
					${LIBRARIES_DIR_PATH}/DeviceListConfigModule/trunk/device_list_ui.c
					${LIBRARIES_DIR_PATH}/DeviceListConfigModule/trunk/device_list_ui.h
					${LIBRARIES_DIR_PATH}/DeviceListConfigModule/trunk/device_list_ui.uir
					${LIBRARIES_DIR_PATH}/dictionary/dictionary.c
					${LIBRARIES_DIR_PATH}/dictionary/dictionary.h
					${LIBRARIES_DIR_PATH}/GCI\ Registry/trunk/GL_CVIRegistry.c
					${LIBRARIES_DIR_PATH}/GCI\ Registry/trunk/GL_CVIRegistry.h
					${LIBRARIES_DIR_PATH}/GCI\ Utils/trunk/gci_types.h
					${LIBRARIES_DIR_PATH}/GCI\ Utils/trunk/gci_menu_utils.c
					${LIBRARIES_DIR_PATH}/GCI\ Utils/trunk/gci_menu_utils.h
					${LIBRARIES_DIR_PATH}/GCI\ Utils/trunk/gci_utils.c
					${LIBRARIES_DIR_PATH}/GCI\ Utils/trunk/gci_utils.h
					${LIBRARIES_DIR_PATH}/iniparser/src/iniparser.c
					${LIBRARIES_DIR_PATH}/iniparser/src/iniparser.h
					${LIBRARIES_DIR_PATH}/iniparser/src/strlib.c
					${LIBRARIES_DIR_PATH}/iniparser/src/strlib.h
					${LIBRARIES_DIR_PATH}/Password/trunk/password.c
					${LIBRARIES_DIR_PATH}/Password/trunk/password.h
					${LIBRARIES_DIR_PATH}/Password/trunk/password_ui.h
					${LIBRARIES_DIR_PATH}/Password/trunk/password_ui.uir
					${LIBRARIES_DIR_PATH}/Profile/trunk/profile.c
					${LIBRARIES_DIR_PATH}/Profile/trunk/profile.h
					${LIBRARIES_DIR_PATH}/Signals/trunk/signals.h
					${LIBRARIES_DIR_PATH}/Signals/trunk/signals.c
					${LIBRARIES_DIR_PATH}/Status/trunk/status.c
					${LIBRARIES_DIR_PATH}/Status/trunk/status.h
					${LIBRARIES_DIR_PATH}/Tooltips/tooltip.h
					${LIBRARIES_DIR_PATH}/Tooltips/tooltip.c
					${LIBRARIES_DIR_PATH}/UIModule/gci_ui_logger.c
					${LIBRARIES_DIR_PATH}/UIModule/gci_ui_module.c
					${LIBRARIES_DIR_PATH}/UIModule/gci_ui_logger.h
					${LIBRARIES_DIR_PATH}/UIModule/gci_ui_module.h
                    ${LIBRARIES_DIR_PATH}/MultipleMonitors/trunk/src/multiple-monitors.c
                    ${LIBRARIES_DIR_PATH}/MultipleMonitors/trunk/src/multiple-monitors.h
					${LIBRARIES_DIR_PATH}/FilePrefixSaveDialog/file_prefix_dialog.c
					${LIBRARIES_DIR_PATH}/FilePrefixSaveDialog/file_prefix_dialog.h
					${LIBRARIES_DIR_PATH}/FilePrefixSaveDialog/file_prefix_dialog_ui.h
					${LIBRARIES_DIR_PATH}/FilePrefixSaveDialog/file_prefix_dialog_ui.uir
					${LIBRARIES_DIR_PATH}/ExceptionHandler/ExceptionHandler.h
					${LIBRARIES_DIR_PATH}/ExceptionHandler/ExceptionHandler.c
					${LIBRARIES_DIR_PATH}/Filename\ Utils/trunk/FilenameUtils.c
					${LIBRARIES_DIR_PATH}/Filename\ Utils/trunk/FilenameUtils.h
                    ${LIBRARIES_DIR_PATH}/AsyncTimer/asynctmr.c
                    ${LIBRARIES_DIR_PATH}/AsyncTimer/asynctmr.h
                    ${LIBRARIES_DIR_PATH}/PolyAlgos/PolyAlgos.c
                    ${LIBRARIES_DIR_PATH}/PolyAlgos/PolyAlgos.h
					${LIBRARIES_DIR_PATH}/ThreadDebug/ThreadDebug.c
					${LIBRARIES_DIR_PATH}/ThreadDebug/ThreadDebug.h
)

INCLUDE_DIRECTORIES(

					${LIBRARIES_DIR_PATH}/ExcelAutomation/trunk
					${LIBRARIES_DIR_PATH}/FreeImageAlgorithms/trunk/include
					${LIBRARIES_DIR_PATH}/FreeImageIcs/trunk/include
					${LIBRARIES_DIR_PATH}/FreeImageIcs/trunk/libics-1.5.1
					${LIBRARIES_DIR_PATH}/String\ Utils/trunk/include
					${LIBRARIES_DIR_PATH}/ImageViewer/trunk/
					${LIBRARIES_DIR_PATH}/DeviceListConfigModule/trunk
					${LIBRARIES_DIR_PATH}/dictionary
					${LIBRARIES_DIR_PATH}/iniparser/src
					${LIBRARIES_DIR_PATH}/GCI\ Registry/trunk/
					${LIBRARIES_DIR_PATH}/GCI\ Utils/trunk/
					${LIBRARIES_DIR_PATH}/Password/trunk/
					${LIBRARIES_DIR_PATH}/Profile/trunk/
					${LIBRARIES_DIR_PATH}/Signals/trunk/
					${LIBRARIES_DIR_PATH}/Status/trunk
					${LIBRARIES_DIR_PATH}/Tooltips/
					${LIBRARIES_DIR_PATH}/UIModule/
                    ${LIBRARIES_DIR_PATH}/MultipleMonitors/trunk/src/
					${LIBRARIES_DIR_PATH}/FilePrefixSaveDialog/
					${LIBRARIES_DIR_PATH}/Filename\ Utils/trunk/
					${LIBRARIES_DIR_PATH}/XML\ utils/trunk/
					${LIBRARIES_DIR_PATH}/ExceptionHandler
                    ${LIBRARIES_DIR_PATH}/AsyncTimer
                    ${LIBRARIES_DIR_PATH}/PolyAlgos/
					${LIBRARIES_DIR_PATH}/ThreadDebug
					)
                    
SOURCE_GROUP(ATD_Libraries FILES ${LIBRARY_SRCS})