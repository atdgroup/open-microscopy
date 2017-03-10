import ctypes as C
    
ATTR_DATA_TYPE = 710
    
CviAttributes = {   'ATTR_DIMMED' : [500, C.c_int],
                    'ATTR_VISIBLE' : [530, C.c_int],
                    'ATTR_LEFT' : [531, C.c_int],
                    'ATTR_TOP' : [532, C.c_int],
                    'ATTR_WIDTH' : [533, C.c_int],
                    'ATTR_HEIGHT' : [540, C.c_int],
                    'ATTR_PANEL_FIRST_CTRL' : [612, C.c_int],
                    'ATTR_NEXT_CTRL' : [613, C.c_int],
                    'ATTR_NUM_CTRLS' : [582, C.c_int],
                    'ATTR_LABEL_TEXT' : [641, C.c_char_p],
                    'ATTR_DATA_TYPE' : [710, C.c_int],
                    'ATTR_CONSTANT_NAME' : [501, C.c_char_p],
                    'ATTR_FRAME_COLOR' : [550, C.c_int],
                    'ATTR_SCROLL_BARS' : [551, C.c_int],
                    'ATTR_SCROLL_BAR_COLOR' : [552, C.c_int],
                    'ATTR_HSCROLL_OFFSET' : [560, C.c_int],
                    'ATTR_HSCROLL_OFFSET_MAX' : [561, C.c_int],
                    'ATTR_BACKCOLOR' : [570, C.c_int],
                    'ATTR_TITLEBAR_VISIBLE' : [571, C.c_int],
                    'ATTR_TITLEBAR_THICKNESS' : [572, C.c_int],
                    'ATTR_TITLE' : [573, C.c_char_p],
                    'ATTR_TITLE_LENGTH' : [574, C.c_int],
                    'ATTR_TITLE_COLOR' : [575, C.c_int],
                    'ATTR_TITLE_BACKCOLOR' : [576, C.c_int],
                    'ATTR_FRAME_STYLE' : [577, C.c_int],
                    'ATTR_FRAME_THICKNESS' : [578, C.c_int],
                    'ATTR_MENU_HEIGHT' : [579, C.c_int],
                    'ATTR_SIZABLE ' : [580, C.c_int],
                    'ATTR_MOVABLE' : [581, C.c_int],
                    'ATTR_MOUSE_CURSOR' : [583, C.c_int],
                    'ATTR_TITLE_FONT' : [584, C.c_char_p],
                    'ATTR_TITLE_ITALIC' : [585, C.c_char_p],
                    'ATTR_TITLE_UNDERLINE' : [586, C.c_int],
                    'ATTR_TITLE_STRIKEOUT' : [587, C.c_int],
                    'ATTR_TITLE_POINT_SIZE' : [588, C.c_int],
                    'ATTR_TITLE_BOLD' : [589, C.c_int],
                    'ATTR_TITLE_FONT_NAME_LENGTH' : [590, C.c_int],
                    'ATTR_MENU_BAR_VISIBLE' : [591, C.c_int],
                    'ATTR_CLOSE_CTRL' : [592, C.c_int],
                    'ATTR_VSCROLL_OFFSET' : [593, C.c_int],
                    'ATTR_VSCROLL_OFFSET_MAX' : [594, C.c_int],
                    'ATTR_PARENT_SHARES_SHORTCUT_KEYS' : [595, C.c_int],
                    'ATTR_ACTIVATE_WHEN_CLICKED_ON' : [596, C.c_int],
                    'ATTR_WINDOW_ZOOM' : [597, C.c_int],
                    'ATTR_SYSTEM_WINDOW_HANDLE' : [598, C.c_int],
                    'ATTR_CAN_MINIMIZE' : [599, C.c_int],
                    'ATTR_CAN_MAXIMIZE' : [600, C.c_int],
                    'ATTR_CLOSE_ITEM_VISIBLE' : [601, C.c_int],
                    'ATTR_SYSTEM_MENU_VISIBLE' : [602, C.c_int],
                    'ATTR_PANEL_MENU_BAR_CONSTANT' : [603, C.c_char_p],   
                    'ATTR_PANEL_MENU_BAR_CONSTANT_LENGTH' : [604, C.c_int],
                    'ATTR_TITLE_SIZE_TO_FONT' : [605, C.c_int],
                    'ATTR_PANEL_PARENT' : [606, C.c_int],
                    'ATTR_NUM_CHILDREN' : [607, C.c_int],
                    'ATTR_FIRST_CHILD' : [608, C.c_int],
                    'ATTR_NEXT_PANEL' : [609, C.c_int],
                    'ATTR_ZPLANE_POSITION' : [610, C.c_int],
                    'ATTR_CTRL_STYLE' : [611, C.c_int],
                    'ATTR_CTRL_TAB_POSITION' : [612, C.c_int],
                    'ATTR_ACTIVE' : [614, C.c_int],
                    'ATTR_FLOATING' : [615, C.c_int],
                    'ATTR_TL_ACTIVATE_WHEN_CLICKED_ON' : [616, C.c_int],
                    'ATTR_CONFORM_TO_SYSTEM' : [617, C.c_int],
                    'ATTR_SCALE_CONTENTS_ON_RESIZE' : [618, C.c_int],
                    'ATTR_MIN_HEIGHT_FOR_SCALING' : [619, C.c_int],
                    'ATTR_CTRL_MODE' : [620, C.c_int],
                    'ATTR_MIN_WIDTH_FOR_SCALING' : [621, C.c_int],
                    'ATTR_HAS_TASKBAR_BUTTON' : [622, C.c_int],
                    'ATTR_OWNER_THREAD_ID' : [623, C.c_int],
                    'ATTR_SCROLL_BAR_STYLE' : [624, C.c_int],
                    'ATTR_CTRL_VAL' : [630, "Variable"],
                    'ATTR_LABEL_COLOR' : [640, C.c_int],
                    'ATTR_LABEL_FONT' : [642, C.c_int],
                    'ATTR_LABEL_ITALIC' : [643, C.c_int],
                    'ATTR_LABEL_UNDERLINE' : [644, C.c_int],
                    'ATTR_LABEL_STRIKEOUT' : [645, C.c_int],
                    'ATTR_LABEL_POINT_SIZE' : [646, C.c_int],
                    'ATTR_LABEL_BOLD' : [647, C.c_int],
                    'ATTR_LABEL_TEXT_LENGTH' : [648, C.c_int],
                    'ATTR_LABEL_SIZE_TO_TEXT' : [649, C.c_int],
                    'ATTR_LABEL_FONT_NAME_LENGTH' : [650, C.c_int],
                    'ATTR_LABEL_VISIBLE' : [660, C.c_int],
                    'ATTR_LABEL_LEFT' : [670, C.c_int],
                    'ATTR_LABEL_TOP' : [671, C.c_int],
                    'ATTR_LABEL_WIDTH' : [672, C.c_int],
                    'ATTR_LABEL_HEIGHT' : [673, C.c_int],
                    'ATTR_LABEL_BGCOLOR' : [674, C.c_int],
                    'ATTR_LABEL_JUSTIFY' : [675, C.c_int],
                    'ATTR_LABEL_RAISED' : [676, C.c_int],
                    'ATTR_TEXT_COLOR' : [680, C.c_int],
                    'ATTR_TEXT_FONT_NAME_LENGTH' : [681, C.c_int],
                    'ATTR_TEXT_FONT' : [682, C.c_int],
                    'ATTR_TEXT_ITALIC' : [683, C.c_int],
                    'ATTR_TEXT_UNDERLINE' : [684, C.c_int],
                    'ATTR_TEXT_STRIKEOUT' : [685, C.c_int],
                    'ATTR_TEXT_POINT_SIZE' : [686, C.c_int],
                    'ATTR_TEXT_BOLD' : [687, C.c_int],
                    'ATTR_TEXT_BGCOLOR' : [690, C.c_int],
                    'ATTR_TEXT_JUSTIFY' : [700, C.c_int],
                    'ATTR_DATA_TYPE' : [710, C.c_int],
                    'ATTR_CTRL_INDEX' : [720, C.c_int],
                    'ATTR_DFLT_INDEX' : [721, C.c_int],
                    'ATTR_MAX_VALUE' : [730, C.c_int],
                    'ATTR_MIN_VALUE' : [731, C.c_int],
                    'ATTR_DFLT_VALUE' : [732, C.c_int],
                    'ATTR_INCR_VALUE' : [733, C.c_int],
                    'ATTR_FORMAT' : [734, C.c_int],
                    'ATTR_PRECISION' : [735, C.c_int],
                    'ATTR_SHOW_RADIX' : [736, C.c_int],
                    'ATTR_SHOW_INCDEC_ARROWS' : [737, C.c_int],
                    'ATTR_CHECK_RANGE' : [738, C.c_int],
                    'ATTR_PADDING' : [739, C.c_int],
                    'ATTR_DFLT_VALUE_LENGTH' : [740, C.c_int],
                    'ATTR_DISABLE_CHECK_MARK' : [755, C.c_int],
                    'ATTR_MAX_ENTRY_LENGTH' : [760, C.c_int],
                    'ATTR_MAX_ENTRY_CHARS' : [761, C.c_int],
                    'ATTR_TEXT_SELECTION_START' : [762, C.c_int],
                    'ATTR_TEXT_SELECTION_LENGTH' : [763, C.c_int],
                    'ATTR_STRING_TEXT_LENGTH' : [770, C.c_int],
                    'ATTR_FIRST_VISIBLE_LINE' : [790, C.c_int],
                    'ATTR_WRAP_MODE' : [791, C.c_int],
                    'ATTR_EXTRA_LINES' : [793, C.c_int],
                    'ATTR_TOTAL_LINES' : [794, C.c_int],
                    'ATTR_ENTER_IS_NEWLINE' : [795, C.c_int],
                    'ATTR_SCROLL_BAR_SIZE' : [820, C.c_int],
                    'ATTR_VISIBLE_LINES' : [821, C.c_int],
                    'ATTR_NO_EDIT_TEXT' : [840, C.c_int],
                    'ATTR_TEXT_RAISED' : [860, C.c_int],
                    'ATTR_SIZE_TO_TEXT' : [861, C.c_int],
                    'ATTR_CMD_BUTTON_COLOR' : [880, C.c_int],
                    'ATTR_AUTO_SIZING' : [881, C.c_int],
                    'ATTR_ON_VALUE' : [900, C.c_int],
                    'ATTR_OFF_VALUE' : [901, C.c_int],
                    'ATTR_ON_VALUE_LENGTH' : [902, C.c_int],
                    'ATTR_OFF_VALUE_LENGTH' : [903, C.c_int],
                    'ATTR_BINARY_SWITCH_COLOR' : [904, C.c_int],
                    'ATTR_ON_COLOR' : [920, C.c_int],
                    'ATTR_OFF_COLOR' : [921, C.c_int],
                    'ATTR_ON_TEXT' : [940, C.c_int],
                    'ATTR_OFF_TEXT' : [941, C.c_int],
                    'ATTR_ON_TEXT_LENGTH' : [942, C.c_int],
                    'ATTR_OFF_TEXT_LENGTH' : [943, C.c_int],
                    'ATTR_DIG_DISP_TOP' : [970, C.c_int],
                    'ATTR_DIG_DISP_LEFT' : [971, C.c_int],
                    'ATTR_DIG_DISP_WIDTH' : [972, C.c_int],
                    'ATTR_DIG_DISP_HEIGHT' : [973, C.c_int],
                    'ATTR_SHOW_DIG_DISP' : [974, C.c_int],
                    'ATTR_SLIDER_COLOR' : [980, C.c_int],
                    'ATTR_NEEDLE_COLOR' : [980, C.c_int],
                    'ATTR_SLIDER_COLOR' : [980, C.c_int],
                    'ATTR_FILL_HOUSING_COLOR' : [981, C.c_int],
                    'ATTR_MARKER_STYLE' : [982, C.c_int],
                    'ATTR_TICK_STYLE' : [983, C.c_int],
                    'ATTR_FILL_COLOR' : [985, C.c_int],
                    'ATTR_FILL_OPTION' : [986, C.c_int],
                    'ATTR_MARKER_START_ANGLE' : [990, C.c_int],
                    'ATTR_MARKER_END_ANGLE' : [991, C.c_int],
                    'ATTR_SLIDER_WIDTH' : [992, C.c_int],
                    'ATTR_SLIDER_HEIGHT' : [993, C.c_int],
                    'ATTR_SHOW_MORE_BUTTON' : [995, C.c_int],
                    'ATTR_SHOW_TRANSPARENT' : [996, C.c_int],
                    'ATTR_SLIDER_LEFT' : [997, C.c_int],
                    'ATTR_SLIDER_TOP' : [998, C.c_int],
                    'ATTR_NUM_DIVISIONS' : [999, C.c_int],
                    'ATTR_MENU_ARROW_COLOR' : [1000, C.c_int],
                    'ATTR_MENU_BAR_POINT_SIZE' : [1010, C.c_int],
                    'ATTR_MENU_BAR_BOLD' : [1011, C.c_int],
                    'ATTR_MENU_BAR_ITALIC' : [1012, C.c_int],
                    'ATTR_MENU_BAR_UNDERLINE' : [1013, C.c_int],
                    'ATTR_MENU_BAR_STRIKEOUT' : [1014, C.c_int],
                    'ATTR_MENU_BAR_FONT' : [1015, C.c_int],
                    'ATTR_MENU_BAR_FONT_NAME_LENGTH' : [1016, C.c_int],
                    'ATTR_MENU_IMAGE_BACKGROUND_COLOR' : [1017, C.c_int],
                    'ATTR_MENU_BAR_IMAGE_SIZE' : [1018, C.c_int],
                    'ATTR_SHORTCUT_KEY' : [1020, C.c_int],
                    'ATTR_CHECKED' : [1040, C.c_int],
                    'ATTR_IS_SEPARATOR' : [1041, C.c_int],
                    'ATTR_ITEM_NAME' : [1042, C.c_char_p],
                    'ATTR_ITEM_NAME_LENGTH' : [1043, C.c_int],
                    'ATTR_SUBMENU_ID' : [1044, C.c_int],
                    'ATTR_NEXT_ITEM_ID' : [1045, C.c_int],
                    'ATTR_BOLD' : [1046, C.c_int],
                    'ATTR_IMAGE_FILE' : [1050, C.c_int],
                    'ATTR_IMAGE_FILE_LENGTH' : [1051, C.c_int],
                    'ATTR_FAST_DRAW_BUTTON' : [1052, C.c_int],
                    'ATTR_USE_SUBIMAGE' : [1053, C.c_int],
                    'ATTR_SUBIMAGE_TOP' : [1054, C.c_int],
                    'ATTR_SUBIMAGE_LEFT' : [1055, C.c_int],
                    'ATTR_SUBIMAGE_WIDTH' : [1056, C.c_int],
                    'ATTR_SUBIMAGE_HEIGHT' : [1057, C.c_int],
                    'ATTR_MENU_NAME' : [1060, C.c_char_p],
                    'ATTR_MENU_NAME_LENGTH' : [1061, C.c_int],
                    'ATTR_NUM_MENU_ITEMS' : [1062, C.c_int],
                    'ATTR_NEXT_MENU_ID' : [1063, C.c_int],
                    'ATTR_FIRST_ITEM_ID' : [1064, C.c_int],
                    'ATTR_NUM_MENUS' : [1070, C.c_int],
                    'ATTR_DRAW_LIGHT_BEVEL' : [1071, C.c_int],
                    'ATTR_DIMMER_CALLBACK' : [1072, C.c_int],
                    'ATTR_FIRST_MENU_ID' : [1073, C.c_int],
                    'ATTR_SEND_DIMMER_EVENTS_FOR_ALL_KEYS' : [1074, C.c_int],
                    'ATTR_GRID_COLOR' : [1080, C.c_int],
                    'ATTR_PLOT_BGCOLOR' : [1081, C.c_int],
                    'ATTR_XYNAME_FONT' : [1082, C.c_int],
                    'ATTR_XYNAME_COLOR' : [1083, C.c_int],
                    'ATTR_XYLABEL_FONT' : [1084, C.c_int],
                    'ATTR_XYLABEL_COLOR' : [1085, C.c_int],
                    'ATTR_XNAME' : [1086, C.c_int],
                    'ATTR_XGRID_VISIBLE' : [1087, C.c_int],
                    'ATTR_XLABEL_VISIBLE' : [1088, C.c_int],
                    'ATTR_XFORMAT' : [1089, C.c_int],
                    'ATTR_XDIVISIONS' : [1090, C.c_int],
                    'ATTR_XPRECISION' : [1091, C.c_int],
                    'ATTR_XENG_UNITS' : [1092, C.c_int],
                    'ATTR_YNAME' : [1093, C.c_int],
                    'ATTR_YGRID_VISIBLE' : [1094, C.c_int],
                    'ATTR_YLABEL_VISIBLE' : [1095, C.c_int],
                    'ATTR_YMAP_MODE' : [1096, C.c_int],
                    'ATTR_YFORMAT' : [1097, C.c_int],
                    'ATTR_YDIVISIONS' : [1098, C.c_int],
                    'ATTR_YPRECISION' : [1099, C.c_int],
                    'ATTR_YENG_UNITS' : [1100, C.c_int],
                    'ATTR_EDGE_STYLE' : [1101, C.c_int],
                    'ATTR_BORDER_VISIBLE' : [1102, C.c_int],
                    'ATTR_XYNAME_BOLD' : [1103, C.c_int],
                    'ATTR_XYNAME_ITALIC' : [1104, C.c_int],
                    'ATTR_XYNAME_UNDERLINE' : [1105, C.c_int],
                    'ATTR_XYNAME_STRIKEOUT' : [1106, C.c_int],
                    'ATTR_XYNAME_POINT_SIZE' : [1107, C.c_int],
                    'ATTR_XNAME_LENGTH' : [1108, C.c_int],
                    'ATTR_YNAME_LENGTH' : [1109, C.c_int],
                    'ATTR_XYNAME_FONT_NAME_LENGTH' : [1110, C.c_int],
                    'ATTR_XYLABEL_BOLD' : [1111, C.c_int],
                    'ATTR_XYLABEL_ITALIC' : [1112, C.c_int],
                    'ATTR_XYLABEL_UNDERLINE' : [1113, C.c_int],
                    'ATTR_XYLABEL_STRIKEOUT' : [1114, C.c_int],
                    'ATTR_XYLABEL_POINT_SIZE' : [1115, C.c_int],
                    'ATTR_XYLABEL_FONT_NAME_LENGTH' : [1116, C.c_int],
                    'ATTR_GRAPH_BGCOLOR' : [1117, C.c_int],
                    'ATTR_PLOT_AREA_WIDTH' : [1118, C.c_int],
                    'ATTR_PLOT_AREA_HEIGHT' : [1119, C.c_int],
                    'ATTR_INNER_MARKERS_VISIBLE' : [1120, C.c_int],
                    'ATTR_YREVERSE' : [1121, C.c_int],
                    'ATTR_XUSE_LABEL_STRINGS' : [1122, C.c_int],
                    'ATTR_YUSE_LABEL_STRINGS' : [1123, C.c_int],
                    'ATTR_XAXIS_GAIN' : [1124, C.c_int],
                    'ATTR_YAXIS_GAIN' : [1125, C.c_int],
                    'ATTR_XAXIS_OFFSET' : [1126, C.c_int],
                    'ATTR_YAXIS_OFFSET' : [1127, C.c_int],
                    'ATTR_PLOT_AREA_TOP' : [1128, C.c_int],
                    'ATTR_PLOT_AREA_LEFT' : [1129, C.c_int],
                    'ATTR_XPADDING' : [1130, C.c_int],
                    'ATTR_YPADDING' : [1131, C.c_int],
                    'ATTR_ACTUAL_XDIVISIONS' : [1132, C.c_int],
                    'ATTR_ACTUAL_YDIVISIONS' : [1133, C.c_int],
                    'ATTR_ACTUAL_XPRECISION' : [1134, C.c_int],
                    'ATTR_ACTUAL_YPRECISION' : [1135, C.c_int],
                    'ATTR_XMINORGRID_VISIBLE' : [1136, C.c_int],
                    'ATTR_YMINORGRID_VISIBLE' : [1137, C.c_int],
                    'ATTR_NUM_CURSORS' : [1140, C.c_int],
                    'ATTR_XMAP_MODE' : [1141, C.c_int],
                    'ATTR_DATA_MODE' : [1142, C.c_int],
                    'ATTR_COPY_ORIGINAL_DATA' : [1143, C.c_int],
                    'ATTR_XMARK_ORIGIN' : [1144, C.c_int],
                    'ATTR_YMARK_ORIGIN' : [1145, C.c_int],
                    'ATTR_SMOOTH_UPDATE' : [1146, C.c_int],
                    'ATTR_REFRESH_GRAPH' : [1147, C.c_int],
                    'ATTR_SHIFT_TEXT_PLOTS' : [1148, C.c_int],
                    'ATTR_ACTIVE_YAXIS' : [1149, C.c_int],
                    'ATTR_XREVERSE' : [1150, C.c_int],
                    'ATTR_ENABLE_ZOOM_AND_PAN' : [1151, C.c_int],
                    'ATTR_XLOOSE_FIT_AUTOSCALING' : [1152, C.c_int],
                    'ATTR_YLOOSE_FIT_AUTOSCALING' : [1153, C.c_int],
                    'ATTR_XLOOSE_FIT_AUTOSCALING_UNIT' : [1154, C.c_int],
                    'ATTR_YLOOSE_FIT_AUTOSCALING_UNIT' : [1155, C.c_int],
                    'ATTR_ANTI_ALIASED_PLOTS' : [1156, C.c_int],
                    'ATTR_LEGEND_VISIBLE' : [1157, C.c_int],
                    'ATTR_LEGEND_TOP' : [1158, C.c_int],
                    'ATTR_LEGEND_LEFT' : [1159, C.c_int],
                    'ATTR_LEGEND_WIDTH' : [1160, C.c_int],
                    'ATTR_LEGEND_HEIGHT' : [1161, C.c_int],
                    'ATTR_LEGEND_FRAME_COLOR' : [1162, C.c_int],
                    'ATTR_LEGEND_PLOT_BGCOLOR' : [1163, C.c_int],
                    'ATTR_LEGEND_SHOW_SAMPLES' : [1164, C.c_int],
                    'ATTR_LEGEND_AUTO_SIZE' : [1165, C.c_int],
                    'ATTR_LEGEND_AUTO_DISPLAY' : [1166, C.c_int],
                    'ATTR_LEGEND_NUM_VISIBLE_ITEMS' : [1167, C.c_int],
                    'ATTR_ACTIVE_XAXIS' : [1168, C.c_int],
                    'ATTR_LEGEND_INTERACTIVE' : [1169, C.c_int],
                    'ATTR_NUM_TRACES' : [1170, C.c_int],
                    'ATTR_POINTS_PER_SCREEN' : [1171, C.c_int],
                    'ATTR_SCROLL_MODE' : [1172, C.c_int],
                    'ATTR_CURSOR_MODE' : [1200, C.c_int],
                    'ATTR_CURSOR_POINT_STYLE' : [1201, C.c_int],
                    'ATTR_CROSS_HAIR_STYLE' : [1202, C.c_int],
                    'ATTR_CURSOR_COLOR' : [1203, C.c_int],
                    'ATTR_CURSOR_YAXIS' : [1204, C.c_int],
                    'ATTR_CURSOR_ENABLED' : [1205, C.c_int],
                    'ATTR_CURSOR_XAXIS' : [1206, C.c_int],
                    'ATTR_TRACE_COLOR' : [1230, C.c_int],
                    'ATTR_PLOT_STYLE' : [1231, C.c_int],
                    'ATTR_TRACE_POINT_STYLE' : [1232, C.c_int],
                    'ATTR_LINE_STYLE' : [1233, C.c_int],
                    'ATTR_TRACE_VISIBLE' : [1234, C.c_int],
                    'ATTR_TRACE_BGCOLOR' : [1240, C.c_int],
                    'ATTR_PLOT_FONT' : [1241, C.c_int],
                    'ATTR_PLOT_FONT_NAME_LENGTH' : [1242, C.c_int],
                    'ATTR_INTERPOLATE_PIXELS' : [1243, C.c_int],
                    'ATTR_PLOT_ZPLANE_POSITION' : [1244, C.c_int],
                    'ATTR_NUM_POINTS' : [1245, C.c_int],
                    'ATTR_PLOT_XDATA' : [1246, C.c_int],
                    'ATTR_PLOT_YDATA' : [1247, C.c_int],
                    'ATTR_PLOT_ZDATA' : [1248, C.c_int],
                    'ATTR_PLOT_XDATA_TYPE' : [1249, C.c_int],
                    'ATTR_PLOT_YDATA_TYPE' : [1250, C.c_int],
                    'ATTR_PLOT_ZDATA_TYPE' : [1251, C.c_int],
                    'ATTR_PLOT_XDATA_SIZE' : [1252, C.c_int],
                    'ATTR_PLOT_YDATA_SIZE' : [1253, C.c_int],
                    'ATTR_PLOT_ZDATA_SIZE' : [1254, C.c_int],
                    'ATTR_PLOT_YAXIS' : [1255, C.c_int],
                    'ATTR_PLOT_SNAPPABLE' : [1256, C.c_int],
                    'ATTR_PLOT_ORIGIN' : [1257, C.c_int],
                    'ATTR_PLOT_THICKNESS' : [1258, C.c_int],
                    'ATTR_PLOT_XAXIS' : [1259, C.c_int],
                    'ATTR_CHECK_MODE' : [1260, C.c_int],
                    'ATTR_CHECK_STYLE' : [1261, C.c_int],
                    'ATTR_TEXT_CLICK_TOGGLES_CHECK' : [1262, C.c_int],
                    'ATTR_HILITE_CURRENT_ITEM' : [1263, C.c_int],
                    'ATTR_ALLOW_ROOM_FOR_IMAGES' : [1264, C.c_int],
                    'ATTR_DRAGGABLE_MARKS' : [1265, C.c_int],
                    'ATTR_INTERVAL' : [1270, C.c_float],
                    'ATTR_ENABLED' : [1271, C.c_int],
                    'ATTR_PLOT_LG_VISIBLE' : [1275, C.c_int],
                    'ATTR_PLOT_LG_TEXT' : [1276, C.c_int],
                    'ATTR_PLOT_LG_TEXT_LENGTH' : [1277, C.c_int],
                    'ATTR_PLOT_LG_TEXT_COLOR' : [1278, C.c_int],
                    'ATTR_PLOT_LG_FONT' : [1280, C.c_int],
                    'ATTR_PLOT_LG_FONT_NAME_LENGTH' : [1281, C.c_int],
                    'ATTR_FRAME_VISIBLE' : [1290, C.c_int],
                    'ATTR_PICT_BGCOLOR' : [1291, C.c_int],
                    'ATTR_FIT_MODE' : [1292, C.c_int],
                    'ATTR_ORIENTATION' : [1300, C.c_int],
                    'ATTR_PRINT_AREA_HEIGHT' : [1301, C.c_int],
                    'ATTR_PRINT_AREA_WIDTH' : [1302, C.c_int],
                    'ATTR_NUMCOPIES' : [1303, C.c_int],
                    'ATTR_XRESOLUTION' : [1304, C.c_int],
                    'ATTR_YRESOLUTION' : [1305, C.c_int],
                    'ATTR_XOFFSET' : [1306, C.c_int],
                    'ATTR_YOFFSET' : [1307, C.c_int],
                    'ATTR_COLOR_MODE' : [1308, C.c_int],
                    'ATTR_DUPLEX' : [1309, C.c_int],
                    'ATTR_EJECT_AFTER' : [1310, C.c_int],
                    'ATTR_TEXT_WRAP' : [1311, C.c_int],
                    'ATTR_TAB_INTERVAL' : [1312, C.c_int],
                    'ATTR_SHOW_PAGE_NUMBERS' : [1313, C.c_int],
                    'ATTR_SHOW_LINE_NUMBERS' : [1314, C.c_int],
                    'ATTR_SHOW_FILE_NAME' : [1315, C.c_int],
                    'ATTR_SHOW_DATE' : [1316, C.c_int],
                    'ATTR_SHOW_TIME' : [1317, C.c_int],
                    'ATTR_PRINT_FONT_NAME' : [1318, C.c_int],
                    'ATTR_PRINT_ITALIC' : [1319, C.c_int],
                    'ATTR_PRINT_UNDERLINE' : [1320, C.c_int],
                    'ATTR_PRINT_STRIKEOUT' : [1321, C.c_int],
                    'ATTR_PRINT_POINT_SIZE' : [1322, C.c_int],
                    'ATTR_PRINT_BOLD' : [1323, C.c_int],
                    'ATTR_PRINT_FONT_NAME_LENGTH' : [1324, C.c_int],
                    'ATTR_PRINTER_NAME' : [1325, C.c_int],
                    'ATTR_PRINTER_NAME_LENGTH' : [1326, C.c_int],
                    'ATTR_BITMAP_PRINTING' : [1327, C.c_int],
                    'ATTR_SYSTEM_PRINT_DIALOG_ONLY' : [1328, C.c_int],
                    'ATTR_CHARS_PER_LINE' : [1329, C.c_int],
                    'ATTR_LINES_PER_PAGE' : [1330, C.c_int],
                    'ATTR_PEN_COLOR' : [1350, C.c_int],
                    'ATTR_PEN_FILL_COLOR' : [1351, C.c_int],
                    'ATTR_PEN_MODE' : [1354, C.c_int],
                    'ATTR_PEN_WIDTH' : [1355, C.c_int],
                    'ATTR_PEN_PATTERN' : [1356, C.c_int],
                    'ATTR_PEN_STYLE' : [1357, C.c_int],
                    'ATTR_DRAW_POLICY' : [1370, C.c_int],
                    'ATTR_OVERLAPPED' : [1372, C.c_int],
                    'ATTR_OVERLAPPED_POLICY' : [1373, C.c_int],
                    'ATTR_XCOORD_AT_ORIGIN' : [1374, C.c_int],
                    'ATTR_YCOORD_AT_ORIGIN' : [1375, C.c_int],
                    'ATTR_XSCALING' : [1376, C.c_int],
                    'ATTR_YSCALING' : [1377, C.c_int],
                    'ATTR_ALLOW_UNSAFE_TIMER_EVENTS' : [1400, C.c_int],
                    'ATTR_REPORT_LOAD_FAILURE' : [1405, C.c_int],
                    'ATTR_ALLOW_MISSING_CALLBACKS' : [1410, C.c_int],
                    'ATTR_SUPPRESS_EVENT_PROCESSING' : [1415, C.c_int],
                    'ATTR_TASKBAR_BUTTON_VISIBLE' : [1420, C.c_int],
                    'ATTR_TASKBAR_BUTTON_TEXT' : [1425, C.c_int],
                    'ATTR_DEFAULT_MONITOR' : [1430, C.c_int],
                    'ATTR_PRIMARY_MONITOR' : [1435, C.c_int],
                    'ATTR_NUM_MONITORS' : [1440, C.c_int],
                    'ATTR_FIRST_MONITOR' : [1445, C.c_int],
                    'ATTR_DISABLE_PROG_PANEL_SIZE_EVENTS' : [1450, C.c_int],
                    'ATTR_USE_LOCALIZED_DECIMAL_SYMBOL' : [1451, C.c_int],
                    'ATTR_LOCALIZED_DECIMAL_SYMBOL' : [1452, C.c_int],
                    'ATTR_RESOLUTION_ADJUSTMENT' : [1500, C.c_int],
                    'ATTR_UPPER_LEFT_CORNER_COLOR' : [1510, C.c_int],
                    'ATTR_ROW_LABELS_COLOR' : [1511, C.c_int],
                    'ATTR_COLUMN_LABELS_COLOR' : [1512, C.c_int],
                    'ATTR_TABLE_BGCOLOR' : [1513, C.c_int],
                    'ATTR_TABLE_MODE' : [1514, C.c_int],
                    'ATTR_ROW_LABELS_VISIBLE' : [1515, C.c_int],
                    'ATTR_COLUMN_LABELS_VISIBLE' : [1516, C.c_int],
                    'ATTR_ROW_LABELS_WIDTH' : [1517, C.c_int],
                    'ATTR_COLUMN_LABELS_HEIGHT' : [1518, C.c_int],
                    'ATTR_FIRST_VISIBLE_ROW' : [1519, C.c_int],
                    'ATTR_FIRST_VISIBLE_COLUMN' : [1520, C.c_int],
                    'ATTR_NUM_VISIBLE_ROWS' : [1521, C.c_int],
                    'ATTR_NUM_VISIBLE_COLUMNS' : [1522, C.c_int],
                    'ATTR_ENABLE_ROW_SIZING' : [1523, C.c_int],
                    'ATTR_ENABLE_COLUMN_SIZING' : [1524, C.c_int],
                    'ATTR_ENABLE_POPUP_MENU' : [1525, C.c_int],
                    'ATTR_GRID_AREA_TOP' : [1526, C.c_int],
                    'ATTR_GRID_AREA_LEFT' : [1527, C.c_int],
                    'ATTR_GRID_AREA_WIDTH' : [1528, C.c_int],
                    'ATTR_GRID_AREA_HEIGHT' : [1529, C.c_int],
                    'ATTR_TABLE_RUN_STATE' : [1530, C.c_int],
                    'ATTR_AUTO_EDIT' : [1531, C.c_int],
                    'ATTR_CELL_TYPE' : [1580, C.c_int],
                    'ATTR_CELL_DIMMED' : [1581, C.c_int],
                    'ATTR_CELL_MODE' : [1582, C.c_int],
                    'ATTR_HORIZONTAL_GRID_COLOR' : [1583, C.c_int],
                    'ATTR_VERTICAL_GRID_COLOR' : [1584, C.c_int],
                    'ATTR_HORIZONTAL_GRID_VISIBLE' : [1585, C.c_int],
                    'ATTR_VERTICAL_GRID_VISIBLE' : [1586, C.c_int],
                    'ATTR_MIN_NUM_LINES_VISIBLE' : [1587, C.c_int],
                    'ATTR_INCDEC_ARROW_COLOR' : [1588, C.c_int],
                    'ATTR_NUM_CELL_DFLT_VALUE' : [1589, C.c_int],
                    'ATTR_STR_CELL_DFLT_VALUE' : [1590, C.c_int],
                    'ATTR_STR_CELL_DFLT_VALUE_LENGTH' : [1591, C.c_int],
                    'ATTR_CELL_JUSTIFY' : [1592, C.c_int],
                    'ATTR_STR_CELL_NUM_LINES' : [1593, C.c_int],
                    'ATTR_CELL_FRAME_COLOR' : [1594, C.c_int],
                    'ATTR_SHOW_RING_ARROW' : [1595, C.c_int],
                    'ATTR_RING_ARROW_LOCATION' : [1596, C.c_int],
                    'ATTR_RING_ITEMS_UNIQUE' : [1597, C.c_int],
                    'ATTR_CASE_SENSITIVE_COMPARE' : [1598, C.c_int],
                    'ATTR_CELL_SHORTCUT_KEY' : [1599, C.c_int],
                    'ATTR_USE_LABEL_TEXT' : [1620, C.c_int],
                    'ATTR_SIZE_MODE' : [1621, C.c_int],
                    'ATTR_LABEL_WRAP_MODE' : [1622, C.c_int],
                    'ATTR_ROW_HEIGHT' : [1650, C.c_int],
                    'ATTR_ROW_ACTUAL_HEIGHT' : [1651, C.c_int],
                    'ATTR_COLUMN_WIDTH' : [1700, C.c_int],
                    'ATTR_COLUMN_ACTUAL_WIDTH' : [1701, C.c_int],
                    'ATTR_COLUMN_VISIBLE' : [1702, C.c_int],
                    'ATTR_DATASOCKET_ENABLED' : [1750, C.c_int],
                    'ATTR_DS_BIND_PLOT_STYLE' : [1770, C.c_int],
                    'ATTR_DS_BIND_POINT_STYLE' : [1771, C.c_int],
                    'ATTR_DS_BIND_LINE_STYLE' : [1772, C.c_int],
                    'ATTR_DS_BIND_PLOT_COLOR' : [1773, C.c_int],
                    'ATTR_DATASOCKET_SOURCE' : [1791, C.c_int],
                    'ATTR_DATASOCKET_SOURCE_LENGTH' : [1792, C.c_int],
                    'ATTR_DATASOCKET_MODE' : [1793, C.c_int],
                    'ATTR_COLOR_DEPTH' : [1800, C.c_int],
                    'ATTR_SYSTEM_MONITOR_HANDLE' : [1801, C.c_int],
                    'ATTR_NEXT_MONITOR' : [1802, C.c_int],
                    'ATTR_POPUP_STYLE' : [1850, C.c_int],
                    'ATTR_TREE_BGCOLOR' : [1900, C.c_int],
                    'ATTR_SHOW_CONNECTION_LINES' : [1901, C.c_int],
                    'ATTR_SHOW_PLUS_MINUS' : [1902, C.c_int],
                    'ATTR_SHOW_MARKS' : [1903, C.c_int],
                    'ATTR_SHOW_IMAGES' : [1904, C.c_int],
                    'ATTR_MARK_REFLECT' : [1905, C.c_int],
                    'ATTR_AUTO_EXPAND' : [1906, C.c_int],
                    'ATTR_AUTO_HSCROLL' : [1907, C.c_int],
                    'ATTR_FULL_ROW_SELECT' : [1908, C.c_int],
                    'ATTR_INDENT_OFFSET' : [1909, C.c_int],
                    'ATTR_RADIO_SIBLING_ALWAYS_MARKED' : [1910, C.c_int],
                    'ATTR_TEXT_CLICK_TOGGLES_MARK' : [1911, C.c_int],
                    'ATTR_HIDE_ACTIVE_ITEM' : [1912, C.c_int],
                    'ATTR_SELECTION_MODE' : [1913, C.c_int],
                    'ATTR_TREE_RUN_STATE' : [1914, C.c_int],
                    'ATTR_ENABLE_DRAG_DROP' : [1915, C.c_int],
                    'ATTR_EXPANDED_IMAGE_INDEX' : [1916, C.c_int],
                    'ATTR_DISABLE_TOOLTIPS' : [1917, C.c_int],
                    'ATTR_COLLAPSED_IMAGE_INDEX' : [1931, C.c_int],
                    'ATTR_MARK_TYPE' : [1950, C.c_int],
                    'ATTR_MARK_STATE' : [1951, C.c_int],
                    'ATTR_SELECTED' : [1952, C.c_int],
                    'ATTR_COLLAPSED' : [1953, C.c_int],
                    'ATTR_NO_EDIT_LABEL' : [1954, C.c_int],
                    'ATTR_ITEM_HEIGHT' : [1955, C.c_int],
                    'ATTR_ITEM_ACTUAL_HEIGHT' : [1956, C.c_int],
                    'ATTR_ENABLE_DRAG' : [1957, C.c_int],
                    'ATTR_ENABLE_DROP' : [1958, C.c_int],
                    'ATTR_IMAGE_INDEX' : [1959, C.c_int],
                    'ATTR_HILITE_ONLY_WHEN_PANEL_ACTIVE' : [1990, C.c_int],
                    'ATTR_TITLEBAR_STYLE' : [1991, C.c_int],
                    'ATTR_ITEM_BITMAP' : [1992, C.c_int],
                    'ATTR_TOP_RANGE' : [2000, C.c_int],
                    'ATTR_BOTTOM_RANGE' : [2001, C.c_int],
                    'ATTR_LEFT_RANGE' : [2002, C.c_int],
                    'ATTR_RIGHT_RANGE' : [2003, C.c_int],
                    'ATTR_SHOW_CONTENTS_WHILE_DRAGGING' : [2004, C.c_int],
                    'ATTR_SPAN_PANEL' : [2005, C.c_int],
                    'ATTR_OPERABLE_AS_INDICATOR' : [2006, C.c_int],
                    'ATTR_TOP_ACTUAL_RANGE' : [2007, C.c_int],
                    'ATTR_BOTTOM_ACTUAL_RANGE' : [2008, C.c_int],
                    'ATTR_LEFT_ACTUAL_RANGE' : [2009, C.c_int],
                    'ATTR_RIGHT_ACTUAL_RANGE' : [2010, C.c_int],
                    'ATTR_DIGWAVEFORM_AUTOSIZE' : [2100, C.c_int],
                    'ATTR_DIGWAVEFORM_LINE_LABEL' : [2101, C.c_int],
                    'ATTR_DIGWAVEFORM_LINE_LABEL_LENGTH' : [2102, C.c_int],
                    'ATTR_DIGWAVEFORM_BUS_LABEL' : [2103, C.c_int],
                    'ATTR_DIGWAVEFORM_BUS_LABEL_LENGTH' : [2104, C.c_int],
                    'ATTR_DIGWAVEFORM_FONT' : [2105, C.c_int],
                    'ATTR_DIGWAVEFORM_FONT_NAME_LENGTH' : [2106, C.c_int],
                    'ATTR_DIGWAVEFORM_SHOW_STATE_LABEL' : [2107, C.c_int],
                    'ATTR_DIGWAVEFORM_EXPAND_BUSES' : [2108, C.c_int],
                    'ATTR_DIGWAVEFORM_PLOT_COLOR' : [2109, C.c_int],
                    'ATTR_TABS_FIT_MODE' : [2200, C.c_int],
                    'ATTR_TABS_LOCATION' : [2201, C.c_int],
                    'ATTR_TABS_VISIBLE' : [2202, C.c_int],
                    'ATTR_TABS_START_OFFSET' : [2203, C.c_int],
                    'ATTR_TABS_END_OFFSET' : [2204, C.c_int],
                    'ATTR_NUM_ANNOTATIONS' : [2300, C.c_int]
                }


