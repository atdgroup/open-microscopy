#include "gci_dummy_camera.h"
#include "uir_files/gci_dummy_camera_ui.h" 
#include "string_utils.h"
#include "gci_utils.h"
#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_Utilities.h"

#include <utility.h>
#include <formatio.h>

//#include <windows.h>
//#include <winbase.h>

// Undef this function we don want the cvi version.
//#undef GetFileTime

/* Function pointers used as virtual functions */
static struct camera_operations om_camera_ops;    

void gci_dummy_set_dummy_file_directory(DummyCamera* om_camera, const char *path)
{
    strcpy(om_camera->om_file_directory, path);
	om_camera->load_type = DUMMY_CAMERA_IMAGE_LOAD_FROM_DIR;
}

static int gci_dummy_camera_power_up(GciCamera* camera)
{
	//printf("Powering up Dummy camera\n");

	gci_camera_set_exposure_time(camera, 10.0);

  	return CAMERA_SUCCESS;
}

static int gci_dummy_camera_get_colour_type (GciCamera* camera)
{
	FIBITMAP *dib = GCI_ImagingWindow_GetDisplayedFIB(camera->_camera_window);

	if(FIA_IsGreyScale(dib))
		return MONO_TYPE;
	else
		return RGB_TYPE;
}

static int gci_dummy_camera_power_down(GciCamera* camera)
{
	//printf("Powering down Dummy camera\n");

	return CAMERA_SUCCESS;
}

static int  gci_dummy_camera_set_exposure_time(GciCamera* camera, double exposure, double *actual_exposure)
{
	//printf("Setting exposure time for Dummy camera to %f\n", exposure);
	*actual_exposure = exposure;

	return CAMERA_SUCCESS;
}


static int  gci_dummy_camera_set_gain(GciCamera* camera, CameraChannel channel, double gain)
{
	//printf("Setting gain for Dummy camera to %f\n", gain);

	return CAMERA_SUCCESS;
}

static int  gci_dummy_camera_get_gain(GciCamera* camera, CameraChannel channel, double *gain)
{
	*gain = 2.0;

	return CAMERA_SUCCESS;
}

static int  gci_dummy_camera_set_live_mode(GciCamera* camera)
{
	//printf("Setting live mode for Dummy camera to live mode.\n");

	return CAMERA_SUCCESS;
}


static int  gci_dummy_camera_set_snap_mode(GciCamera* camera)
{
	//printf("Setting live mode for Dummy camera to snap mode.\n");

	return CAMERA_SUCCESS;
}


static int  gci_dummy_camera_set_snap_sequence_mode(GciCamera* camera)
{
	//printf("Setting live mode for Dummy camera to snap sequence mode.\n");

	return CAMERA_SUCCESS;
}

int ReadLastSpecifiedLoadDirectory(DummyCamera* om_camera, char *directory)
{
	int err, realStringSize;
	char name[UIMODULE_NAME_LEN];
	char reg_key[500];
	GciCamera *camera = (GciCamera*) om_camera;

	gci_camera_get_name(camera, name);

	sprintf(reg_key, "software\\GCI\\Microscope\\Cameras\\%s\\", name);

	err = RegReadString (REGKEY_HKCU, reg_key, "LastSpecifiedLoadDir", directory, 500, &realStringSize);
	
	if(err) {
		strcpy(directory, UIMODULE_GET_DATA_DIR(camera));
	}

	strcpy(om_camera->om_file_directory, directory);

	return err;
}


int CVICALLBACK file_creation_date_sort(void *item1, void *item2)
{
	
	FILETIME file1_createdate, file2_createdate; 

	char *file_str1 = (char *) item1;
	char *file_str2 = (char *) item2;
	
	
	FILE *file1 = fopen(file_str1, "r");
	FILE *file2 = fopen(file_str2, "r");

	GetFileTime(file1, &file1_createdate, NULL, NULL); 
	GetFileTime(file2, &file2_createdate, NULL, NULL);
 
	fclose(file1);
	fclose(file2);

	return !CompareFileTime(&file1_createdate, &file2_createdate);
}


