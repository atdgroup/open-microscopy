%module Camera
%{
#include "gci_dummy_camera.h"
%}

%module Camera
struct Camera {

  struct camera_operations *_cops;
  
  GCIWindow *_camera_window;
  int _timer;
  unsigned int _colour_type;
  unsigned int _data_mode;
  unsigned int _aquire_mode;
  int _flip;
  int _main_ui_panel;
  int _extra_ui_panel;
  int _powered_up;
  int _average_frames;
  double _exposure_time;
  double _gain;
  double _microns_per_pixel;
  char *_name;
  char *_description;
  char *_data_dir;
  int  _bits;
  int  _trigger_mode;
  
  signal_table signal_table;
  
  void (*_error_handler) (char *error_string, GciCamera* camera);
  
};

%extend Camera {
	
	Camera(void) {
		return GCI_Imaging_GetCamera();
	}

	void Init(void) {
		gci_camera_init(self);
	}
	
	int Destroy(void) {
		return gci_camera_destroy(self);
	}
	
	int SetDataDir(const char *dir) {
		return gci_camera_set_data_dir(self, dir);
	}	

	char* GetDataDir() {
		char dir[500];
		gci_camera_get_data_dir(self, dir); 
		return dir;
	}
	
	int SetDescription(const char *description) {
		return gci_camera_set_description(self, description);
	}
	
	int GetDescription() {
		char description[500];
		gci_camera_get_description(self, description); 
		return description;
	}

	int SetName(const char *name) {
		return gci_camera_set_name(self, name);
	}
	
	int GetName() {
		char name[500];
		gci_camera_get_name(self, name); 
		return name;
	}

	int SetBlackLevel(double blacklevel) {
		return gci_camera_set_blacklevel(self, blacklevel);
	}

	double GetBlackLevel() {
		double bl;
		gci_camera_get_blacklevel(self, &bl); 
		return bl;
	}

	int SetDataMode(DataMode data_mode) {
		return gci_camera_set_data_mode(self, data_mode);
	}

	DataMode GetDataMode() {
		DataMode data_mode;
		gci_camera_get_data_mode(self, &data_mode); 
		return data_mode;
	}

	int SetHighestDataMode() {
		return gci_camera_set_highest_data_mode(self);
	}

	int SetHighestDataMode() {
		return gci_camera_set_highest_data_mode(self);
	}

	int SetLightMode(LightMode light_mode) {
		return gci_camera_set_light_mode(self, light_mode);
	}

	LightMode GetLightMode() {
		LightMode mode;
		gci_camera_get_light_mode(self, &mode);
		return mode;
	}

	int GetDataType() {
		return gci_camera_get_data_type(self);	
	}
	
	int SetAverageFrameNumber(int number) {
		return gci_camera_set_average_frame_number(self, number);
	}

	int GetAverageFrameNumber() {
		int number;
		gci_camera_get_average_frame_number(self, &number);
		return number;
	}

	int PowerUp() {
		return gci_camera_power_up(self);
	}
 
	int PowerDown() {
		return gci_camera_power_down(self);
	}
 
	int IsPoweredUp() {
		return gci_camera_is_powered_up(self);
	}

	int GetColourType() {
		return gci_camera_get_colour_type(self);
	}


/*
int  gci_camera_set_exposure_time(GciCamera* camera, double exposure);
int  gci_camera_set_gain(GciCamera* camera, double gain); 				// Same as mono gain
int  gci_camera_set_mono_gain(GciCamera* camera, double gain);
int  gci_camera_get_min_size(GciCamera* camera, unsigned int *width, unsigned int *height);
int  gci_camera_get_max_size(GciCamera* camera, unsigned int *width, unsigned int *height);

int  gci_camera_set_size_position(GciCamera* camera, unsigned int left, unsigned int top,
	unsigned int width, unsigned int height, unsigned char auto_centre);
													 
int  gci_camera_get_size_position(GciCamera* camera, unsigned int *left, unsigned int *top,
	unsigned int *width, unsigned int *height, unsigned char *auto_centre);

int  gci_camera_get_size(GciCamera* camera, unsigned int *width, unsigned int *height);
int  gci_camera_get_position(GciCamera* camera, unsigned int *left, unsigned int *top, unsigned char *auto_centre);

int  gci_camera_get_info(GciCamera* camera, char *vendor, char *model, char *bus,
	char *camera_id, char *camera_version, char *driver_version);

void gci_camera_set_extra_panel_uir(GciCamera* camera, const char *name);
int  gci_camera_display_main_ui(GciCamera* camera);
int  gci_camera_hide_main_ui(GciCamera* camera);
int  gci_camera_display_extra_ui(GciCamera* camera);
int  gci_camera_hide_extra_ui(GciCamera* camera);
int  gci_camera_destroy_extra_ui(GciCamera* camera);
int  gci_camera_hide_ui(GciCamera* camera);
int  gci_camera_disable_ui(GciCamera* camera, int disable);

int  gci_camera_snap_image(GciCamera* camera);
int  gci_camera_is_live_mode(GciCamera* camera);
int  gci_camera_is_snap_mode(GciCamera* camera);
int  gci_camera_is_snap_sequence_mode(GciCamera* camera);

int  gci_camera_set_live_mode(GciCamera* camera);
int  gci_camera_set_snap_mode(GciCamera* camera);
int  gci_camera_set_snap_sequence_mode(GciCamera* camera);
int  gci_camera_activate_live_display(GciCamera* camera);

int  gci_camera_show_window(GciCamera* camera);

int  gci_camera_set_default_settings(GciCamera* camera);

int  gci_camera_save_settings(GciCamera* camera, const char *filepath);
int  gci_camera_load_settings(GciCamera* camera, const char *filepath);
int  gci_camera_save_settings_as_default(GciCamera* camera);

int  gci_camera_save_state(GciCamera* camera);
int  gci_camera_restore_state(GciCamera* camera);

unsigned long gci_camera_get_exposure_time(GciCamera* camera); 
double gci_camera_get_gain(GciCamera* camera);
double gci_camera_get_mono_gain(GciCamera* camera);

int	gci_camera_set_microns_per_pixel(GciCamera* camera, double factor);
double gci_camera_get_microns_per_pixel(GciCamera* camera);

void gci_camera_set_flip(GciCamera* camera, int flip);


void gci_camera_set_exposure_range(GciCamera* camera, double min, double max, const char *label, int type);
void gci_camera_set_gain_range(GciCamera* camera, double min, double max); 
void gci_camera_set_blacklevel_range(GciCamera* camera, int control, double min, double max);
void gci_camera_disable_gain_control(GciCamera* camera);
void gci_camera_enable_gain_control(GciCamera* camera);

GCIWindow*  gci_camera_get_imagewindow(GciCamera* camera);

FIBITMAP*   gci_camera_get_image(GciCamera* camera, const Rect *rect);
FIBITMAP*   gci_camera_get_image_average(GciCamera* camera); 
FIBITMAP**  gci_camera_get_images(GciCamera* camera, int number_of_images);  
FIBITMAP*   gci_camera_get_image_average_for_frames(GciCamera* camera, int frames);
FIBITMAP*   gci_camera_get_displayed_image(GciCamera* camera);

*/	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
};