# Data Types

VAL_CHAR                         = 0
VAL_INTEGER                      = 1
VAL_SHORT_INTEGER                = 2
VAL_FLOAT                        = 3
VAL_DOUBLE                       = 4
VAL_STRING                       = 5
VAL_UNSIGNED_SHORT_INTEGER       = 6
VAL_UNSIGNED_INTEGER             = 7
VAL_UNSIGNED_CHAR                = 8
VAL_NO_TYPE                      = 9    


ctype_types = [C.c_char,
               C.c_int,
               C.c_short,
               C.c_float,
               C.c_double,
               C.c_char_p,
               C.c_ushort,
               C.c_uint,
               C.c_ubyte]

def CviTypeToCType(cvi_dt):
    return ctype_types[cvi_dt]

# Events

EVENT_NONE                       = 0
EVENT_COMMIT                     = 1   
EVENT_VAL_CHANGED                = 2 
EVENT_LEFT_CLICK                 = 3   
EVENT_LEFT_DOUBLE_CLICK          = 4
EVENT_RIGHT_CLICK                = 5   
EVENT_RIGHT_DOUBLE_CLICK         = 6   
EVENT_KEYPRESS                   = 7   
EVENT_GOT_FOCUS                  = 8   
EVENT_LOST_FOCUS                 = 9   
EVENT_IDLE                       = 10
EVENT_CLOSE                      = 11
EVENT_PANEL_SIZE                 = 12
EVENT_PANEL_MOVE                 = 13
EVENT_END_TASK                   = 14 
EVENT_TIMER_TICK                 = 15  
EVENT_DISCARD                    = 16 
EVENT_EXPAND                     = 17
EVENT_COLLAPSE                   = 18
EVENT_DRAG                       = 19
EVENT_DROP                       = 20
EVENT_DROPPED                    = 21
EVENT_SORT                       = 22
EVENT_SELECTION_CHANGE           = 23
EVENT_HSCROLL                    = 24
EVENT_VSCROLL                    = 25
EVENT_MARK_STATE_CHANGE          = 26
EVENT_COMBO_BOX_INSERT           = 27         
EVENT_ACTIVE_CELL_CHANGE         = 28
EVENT_ROW_SIZE_CHANGE            = 29
EVENT_COLUMN_SIZE_CHANGE         = 30 
EVENT_ACTIVE_TAB_CHANGE          = 31 
EVENT_EDIT_MODE_STATE_CHANGE     = 32  