void GetAllImageFilesInDirectoryAsList(GciCamera* camera)
{
	int i, err;
	char filename[GCI_MAX_PATHNAME_LEN] = "", buffer[GCI_MAX_PATHNAME_LEN], spec[GCI_MAX_PATHNAME_LEN], ext[10];
	DummyCamera* om_camera = (DummyCamera*) camera;
	char *extensions = ".ics;.jpg;.jpeg;.png;.tif;.tiff;.gif;.bmp";
	char *tmp = NULL;

	sprintf(spec, "%s\\*", om_camera->om_file_directory);

	if ((err = GetFirstFile(spec, 1, 0, 0, 0, 0, 0, filename)) <= -1) {
		return; // General I0 error
	}	

	if(om_camera->directory_load_files > 0)
		ListDispose(om_camera->directory_load_files);

	om_camera->directory_load_files = ListCreate(sizeof(char) * GCI_MAX_PATHNAME_LEN);

	do
	{
		memset(buffer, 0, GCI_MAX_PATHNAME_LEN);

		get_file_extension(filename, ext);

		// Only a-.llow image files
		if(FindPattern (extensions, 0, -1, ext, 0, 0) < 0)
			continue;

		sprintf(buffer, "%s\\%s", om_camera->om_file_directory, filename); 
		ListInsertItem(om_camera->directory_load_files, buffer, END_OF_LIST);
	}
	while(GetNextFile(filename)==0);

	ListQuickSort (om_camera->directory_load_files, file_creation_date_sort); 

	// Check whether the position is occupied ?
	for(i=1; i <= ListNumItems(om_camera->directory_load_files); i++) {
		
		tmp = (char *) ListGetPtrToItem(om_camera->directory_load_files, i);
		
	//	printf("%s\n", tmp);
	}
}

static FIBITMAP* gci_dummy_load_file_from_dir(GciCamera* camera)
{
	DummyCamera* om_camera = (DummyCamera*) camera;
	FIBITMAP *dib = NULL;
	char filepath[GCI_MAX_PATHNAME_LEN];

	ListGetItem(om_camera->directory_load_files, filepath, om_camera->load_from_directory_count);

	om_camera->load_from_directory_count++;

	// Recycle images
	if(om_camera->load_from_directory_count > ListNumItems(om_camera->directory_load_files))
		om_camera->load_from_directory_count = 1;

	if(has_file_extension(filepath, ".ics"))
		dib = FreeImageIcs_LoadFIBFromIcsFilePath(filepath);  	
	else
		dib = FIA_LoadFIBFromFile(filepath);

	if (dib) SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_LASTFILE, filepath); 

	return dib;
}

static FIBITMAP*  gci_dummy_camera_get_image(GciCamera* camera, const Rect *rect)
{
	DummyCamera* om_camera = (DummyCamera*) camera;
	FIBITMAP *dib = NULL;
	char datadir[GCI_MAX_PATHNAME_LEN]= "",fullpath[GCI_MAX_PATHNAME_LEN] = "";
    int mode = gci_camera_get_data_mode(camera); 
	
	// Testing power monitor
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "TriggerNow", GCI_VOID_POINTER, camera);    

	if(om_camera->load_type == DUMMY_CAMERA_IMAGE_LOAD_SPECIFIC_FILE) {
		// The client may have has a specific file set
		if(has_file_extension(om_camera->filepath, ".ics"))
			dib = FreeImageIcs_LoadFIBFromIcsFilePath(om_camera->filepath);  	
		else
			dib = FIA_LoadFIBFromFile(om_camera->filepath);
		if (dib) SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_LASTFILE, om_camera->filepath); 
	}
	else if(om_camera->load_type == DUMMY_CAMERA_IMAGE_LOAD_FROM_DIR) {
		// We look for our images in this folder.
		dib = gci_dummy_load_file_from_dir(camera);	
	}
	else {

		if(gci_camera_is_live_mode(camera)) {
		
			if(mode == BPP12)
				sprintf(fullpath, "%s\\om_12bit.png", om_camera->om_file_directory);
			else
				sprintf(fullpath, "%s\\om_live_image.tif", om_camera->om_file_directory);
		}
		else {
			if(mode == BPP12)
				sprintf(fullpath, "%s\\om_12bit.png", om_camera->om_file_directory);
			else
				sprintf(fullpath, "%s\\om_image.tif", om_camera->om_file_directory);
		}

		dib = FIA_LoadFIBFromFile(fullpath); 
		if (dib) SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_LASTFILE, fullpath); 
	}

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "PostCapture", GCI_VOID_POINTER, om_camera);   
	
	return dib;
}


