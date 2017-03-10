#ifndef __WHITE_LIGHT_LASER__
#define __WHITE_LIGHT_LASER__ 

#include "laser.h"
#include "FTDI_Utils.h"

typedef enum {FIANIUM_SC450_4, FIANIUM_SC450_M} FianiumLaserType;

typedef enum {WL_MANUAL_MODE, WL_COMPUTER_MODE, WL_EXTERNAL_MODE} WHITE_LIGHT_LASER_MODE;

#define WL_ERROR -1
#define WL_SUCCESS 0

typedef int (*GET_DAC_VALUE_FUNC_PTR) (Laser* laser, int *value); 

typedef struct _WhiteLightLaser
{
    Laser    parent;

	FianiumLaserType	type;

	int		 _destroying;
	int		 _panel_id;
	int		 _extra_panel;
	int		 _initialised;
	int	 	 _com_port;
	int		 _lock;
    int      _timer;
	int		 _log_timer;
    int      _last_amp_value;
	int		 _nErrors;

	char	 _version[100];

	char				device_sn[FTDI_SERIAL_NUMBER_LINE_LENGTH];
	FTDIController*		controller;

	FILE	*_log_fp;

	GET_DAC_VALUE_FUNC_PTR	get_dac_func_ptr;

} WhiteLightLaser;

Laser* whitelight_laser_new(FianiumLaserType type, const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

Laser* whitelight_laser_sc450_4_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

Laser* whitelight_laser_sc450_m_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

int whitelight_laser_destroy (Laser* laser);

int whitelight_laser_hardware_init(Laser* laser);

int whitelight_laser_init(Laser* laser);

int whitelight_laser_is_initialised(Laser* laser);

int whitelight_laser_display_panel(Laser* laser);

int whitelight_laser_hide_panel(Laser* laser);

void whitelight_laser_start_display_timer(Laser* laser);

void whitelight_laser_stop_display_timer(Laser* laser);

int whitelight_laser_set_mode(Laser* laser, WHITE_LIGHT_LASER_MODE mode, int wait);

int whitelight_laser_get_mode(Laser* laser, WHITE_LIGHT_LASER_MODE *mode);

int whitelight_laser_get_back_reflection(Laser* laser, int *value);

int whitelight_laser_get_pre_amp_diode_val(Laser* laser, int *value);

int whitelight_laser_get_running_time_string(Laser* laser, char *value);

int whitelight_laser_is_oscillator_ready(Laser* laser);

int whitelight_laser_turn_on(Laser* laser);

int whitelight_laser_turn_off(Laser* laser);

int whitelight_laser_set_dac(Laser* laser, int value);

int whitelight_laser_get_dac(Laser* laser, int *value);

int whitelight_laser_get_dac_version_509(Laser* laser, int *value);

int whitelight_laser_get_info(Laser* laser, char *info);

#endif