# Controls

CTRL_NUMERIC                    = 100
CTRL_NUMERIC_THERMOMETER        = 101
CTRL_NUMERIC_TANK               = 102
CTRL_NUMERIC_GAUGE              = 103
CTRL_NUMERIC_METER              = 104
CTRL_NUMERIC_KNOB               = 105
CTRL_NUMERIC_DIAL               = 106
CTRL_NUMERIC_VSLIDE             = 107
CTRL_NUMERIC_HSLIDE             = 108
CTRL_NUMERIC_FLAT_VSLIDE        = 109
CTRL_NUMERIC_FLAT_HSLIDE        = 110
CTRL_NUMERIC_LEVEL_VSLIDE       = 111
CTRL_NUMERIC_LEVEL_HSLIDE       = 112
CTRL_NUMERIC_POINTER_VSLIDE     = 113
CTRL_NUMERIC_POINTER_HSLIDE     = 114
CTRL_NUMERIC_LS                 = 115
CTRL_NUMERIC_THERMOMETER_LS     = 116
CTRL_NUMERIC_TANK_LS            = 117
CTRL_NUMERIC_GAUGE_LS           = 118
CTRL_NUMERIC_METER_LS           = 119
CTRL_NUMERIC_KNOB_LS            = 120
CTRL_NUMERIC_DIAL_LS            = 121
CTRL_NUMERIC_LEVEL_VSLIDE_LS    = 126
CTRL_NUMERIC_LEVEL_HSLIDE_LS    = 127
CTRL_NUMERIC_POINTER_VSLIDE_LS  = 128
CTRL_NUMERIC_POINTER_HSLIDE_LS  = 129