static FIBITMAP *  gci_dummy_camera_get_image_average(GciCamera* camera, int frames)
{
	//printf("Returning image average for Dummy camera\n");

	return NULL;
}


static int  gci_dummy_camera_get_min_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = 100;
	*height = 100;

	//printf("returning camera min size\n");

	return CAMERA_SUCCESS;
}


static int  gci_dummy_camera_get_max_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = 1024;
	*height = 1200;

	//printf("returning camera max size\n");

	return CAMERA_SUCCESS;
}


static int gci_dummy_camera_get_info(GciCamera* camera, char *vendor, char *model, char *bus, char *camera_id, char *camera_version, char *driver_version)
{
	if(vendor != NULL)
		strcpy(vendor, "Dummy Vendor");
	
	if(model != NULL)
		strcpy(model,"Dummy Model");
	
	if(bus != NULL)
		strcpy(bus,"Dummy Bus");
	
	if(camera_id != NULL)
		strcpy(camera_id,"Dummy camera id");
	
	if(camera_version)
		strcpy(camera_version,"1.03");
	
	if(driver_version)
		strcpy(driver_version,"2.78");
	
	return CAMERA_SUCCESS;
}


static int gci_dummy_camera_set_trigger_mode(GciCamera* camera, TriggerMode trig_mode)
{
	//printf("Set data mode for dummy camera\n");
	
	return CAMERA_SUCCESS;
}
	
static int gci_dummy_camera_set_data_mode(GciCamera* camera, DataMode data_mode)
{
	//printf("Set data mode for dummy camera\n");
	
	return CAMERA_SUCCESS;
}


int gci_dummy_camera_destroy(GciCamera* camera)
{
	if(camera == NULL)
		return CAMERA_ERROR;

  	return CAMERA_SUCCESS;
}


int gci_dummy_get_size_position(GciCamera* camera, unsigned int *left, unsigned int *top,
												 unsigned int *width, unsigned int *height, unsigned char *auto_centre)
{
	FIBITMAP* dib = gci_dummy_camera_get_image(camera, NULL);

	*left = 0;
	*top = 0;
	*width = FreeImage_GetWidth(dib);
	*height = FreeImage_GetHeight(dib);
	*auto_centre = 0;

	FreeImage_Unload(dib);

	return CAMERA_SUCCESS;
}

static int gci_dummy_save_settings (GciCamera* camera, const char *filepath, const char *mode)
{
	return CAMERA_SUCCESS;
}

static int gci_dummy_load_settings (GciCamera* camera, const char *filepath) 
{
	return CAMERA_SUCCESS;
}

static int gci_dummy_camera_save_state(GciCamera* camera, CameraState *state)
{
	state->data_mode = camera->_data_mode;
	state->exposure_time = camera->_exposure_time;
    state->gain1 = camera->_gain;
	state->live = gci_camera_is_live_mode(camera);

	state->binning = NO_BINNING;
	state->bpp = 8;
	state->dual_tap = 0;
	state->blacklevel1 = 0;
	state->blacklevel2 = 0;
	state->sensitivity = 0;
	state->gain2 = 0;
	state->light_mode = 0;
	state->photon_mode = 0;
	state->trigger_mode = CAMERA_NO_TRIG;
	
	return CAMERA_SUCCESS;
}

static int gci_dummy_camera_restore_state(GciCamera* camera, CameraState *state)
{
	gci_camera_set_snap_mode(camera);
	gci_camera_set_data_mode(camera, state->data_mode);
	gci_camera_set_exposure_time(camera, state->exposure_time);
	gci_camera_set_gain(camera, CAMERA_ALL_CHANNELS, state->gain1);

	if(state->live) {
		gci_camera_set_live_mode(camera);
		gci_camera_activate_live_display(camera);
	}
		
	return CAMERA_SUCCESS;
}

static int gci_dummy_camera_set_default_settings (GciCamera* camera)
{
	gci_camera_set_data_mode(camera, BPP12);

	return CAMERA_SUCCESS; 
}

static int gci_dummy_camera_get_highest_datamode (GciCamera* camera, DataMode *data_mode)  
{
	*data_mode = BPP12;
	
	return CAMERA_SUCCESS; 
}

static int gci_dummy_camera_get_lowest_datamode (GciCamera* camera, DataMode *data_mode)  
{
	*data_mode = BPP8;
	
	return CAMERA_SUCCESS; 
}

static double gci_dummy_camera_get_fps(GciCamera* camera)
{
	return 1.0;
}

