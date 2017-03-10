#include "microscope_python_wrappers.h"

#include "microscope.h"

static PyObject* microscope_set_mode(PyObject *self, PyObject *args)
{
	int mode;
	
  	Microscope *ms = microscope_get_microscope();

	// Takes 1 integers
    if(!PyArg_ParseTuple(args, "i", &mode))
        return NULL;
	
    if(microscope_set_illumination_mode(ms, mode) == MICROSCOPE_ERROR) {
		PyErr_SetString(PyExc_ValueError, "microscope_set_illumination_mode returned error.");
		return NULL;	
	}
	
	Py_INCREF(Py_None);
	
 	return Py_None;
}


/************************* Microscope_Py_Methods **********************************************/

PyMethodDef Microscope_Py_Methods[] = {
	 
	#ifdef MICROSCOPE_PYTHON_AUTOMATION

	// Timelapse Stuff
	{"GetTimeLapsePoints", microscope_get_all_timelaspe_points, METH_VARARGS,
     "Gets the points defined in the timelapse module."}, 
	
	{"GetTimeLapseHasRegion", microscope_get_timelapse_hasRegion, METH_VARARGS,
     "Returns 1 if the point has a region."}, 

	{"GetTimeLapseRegion", microscope_get_timelapse_region, METH_VARARGS,
     "Gets the region information for the current point."}, 
	
	{"GetFocalPlaneOptions", microscope_get_timelapse_focal_plane, METH_VARARGS,
     "Gets the region focal plane information for the current point."}, 
	
	{"GetCubeOptions", microscope_get_timelapse_cube_options, METH_VARARGS,
     "Gets the cube options for the given cube position."}, 
	
    {"VisitPoints", microscope_visit_timelapse_points, METH_VARARGS,
     "Starts the cycle of visiting the timelapse points in a timelapse script."},
	
	 {"AbortTimeLapseVisitPoints", microscope_abort_timelapse_cycle, METH_VARARGS,
     "Aborts the cycle of visiting the timelapse points in a timelapse script."},
	 
	{"HasTimeLapseBeenAborted", microscope_timelapse_has_been_aborted, METH_VARARGS,
     "Indicates that the timelapse has been aborted."},

	#endif // MICROSCOPE_PYTHON_AUTOMATION

	// Camera stuff

	{"GetMicronsPerPixel", microscope_get_microns_per_pixel, METH_VARARGS,
	  "Returns the microns per pixel of the currently used camera"}, 

	{"GetImage", microscope_get_image, METH_VARARGS,
	  "Returns an FreeImage bitmap opaque python object. Don't use directly"}, 

    {"SetCameraLiveMode", microscope_set_camera_live_mode, METH_VARARGS,
     "Sets the camera to live or snap mode."},
	 
	{"SnapImage", microscope_snap_image, METH_VARARGS,
     "Instructs the camera to take an picture."},
	
	{"SetExposure", microscope_set_exposure, METH_VARARGS,
     "Sets the camera to a specific exposure time in milli seconds."},
	
	{"SetGain", microscope_set_gain, METH_VARARGS,
     "Sets the camera to a specific gain. The passed gain value is camera dependant."
	 "Sets all the camera channels if the camera has more than one channel."},

	{"PerformAutoExposure", microscope_perform_autoexposure, METH_VARARGS,
     "Attempts to calculate a good exposure."},
	 
	// Cubes stuff
	{"GetCubes", microscope_get_cubes, METH_VARARGS,
     "Returns the cubes on the microscope."}, 
	 
	{"MoveCubeToPosition", microscope_move_cube_to_position, METH_VARARGS,
     "Changes the cube position on the microscope."}, 
	 
	{"WaitForCube", microscope_wait_for_cube, METH_VARARGS,
     "Waits for the cubes to stop moving."}, 
	 
	 // Stage Stuff
	 {"GetStagePosition", microscope_get_stage_position, METH_VARARGS,
     "Gets the position of the stage."}, 
	 
	 {"SetStagePosition", microscope_set_stage_position, METH_VARARGS,
     "Sets the position of the stage."}, 
	 
	 {"WaitForStage", microscope_wait_for_stage, METH_VARARGS,
     "Waits for the stage to stop moving."}, 
	 
	 {"SetStageZPosition", microscope_set_stage_z_position, METH_VARARGS,
     "Sets the position of the z stage."}, 
	 
	 {"SetStageRelativeZPosition", microscope_set_stage_z_rel_position, METH_VARARGS,
     "Sets the position of the z stage relative to the current position."}, 

	 // Lamp
	 {"TurnLampOn", microscope_lamp_on, METH_VARARGS,
     "Turns the microscope lamp on."}, 
	 
	 {"TurnLampOff", microscope_lamp_off, METH_VARARGS,
     "Turns the microscope lamp off."}, 
	 
	 {"SetLampIntensity", microscope_lamp_set_intensity, METH_VARARGS,
     "sets the intensity of the lamp on the microscope."}, 
	 
	 // Optical Path
	 {"GetOpticalPaths", microscope_get_opticalpaths, METH_VARARGS,
     "Returns the optical paths on the microscope."}, 
	 
	 {"MoveOpticalPathToPosition", microscope_move_opticalpath_to_position, METH_VARARGS,
     "Changes the optical path on the microscope."}, 
	 
	 // Microscope mode ie fluorescence bright field laser scanning etc
	 {"MicroscopeSetMode", microscope_set_mode, METH_VARARGS,
     "Changes the mode of the microscope. "
	 "Values can be FLUORESCENCE_MODE, BRIGHT_FIELD_MODE, PHASE_CONTRAST_MODE, LASER_SCANNING_MODE, FLUOR_NO_SHUTTER_MODE"}, 
	 
	 // Shutter
	 
	 {"OpenShutter", microscope_shutter_open, METH_VARARGS,
     "Opens the microscope shutter."}, 
	 
	 {"CloseShutter", microscope_shutter_close, METH_VARARGS,
     "Closes the microscope shutter."}, 
	 
	 {"IsShutterOpen", microscope_is_shutter_open, METH_VARARGS,
     "Checks if the microscope shutter is open or closed."}, 
	 
	 {"SetShutterOpenTime", microscope_set_shutter_open_time, METH_VARARGS,
     "Sets the open time of the shutter in milli seconds. 0 is consider infinity."}, 
	
	 
	 // Save Stuff
	 		
	 {"GetUserDataDirectory", microscope_get_user_data_dir, METH_VARARGS,
     "Gets the directory where user data is saved."}, 
	 
	 {"SnapAndSaveImage", microscope_snap_and_save_image, METH_VARARGS,
     "Snaps an Image and then saves it to the desired location."}, 
	 
	 {"SaveCurrentDisplayedImage", microscope_save_current_displayed_image, METH_VARARGS,
     "Saves the currently displayed image to the desired location."}, 

	 {"SaveWidefieldMetadata", microscope_save_widefield_metadata, METH_VARARGS,
     "Saves the current microscope metadata to the desired location."}, 

	 {"ParseSequenceFilename", microscope_parse_file_sequence_value, METH_VARARGS,
     "Parses a sequence filename, substituting in the current date time and sequence number."}, 

	 {"InsertCubeIntoFilename", microscope_insert_cube_into_filename, METH_VARARGS,
     "Takes a cube index and inserts it's name into a filename surrounded by underscores ('_')."}, 

	 {"InsertTextIntoFilename", microscope_insert_text_into_filename, METH_VARARGS,
     "Takes some text and inserts it into a filename surrounded by underscores ('_')."}, 
	 
	 {"ShowFileSequenceSaveDialog", microscope_file_sequence_dialog, METH_VARARGS,
     "Asks the user for the desired filename format."}, 

	 {"ShowSimpleFileSequenceSaveDialog", microscope_simple_file_sequence_dialog, METH_VARARGS,
     "Asks the user for the desired filename prefix."}, 
	 
	 {"PerformSoftwareAutoFocus", microscope_sw_autofocus, METH_VARARGS,
     "Performs a software based auto focus and the snaps and image."}, 
	
	 {"AbortSoftwareAutoFocus", microscope_sw_autofocus_abort, METH_VARARGS,
     "Aborts a software based auto focus and the snaps and image."}, 
	 
	 //{"DisplayBasicWindow", microscope_display_win32_window, METH_VARARGS,
     //"Displays an image in a basic win32 window."}, 
 
	 // Sofware Autofocus
	 
#ifdef BUILD_MODULE_SPC
	 // SPC
	 
	 //{"ShowFlimFileSequenceSaveDialog", microscope_spc_file_sequence_dialog, METH_VARARGS,
     //"Asks the user for the desired filename format for a flim image."}, 
	 
	 {"SpcStartAdvanced", microscope_spc_start, METH_VARARGS,
     "Start the acquisition of a time resolved image."}, 

	 {"SpcStop", microscope_spc_stop, METH_VARARGS,
     "Stops the acquisition of a time resolved image."}, 

	 {"SpcAcquireAndSaveUsingInterfaceValues", spc_wrap_acquire_and_save_to_filename_using_ui_values, METH_VARARGS,
     "Start the acquisition of a time resolved image. Using values from the user interface."}, 
	 
	 {"SpcClearBoardMemory", spc_wrap_clear_board_memory, METH_VARARGS,
     "clears the memory of the spc board."}, 
	 
	 {"SetSpcResolution", microscope_spc_set_adc_res, METH_VARARGS,
     "Sets the ADC resolution of the SPC module."}, 
	 
	 {"SetSpcAcquisitionLimit", microscope_spc_set_acq_limit, METH_VARARGS,
     "Sets the acquisition limit of the SPC module."}, 
#endif

	 // Scanner
#ifdef BUILD_MODULE_SCANNER

	 {"SetScannerProperties", microscope_scanner_set_properties, METH_VARARGS,
	  "Sets the proerties of the scanner module.\nTakes Parameters\nNumber of Frames - Number of frames to scan"}, 
	 
	 {"StartScanner", microscope_scanner_start , METH_VARARGS,
	  "Starts the scanner"}, 
	 
	 {"StopScanner", microscope_scanner_stop , METH_VARARGS,
	  "Stops the scanner"},  

#endif

	  // RegionScan

	 {"RegionScanImpl", wrap_new_regionscan, METH_VARARGS,
	  "Return an opaque regionscan pointer. Don't use this directly in a python script"}, 
	 
	 {"RegionScanStart", regionscan_wrap_start, METH_VARARGS,
	  "Starts a region scan. Don't use this directly in a python script"},

	 {"RegionScanStop", regionscan_wrap_stop, METH_VARARGS,
	  "Stops a region scan. Don't use this directly in a python script"},

	 {"RegionScanHide", regionscan_wrap_hide, METH_VARARGS,
	  "Hides any panels of region scan. Don't use this directly in a python script"},

	 {"RegionScanSetRoi", regionscan_wrap_set_roi, METH_VARARGS,
	  "Sets the roi in microns for a region scan"},

	 {"RegionScanSetFocalPlaneOptions", regionscan_wrap_set_focal_plane, METH_VARARGS,
	  "Sets the focal plane options for a region scan usually from a timelapse region"},

	 {"RegionScanSetFocalPlaneOffset", regionscan_wrap_set_focal_offset, METH_VARARGS,
	  "Sets a focal plane offset for a region scan usually from a timelapse region"},

	 {"RegionScanSetFocalPlaneOffsetFromXYZ", regionscan_wrap_set_focal_offset_from_xyz, METH_VARARGS,
	  "Sets a focal plane offset for a region scan from a new x,y,z corrdinate, usually from a timelapse region"},

	 {"RegionScanSetPerformSwAutofocus", regionscan_wrap_set_perform_swautofocus, METH_VARARGS,
	  "Turns on or off (pass 1 or 0) the software autofocus at every point of the time lapse (default=off)"},

	 {"BackgroundCorrectionEnable", background_correction_enable_wrap, METH_VARARGS,
	  "Enables background correction"},

	 {"BackgroundCorrectionCanProcess", background_correction_can_process_wrap, METH_VARARGS,
	  "Returns true if background correction can be performed"},


	  // CellFinding

	 {"CellFindingImpl", wrap_new_cellfinding, METH_VARARGS,
	  "Return an opaque regionscan pointer. Don't use this directly in a python script"}, 
	
	  /*
	 {"RegionScanStart", regionscan_wrap_start, METH_VARARGS,
	  "Starts a region scan. Don't use this directly in a python script"},

	 {"RegionScanStop", regionscan_wrap_stop, METH_VARARGS,
	  "Stops a region scan. Don't use this directly in a python script"},

	 {"RegionScanSetRoi", regionscan_wrap_set_roi, METH_VARARGS,
	  "Sets the roi in microns for a region scan"},

	 {"BackgroundCorrectionEnable", background_correction_enable_wrap, METH_VARARGS,
	  "Enables background correction"},

	 {"BackgroundCorrectionCanProcess", background_correction_can_process_wrap, METH_VARARGS,
	  "Returns true if background correction can be performed"},
	  */


	// Batch Counter
	
#ifdef BUILD_MODULE_BATCHCOUNTER
	{"BatchCounterStart", microscope_batchcounter_start, METH_VARARGS,
     "Starts the batch counter irradiation with current ui settings."}, 

	 {"BatchCounterReset", microscope_batchcounter_reset, METH_VARARGS,
     "Starts the batch counter irradiation with current ui settings."}, 

	 {"BatchCounterWaitForCounts", microscope_batchcounter_wait_for_counts, METH_VARARGS,
     "Starts the batch counter irradiation with current ui settings."},

	 {"BatchCounterBeamOn", microscope_batchcounter_beamOn, METH_VARARGS,
     "Sets the batch counter mode to beam ON with current ui settings."},

	 {"BatchCounterBeamOff", microscope_batchcounter_beamOff, METH_VARARGS,
     "Sets the batch counter mode to beam OFF with current ui settings."},

	 {"BatchCounterSetCounts", microscope_batchcounter_set_counts, METH_VARARGS,
     "Sets the batch counter mode to a number of counts with current ui settings."},


#endif

    {NULL, NULL, 0, NULL}
};