CTRL_COLOR_NUMERIC              = 130
CTRL_COLOR_NUMERIC_LS           = 131

CTRL_STRING                     = 150
CTRL_STRING_LS                  = 151

CTRL_TEXT_MSG                   = 160

CTRL_TEXT_BOX                   = 170
CTRL_TEXT_BOX_LS                = 171

#Command buttons
CTRL_SQUARE_COMMAND_BUTTON      = 200
CTRL_OBLONG_COMMAND_BUTTON      = 201
CTRL_ROUND_COMMAND_BUTTON       = 202
CTRL_ROUNDED_COMMAND_BUTTON     = 203
CTRL_PICTURE_COMMAND_BUTTON     = 204
CTRL_SQUARE_COMMAND_BUTTON_LS   = 205
CTRL_PICTURE_COMMAND_BUTTON_LS  = 206

#Buttons 
CTRL_ROUND_BUTTON               = 220
CTRL_SQUARE_BUTTON              = 221
CTRL_ROUND_FLAT_BUTTON          = 222
CTRL_SQUARE_FLAT_BUTTON         = 223
CTRL_ROUND_RADIO_BUTTON         = 224
CTRL_SQUARE_RADIO_BUTTON        = 225
CTRL_CHECK_BOX                  = 226
CTRL_ROUND_PUSH_BUTTON          = 227
CTRL_SQUARE_PUSH_BUTTON         = 228
CTRL_ROUND_PUSH_BUTTON2         = 229
CTRL_SQUARE_PUSH_BUTTON2        = 230
CTRL_SQUARE_TEXT_BUTTON         = 231
CTRL_OBLONG_TEXT_BUTTON         = 232
CTRL_ROUND_TEXT_BUTTON          = 233
CTRL_ROUNDED_TEXT_BUTTON        = 234
CTRL_PICTURE_TOGGLE_BUTTON      = 235
CTRL_SQUARE_BUTTON_LS           = 240
CTRL_PICTURE_TOGGLE_BUTTON_LS   = 241
CTRL_SQUARE_PUSH_BUTTON_LS      = 242
CTRL_SQUARE_TEXT_BUTTON_LS      = 243