void gci_dummy_set_display_file_path(DummyCamera* om_camera, const char *path)
{
	om_camera->load_type = DUMMY_CAMERA_IMAGE_LOAD_SPECIFIC_FILE;
	strcpy(om_camera->filepath, path);
}

static int gci_dummy_camera_initialise (GciCamera* camera)
{
	DummyCamera* om_camera = (DummyCamera*) camera;
	char *buffer[GCI_MAX_PATHNAME_LEN];

	om_camera->load_from_directory_count = 1;

	gci_camera_set_extra_panel_uir(camera, "gci_dummy_camera_ui.uir", EXTRA_PNL); 

  	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_LOAD, OnDummyCameraSetLoadDirectory, camera) < 0)
		return CAMERA_ERROR;
  	
	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_QUIT, OnDummyCameraOnExtraPanelClose, camera) < 0)
		return CAMERA_ERROR;

	if(ReadLastSpecifiedLoadDirectory(om_camera, buffer) == 0) {

		om_camera->load_type = DUMMY_CAMERA_IMAGE_LOAD_FROM_DIR;
	}

	GetAllImageFilesInDirectoryAsList(camera);

	return CAMERA_SUCCESS;
}

GciCamera* gci_dummy_camera_new(const char *name, const char* description)
{
	DummyCamera* om_camera = (DummyCamera*) malloc(sizeof(DummyCamera));
	GciCamera *camera = (GciCamera*) om_camera;

	memset(om_camera, 0, sizeof(DummyCamera));

	gci_camera_constructor(camera, name, description);

	om_camera->load_type = DUMMY_CAMERA_IMAGE_LOAD_DEFAULT;

	gci_camera_set_min_size(camera, 32, 8);
	gci_camera_set_max_size(camera, 1344, 1024);
	
	CAMERA_VTABLE_PTR(camera).initialise = gci_dummy_camera_initialise;
	CAMERA_VTABLE_PTR(camera).set_default_settings = gci_dummy_camera_set_default_settings;
	CAMERA_VTABLE_PTR(camera).destroy = gci_dummy_camera_destroy;
	CAMERA_VTABLE_PTR(camera).power_up = gci_dummy_camera_power_up;
	CAMERA_VTABLE_PTR(camera).power_down = gci_dummy_camera_power_down;
	CAMERA_VTABLE_PTR(camera).set_exposure_time = gci_dummy_camera_set_exposure_time;
	CAMERA_VTABLE_PTR(camera).set_gain = gci_dummy_camera_set_gain; 
	CAMERA_VTABLE_PTR(camera).get_gain = gci_dummy_camera_get_gain;
	CAMERA_VTABLE_PTR(camera).set_live_mode = gci_dummy_camera_set_live_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_mode = gci_dummy_camera_set_snap_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_sequence_mode = gci_dummy_camera_set_snap_sequence_mode;
	CAMERA_VTABLE_PTR(camera).get_image = gci_dummy_camera_get_image; 
	CAMERA_VTABLE_PTR(camera).get_image_average = gci_dummy_camera_get_image_average; 
	CAMERA_VTABLE_PTR(camera).get_size_position = gci_dummy_get_size_position;
	CAMERA_VTABLE_PTR(camera).get_info =	gci_dummy_camera_get_info;
	CAMERA_VTABLE_PTR(camera).set_datamode =	gci_dummy_camera_set_data_mode;
	CAMERA_VTABLE_PTR(camera).set_trigger_mode =	gci_dummy_camera_set_trigger_mode; 
	CAMERA_VTABLE_PTR(camera).save_settings = gci_dummy_save_settings;
	CAMERA_VTABLE_PTR(camera).load_settings = gci_dummy_load_settings;
	CAMERA_VTABLE_PTR(camera).save_state = gci_dummy_camera_save_state;
	CAMERA_VTABLE_PTR(camera).restore_state = gci_dummy_camera_restore_state;
	CAMERA_VTABLE_PTR(camera).get_colour_type = gci_dummy_camera_get_colour_type;
	//CAMERA_VTABLE_PTR(camera).get_fps = gci_dummy_camera_get_fps;   
	CAMERA_VTABLE_PTR(camera).get_highest_datamode = gci_dummy_camera_get_highest_datamode;
	CAMERA_VTABLE_PTR(camera).get_lowest_datamode = gci_dummy_camera_get_lowest_datamode;
	
	return camera;
}



