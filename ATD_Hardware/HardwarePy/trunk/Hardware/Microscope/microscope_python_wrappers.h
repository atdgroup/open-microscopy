#include "Python.h" 

extern PyMethodDef Microscope_Py_Methods[];

// Camera

PyObject* microscope_set_camera_live_mode(PyObject *self, PyObject *args);
PyObject* microscope_snap_image(PyObject *self, PyObject *args);
PyObject* microscope_set_exposure(PyObject *self, PyObject *args);
PyObject* microscope_set_gain(PyObject *self, PyObject *args);
PyObject* microscope_perform_autoexposure(PyObject *self, PyObject *args);
PyObject* microscope_get_microns_per_pixel(PyObject *self, PyObject *args);
PyObject* microscope_get_image(PyObject *self, PyObject *args);
PyObject* microscope_save_current_displayed_image(PyObject *self, PyObject *args);
PyObject* microscope_snap_and_save_image(PyObject *self, PyObject *args);
PyObject* microscope_save_widefield_metadata(PyObject *self, PyObject *args);

// TimeLapse

PyObject* microscope_visit_timelapse_points(PyObject *self, PyObject *args);
PyObject* microscope_get_timelapse_region(PyObject *self, PyObject *args);
PyObject* microscope_get_timelapse_hasRegion(PyObject *self, PyObject *args);
PyObject* microscope_get_timelapse_focal_plane(PyObject *self, PyObject *args);
PyObject* microscope_get_timelapse_cube_options(PyObject *self, PyObject *args);
PyObject* microscope_abort_timelapse_cycle(PyObject *self, PyObject *args);
PyObject* microscope_timelapse_has_been_aborted(PyObject *self, PyObject *args);
PyObject* microscope_get_all_timelaspe_points(PyObject *self, PyObject *args);
// MJM -- Need to 'add point'
PyObject* microscope_add_timelapse_point(PyObject *self, PyObject *args);
// Cubes

PyObject* microscope_get_cubes(PyObject *self, PyObject *args);
PyObject* microscope_move_cube_to_position(PyObject *self, PyObject *args);
PyObject* microscope_wait_for_cube(PyObject *self, PyObject *args);


// Stage

PyObject* microscope_get_stage_position(PyObject *self, PyObject *args);
PyObject* microscope_set_stage_position(PyObject *self, PyObject *args);
PyObject* microscope_wait_for_stage(PyObject *self, PyObject *args);
PyObject* microscope_set_stage_z_position(PyObject *self, PyObject *args);
PyObject* microscope_set_stage_z_rel_position(PyObject *self, PyObject *args);

// OpticalPath

PyObject* microscope_get_opticalpaths(PyObject *self, PyObject *args);
PyObject* microscope_move_opticalpath_to_position(PyObject *self, PyObject *args);

// Shutter

PyObject* microscope_shutter_open(PyObject *self, PyObject *args);
PyObject* microscope_shutter_close(PyObject *self, PyObject *args);
PyObject* microscope_is_shutter_open(PyObject *self, PyObject *args);
PyObject* microscope_set_shutter_open_time(PyObject *self, PyObject *args);

// SPC

PyObject* spc_wrap_clear_board_memory(PyObject *self, PyObject *args);
PyObject* microscope_spc_start(PyObject *self, PyObject *args);
PyObject* spc_wrap_acquire_and_save_to_filename_using_ui_values(PyObject *self, PyObject *args);
PyObject* microscope_spc_set_adc_res(PyObject *self, PyObject *args);
PyObject* microscope_spc_set_acq_limit(PyObject *self, PyObject *args);
PyObject* microscope_spc_stop(PyObject *self, PyObject *args);

// Scanner

PyObject* microscope_scanner_set_properties(PyObject *self, PyObject *args);
PyObject* microscope_scanner_start(PyObject *self, PyObject *args);
PyObject* microscope_scanner_stop(PyObject *self, PyObject *args);


// ZDrive

PyObject* microscope_zdrive_set_pos(PyObject *self, PyObject *args);

// RegionScan

PyObject *wrap_new_regionscan(PyObject *self, PyObject* args);
PyObject *regionscan_wrap_start(PyObject *self, PyObject* args);
PyObject *regionscan_wrap_set_roi(PyObject *self, PyObject* args);
PyObject *regionscan_wrap_set_focal_plane(PyObject *self, PyObject* args);
PyObject *regionscan_wrap_set_focal_offset(PyObject *self, PyObject* args);
PyObject *regionscan_wrap_set_focal_offset_from_xyz(PyObject *self, PyObject* args);
PyObject *regionscan_wrap_stop(PyObject *self, PyObject* args);
PyObject *regionscan_wrap_hide(PyObject *self, PyObject* args);
PyObject *regionscan_wrap_set_perform_swautofocus(PyObject *self, PyObject* args);

// CellFinding

PyObject *wrap_new_cellfinding(PyObject *self, PyObject* args);

// Background Correction

PyObject *background_correction_enable_wrap(PyObject *self, PyObject* args);
PyObject *background_correction_can_process_wrap(PyObject *self, PyObject* args);

// Lamp

PyObject* microscope_lamp_on(PyObject *self, PyObject *args);
PyObject* microscope_lamp_off(PyObject *self, PyObject *args);
PyObject* microscope_lamp_set_intensity(PyObject *self, PyObject *args);

// Dialogs

PyObject* microscope_get_user_data_dir(PyObject *self, PyObject *args);
PyObject* microscope_file_sequence_dialog(PyObject *self, PyObject *args);
PyObject* microscope_simple_file_sequence_dialog(PyObject *self, PyObject *args);
PyObject* microscope_parse_file_sequence_value(PyObject *self, PyObject *args);
PyObject* microscope_display_win32_window(PyObject *self, PyObject *args);
PyObject* microscope_insert_text_into_filename(PyObject *self, PyObject *args);
PyObject* microscope_insert_cube_into_filename(PyObject *self, PyObject *args);

// Software Autofocus

PyObject* microscope_sw_autofocus(PyObject *self, PyObject *args);
PyObject* microscope_sw_autofocus_abort(PyObject *self, PyObject *args);

// Batch Counter

PyObject* microscope_batchcounter_start(PyObject *self, PyObject *args);
PyObject* microscope_batchcounter_reset(PyObject *self, PyObject *args);
PyObject* microscope_batchcounter_wait_for_counts(PyObject *self, PyObject *args);


// Added by MJM 14/01/2010
PyObject* microscope_batchcounter_set_counts(PyObject *self, PyObject *args);
PyObject* microscope_batchcounter_beamOn(PyObject *self, PyObject *args);
PyObject* microscope_batchcounter_beamOff(PyObject *self, PyObject *args);


//Added MJM 02/03/2010
//PyObject * microscope_batchcounter_read_counts(PyObject *self, PyObject *args);