#LED's
CTRL_ROUND_LIGHT                = 260
CTRL_SQUARE_LIGHT               = 261
CTRL_ROUND_LED                  = 262
CTRL_SQUARE_LED                 = 263
CTRL_ROUND_LED_LS               = 264
CTRL_SQUARE_LED_LS              = 265

#binary switches
CTRL_HSWITCH                    = 280
CTRL_VSWITCH                    = 281
CTRL_GROOVED_HSWITCH            = 282
CTRL_GROOVED_VSWITCH            = 283
CTRL_TOGGLE_HSWITCH             = 284
CTRL_TOGGLE_VSWITCH             = 285
CTRL_TOGGLE_HSWITCH_LS          = 288
CTRL_TOGGLE_VSWITCH_LS          = 289

#rings
CTRL_RING                       = 300
CTRL_RECESSED_MENU_RING         = 301
CTRL_MENU_RING                  = 302
CTRL_POPUP_MENU_RING            = 303
CTRL_RING_VSLIDE                = 304
CTRL_RING_HSLIDE                = 305
CTRL_RING_FLAT_VSLIDE           = 306
CTRL_RING_FLAT_HSLIDE           = 307
CTRL_RING_LEVEL_VSLIDE          = 308
CTRL_RING_LEVEL_HSLIDE          = 309
CTRL_RING_POINTER_VSLIDE        = 310
CTRL_RING_POINTER_HSLIDE        = 311
CTRL_RING_THERMOMETER           = 312
CTRL_RING_TANK                  = 313
CTRL_RING_GAUGE                 = 314
CTRL_RING_METER                 = 315
CTRL_RING_KNOB                  = 316
CTRL_RING_DIAL                  = 317
CTRL_PICTURE_RING               = 318
CTRL_RING_LS                    = 319
CTRL_RECESSED_MENU_RING_LS      = 320
CTRL_MENU_RING_LS               = 321
CTRL_POPUP_MENU_RING_LS         = 322
CTRL_RING_LEVEL_VSLIDE_LS       = 327
CTRL_RING_LEVEL_HSLIDE_LS       = 328
CTRL_RING_POINTER_VSLIDE_LS     = 329
CTRL_RING_POINTER_HSLIDE_LS     = 330
CTRL_RING_THERMOMETER_LS        = 331
CTRL_RING_TANK_LS               = 332
CTRL_RING_GAUGE_LS              = 333
CTRL_RING_METER_LS              = 334
CTRL_RING_KNOB_LS               = 335
CTRL_RING_DIAL_LS               = 336
CTRL_PICTURE_RING_LS            = 337

