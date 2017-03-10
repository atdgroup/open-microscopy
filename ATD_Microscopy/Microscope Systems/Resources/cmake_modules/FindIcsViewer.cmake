INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (ICSVIEWER_DIR_PATH trunk/trunk ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Libraries/IcsViewer
                                                  NO_DEFAULT_PATH)

IF (ICSVIEWER_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find IcsViewer")
ENDIF (ICSVIEWER_DIR_PATH-NOTFOUND)

MAKE_WINDOWS_PATH(ICSVIEWER_DIR_PATH)

SET(ICSVIEWER_SRCS 	${ICSVIEWER_DIR_PATH}/trunk/Fortify/fortify.c
					${ICSVIEWER_DIR_PATH}/trunk/Fortify/fortify.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_3d.c
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_3d.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_com_utils.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_droptarget.c
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_event_handlers.c
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_ics3d.c
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_ics3d.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_plugin_menu.c
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_plugin_menu.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_private.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_signals.c
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_signals.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_tools.c
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_tools.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_uir.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_uir.uir
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_window.c
					${ICSVIEWER_DIR_PATH}/trunk/Shell/icsviewer_window.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/ImageViewer_Drawing.c
					${ICSVIEWER_DIR_PATH}/trunk/Shell/ImageViewer_Drawing.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/ImageViewer_Drawing.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/TWAIN/EZTWAIN.C
					${ICSVIEWER_DIR_PATH}/trunk/Shell/TWAIN/EZTWAIN.h
					${ICSVIEWER_DIR_PATH}/trunk/Shell/TWAIN/TWAIN.H	
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/background_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/background_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/counter_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/counter_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/false_colour_dialog.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/fft_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/fft_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/grid_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/grid_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/histogram_equalisation_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/histogram_equalisation_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/histogram_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/histogram_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/linearscale_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/linearscale_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/metadata_editable_cell_tree.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/metadata_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/metadata_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/palette_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/palette_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/palettebar_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/palettebar_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/profile_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/profile_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/resample_menu_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/resample_menu_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/binning_menu_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/binning_menu_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/rotate_menu_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/rotate_menu_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/save_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/save_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/scalebar_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/scalebar_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/screenshot_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/screenshot_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/titlebar_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/titlebar_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/twain_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/twain_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/streamdevice_plugin.c
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/streamdevice_plugin.h
					${ICSVIEWER_DIR_PATH}/trunk/Plugins/directshow_wrapper.c
                    ${ICSVIEWER_DIR_PATH}/trunk/Plugins/directshow_wrapper.h    
					${ICSVIEWER_DIR_PATH}/trunk/Tools/crosshair_tool.c
					${ICSVIEWER_DIR_PATH}/trunk/Tools/crosshair_tool.h
					${ICSVIEWER_DIR_PATH}/trunk/Tools/line_tool.c
					${ICSVIEWER_DIR_PATH}/trunk/Tools/line_tool.h
					${ICSVIEWER_DIR_PATH}/trunk/Tools/roi_tool.c
					${ICSVIEWER_DIR_PATH}/trunk/Tools/roi_tool.h
					${ICSVIEWER_DIR_PATH}/trunk/Tools/zoom_tool.c
					${ICSVIEWER_DIR_PATH}/trunk/Tools/zoom_tool.h
                    ${ICSVIEWER_DIR_PATH}/trunk/Icons/pin_in.bmp
                    ${ICSVIEWER_DIR_PATH}/trunk/Icons/pin_out.bmp
                    ${ICSVIEWER_DIR_PATH}/trunk/Icons/play_off.bmp
                    ${ICSVIEWER_DIR_PATH}/trunk/Icons/play_on.bmp
                    ${ICSVIEWER_DIR_PATH}/trunk/Icons/roi_icon.bmp
                    ${ICSVIEWER_DIR_PATH}/trunk/Icons/stop_off.bmp
                    ${ICSVIEWER_DIR_PATH}/trunk/Icons/stop_on.bmp
                    ${ICSVIEWER_DIR_PATH}/trunk/Icons/zoom_icon.bmp
                    ${ICSVIEWER_DIR_PATH}/trunk/Icons/zoom.cur
)

INCLUDE_DIRECTORIES(
					
                  
					${ICSVIEWER_DIR_PATH}/trunk/Fortify
					${ICSVIEWER_DIR_PATH}/trunk/Shell
					${ICSVIEWER_DIR_PATH}/trunk/Shell/Twain
					${ICSVIEWER_DIR_PATH}/trunk/Plugins
					${ICSVIEWER_DIR_PATH}/trunk/Tools		
					
					)
                    
SOURCE_GROUP(IcsViewer FILES ${ICSVIEWER_SRCS})