CTRL_LIST                       = 340
CTRL_LIST_LS                    = 341

#decorations 
CTRL_RAISED_BOX                 = 380
CTRL_RECESSED_BOX               = 381
CTRL_FLAT_BOX                   = 382
CTRL_RAISED_CIRCLE              = 383
CTRL_RECESSED_CIRCLE            = 384
CTRL_FLAT_CIRCLE                = 385
CTRL_RAISED_FRAME               = 386
CTRL_RECESSED_FRAME             = 387
CTRL_FLAT_FRAME                 = 388
CTRL_RAISED_ROUND_FRAME         = 389
CTRL_RECESSED_ROUND_FRAME       = 390
CTRL_FLAT_ROUND_FRAME           = 391
CTRL_RAISED_ROUNDED_BOX         = 392
CTRL_RECESSED_ROUNDED_BOX       = 393
CTRL_FLAT_ROUNDED_BOX           = 394
CTRL_RAISED_BOX_LS              = 395
CTRL_RECESSED_BOX_LS            = 396
CTRL_SMOOTH_VERTICAL_BOX_LS     = 397
CTRL_SMOOTH_HORIZONTAL_BOX_LS   = 398
CTRL_RECESSED_NARROW_FRAME      = 410

CTRL_GRAPH                      = 440
CTRL_GRAPH_LS                   = 441
CTRL_DIGITAL_GRAPH              = 450
CTRL_DIGITAL_GRAPH_LS           = 451
CTRL_STRIP_CHART                = 460
CTRL_STRIP_CHART_LS             = 461

CTRL_PICTURE                    = 480
CTRL_PICTURE_LS                 = 481

CTRL_TIMER                      = 490

CTRL_CANVAS                     = 500

CTRL_ACTIVEX                    = 503

CTRL_TABLE                      = 510
CTRL_TABLE_LS                   = 511

CTRL_TREE                       = 512
CTRL_TREE_LS                    = 513

CTRL_HORIZONTAL_SPLITTER        = 520
CTRL_VERTICAL_SPLITTER          = 521
CTRL_HORIZONTAL_SPLITTER_LS     = 522
CTRL_VERTICAL_SPLITTER_LS       = 523

CTRL_TABS                       = 540
    

# RGB colors (for all color attributes)
VAL_RED                         = 0xFF0000
VAL_GREEN                       = 0x00FF00
VAL_BLUE                        = 0x0000FF
VAL_CYAN                        = 0x00FFFF
VAL_MAGENTA                     = 0xFF00FF
VAL_YELLOW                      = 0xFFFF00
VAL_DK_RED                      = 0x800000
VAL_DK_BLUE                     = 0x000080
VAL_DK_GREEN                    = 0x008000
VAL_DK_CYAN                     = 0x008080
VAL_DK_MAGENTA                  = 0x800080
VAL_DK_YELLOW                   = 0x808000
VAL_LT_GRAY                     = 0xC0C0C0
VAL_DK_GRAY                     = 0x808080
VAL_BLACK                       = 0x000000
VAL_WHITE                       = 0xFFFFFF
VAL_PANEL_GRAY                  = VAL_LT_GRAY
VAL_GRAY                        = 0xA0A0A0
VAL_OFFWHITE                    = 0xE0E0E0
VAL_TRANSPARENT                 = 0x1000000


# CVI ERRORS

CviErros = { 0 : "Success",
            -1 : "The Interface Manager could not be opened.",
            -2 : "The system font could not be loaded.",
            -3 : "The operation attempted cannot be performed while a pop-up menu is active.",
            -4 : "Panel, pop-up, menu bar, or plot ID is invalid.",
            -5 : "Attempted to position panel at an invalid location ",
            -6 : "Attempted to make an inoperable control the active control.",
            -7 : "The operation requires that a panel be loaded.",
            -8 : "The operation requires that a pop-up menu be active.",
            -9 : "The operation requires that a menu bar be loaded.",
            -10 : "The control is not the type expected by the function.",
            -11 : "Invalid menu item ID.",
            -12 : "Out of memory! ",
            -13 : "Invalid control ID.",
            -14 : "Value is invalid or out of range.",
            -15 : "File is not a User Interface file or has been corrupted.",
            -16 : "File format is out-of-date.",
            -17 : "PCX image is corrupted or incompatible with current display type.",
            -18 : "No user event possible in current configuration.",
            -19 : "Unable to open UIR file.",
            -20 : "Error reading UIR file.",
            -21 : "Error writing UIR file.",
            -22 : "Error closing UIR file.",
            -23 : "Panel state file has invalid format.",
            -24 : "Invalid panel ID or menu bar ID in resource file.",
            -25 : "Error occurred during hardcopy output.",
            -26 : "Invalid default directory specified in FileSelectPopup function.",
            -27 : "Operation is invalid for specified object.",
            -28 : "Unable to find specified string in menu.",
            -29 : "Palette menu items can only be added to the end of the menu.",
            -30 : "Too many menus in the menu bar.",
            -31 : "Separators cannot have checkmarks.",
            -32 : "Separators cannot have submenus.",
            -33 : "The menu item must be a separator.",
            -34 : "The menu item cannot be a separator.",
            -35 : "The menu item already has a submenu.",
            -36 : "The menu item does not have a submenu.",
            -37 : "The control ID passed must be a menu ID, a menu item ID, or NULL.",
            -38 : "The control ID passed must be a menu ID, or a menu item ID.",
            -39 : "The control ID passed was not a submenu ID.",
            -40 : "The control ID passed was not a valid ID.",
            -41 : "The ID is not a menu bar ID.",
            -42 : "The ID is not a panel ID.",
            -43 : "This operation cannot be performed while this pop-up panel is active.",
            -44 : "This control/panel/menu does not have the specified attribute.",
            -45 : "The control type passed was not a valid type.",
            -46 : "The attribute passed is invalid.",
            -47 : "The fill option must be set to fill above or fill below to paint ring slide's fill color.",
            -48 : "The fill option must be set to fill above or fill below to paint numeric slide's fill color.",
            -49 : "The control passed is not a ring slide.",
            -50 : "The control passed is not a numeric slide.",
            -51 : "The control passed is not a ring slide with inc/dec arrows.",
            -52 : "The control passed is not a numeric slide with inc/dec arrows.",
            -53 : "The data type passed in is not a valid data type for the control.",
            -54 : "The attribute passed is not valid for the data type of the control.",
            -55 : "The index passed is out of range.",
            -56 : "There are no items in the list control.",
            -57 : "The buffer passed was to small for the operation.",
            -58 : "The control does not have a value.",
            -59 : "The value passed is not in the list control.",
            -60 : "The control passed must be a list control.",
            -61 : "The control passed must be a list control or a binary switch.",
            -62 : "The data type of the control passed must be set to a string.",
            -63 : "That attribute is not a settable attribute.",
            -64 : "The value passed is not a valid mode for this control.",
            -65 : "A NULL pointer was passed when a non-NULL pointer was expected.",
            -66 : "The text background color on a menu ring cannot be set or gotten.",
            -67 : "The ring control passed must be one of the menu ring styles.",
            -68 : "Text cannot be colored transparent.",
            -69 : "A value cannot be converted to the specified data type.",
            -70 : "Invalid tab order position for control.",
            -71 : "The tab order position of an indicator-only control cannot be set.",
            -72 : "Invalid number.",
            -73 : "There is no menu bar installed for the panel.",
            -74 : "The control passed is not a text box.",
            -75 : "Invalid scroll mode for chart.",
            -76 : "Invalid image type for picture.",
            -77 : "The attribute is valid for child panels only. Some attributes of top level panels are determined by the host operating system.",
            -78 : "The list control passed is not in check mode.",
            -79 : "The control values could not be completely loaded into the panel because the panel has changed.",
            -80 : "Maximum value must be greater than minimum value.",
            -81 : "Graph does not have that many cursors.",
            -82 : "Invalid plot.",
            -83 : "New cursor position is outside plot area.",
            -84 : "The length of the string exceeds the limit.",
            -85 : "The specified callback function does not have the required prototype.",
            -86 : "The specified callback function is not a known function. For external compilers, the UIR callbacks object file cannot be in the executable or DLL.",
            -87 : "Graph cannot be in this mode without cursors.",
            -88 : "Invalid axis scaling mode for chart.",
            -89 : "The font passed is not in font table.",
            -90 : "The attribute value passed is not valid.",
            -91 : "Too many files are open.",
            -92 : "Unexpectedly reached end of file.",
            -93 : "Input/Output error.",
            -94 : "File not found.",
            -95 : "File access permission denied.",
            -96 : "File access is not enabled.",
            -97 : "Disk is full.",
            -98 : "File already exists.",
            -99 : "File already open.",
            -100 : "Badly formed pathname.",
            -101 : "File is damaged.",
            -102 : "The format of the resource file is too old to read.",
            -103 : "File is corrupted.",
            -104 : "The operation could not be performed.",
            -105 : "The control passed is not a ring knob, dial, or gauge.",
            -106 : "The control passed is not a numeric knob, dial, or gauge.",
            -107 : "The count passed is out of range.",
            -108 : "The keycode is not valid.",
            -109 : "The control passed is not a ring slide with a frame.",
            -110 : "Panel background cannot be colored transparent.",
            -111 : "Title background cannot be colored transparent.",
            -112 : "Not enough memory for printing.",
            -113 : "The shortcut key passed is reserved.",
            -114 : "The format of the file is newer than this version of CVI.",
            -115 : "System printing error.",
            -116 : "Driver printing error.",
            -117 : "The deferred callback queue is full.",
            -118 : "The mouse cursor passed is invalid.",
            -119 : "Printing functions are not reentrant.",
            -120 : "Out of Windows GDI space.",
            -121 : "The panel must be visible.",
            -122 : "The control must be visible.",
            -123 : "The attribute not valid for the type of plot.",
            -124 : "Intensity plots cannot use transparent colors.",
            -125 : "Color is invalid.",
            -126 : "The specified callback function differs only by a leading underscore from another function or variable. Change one of the names for proper linking.",
            -127 : "Bitmap is invalid.",
            -128 : "There is no image in the control.",
            -129 : "The specified operation can be performed only in the thread in which the top-level panel was created.",
            -130 : "The specified panel was not found in the .tui file.",
            -131 : "The specified menu bar was not found in the .tui file.",
            -132 : "The specified control style was not found in the .tui file.",
            -133 : "A tag or value is missing in the .tui file.",
            -134 : "Error reading or parsing .sub file.",
            -135 : "There are no printers installed in the system.",
            -136 : "The beginning cell must be in the search range.",
            -137 : "The cell type passed is not valid for this operation ",
            -138 : "Cell type or data type is mismatched.",
            -139 : "Controls of the type passed do not have a menu.",
            -142 : "You must pass your callback function's eventData2 parameter to this function.",
            -143 : "ActiveX error.",
            -144 : "The specified object handle does not refer to an ActiveX control.",
            -145 : "ActiveX control not registered on this computer.",
            -146 : "ActiveX control does not support persistence.",
            -147 : "The id passed was not a valid menu button id.",
            -148 : "Cannot set or get the attributes of built-in control menu items.",
            -149 : "DataSocket Error.",
            -150 : "Control already has an active data binding.",
            -151 : "Control must have an active data binding.",
            -152 : "The panel to be attached must be a direct child of the panel containing the splitter control.",
            -153 : "Item is already attached to splitter control.",
            -154 : "Item is not attached to splitter control.",
            -155 : "Attached control cannot be sized in this direction.",
            -156 : "Splitter control cannot be attached to itself.",
            -157 : "Operation cannot be performed on a bitmap with a transparency mask.",
            -158 : "Operation cannot be performed on a bitmap with an alpha channel.",
            -159 : "Operation can be performed only on a bitmap with a transparency mask.",
            -160 : "Operation can be performed only on a bitmap with an alpha channel.",
            -161 : "Graph does not have that many annotations.",
            -162 : "Operation cannot be performed on a tab panel.",
            -163 : "The attribute passed is only valid for menu bars.",
            -164 : "The attribute passed is only valid for menu items.",
            -165 : "The attribute passed is only valid for menus and submenus."
            }



