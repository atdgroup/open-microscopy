#ifndef __GCI_UTILS__
#define __GCI_UTILS__

#include <userint.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

/* For reference
icons:
		IDI_APPLICATION Default application icon.
		IDI_ASTERISK Asterisk icon. Same as IDI_INFORMATION.
		IDI_ERROR Hand-shaped icon.
		IDI_EXCLAMATION Exclamation point icon. Same as IDI_WARNING.
		IDI_HAND Hand-shaped icon. Same as IDI_ERROR.
		IDI_INFORMATION Asterisk icon.
		IDI_QUESTION Question mark icon.
		IDI_SHIELD Security Shield icon.
		IDI_WARNINGE xclamation point icon.
		IDI_WINLOGO Default application icon.
*/

// This is an old trick to exchange the values of the variables a and b 
// without using extra space for a temporary variable.
#define SWAP(a,b) (((a) == (b)) || (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))))

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif


#define GCI_MAX_PATHNAME_LEN 500

#define FEED_OK     0x01
#define FEED_CANCEL 0x03

#define GCI_CANCEL_BUTTON 0x01
#define GCI_OK_BUTTON 0x02

typedef DWORD   COLORREF;

typedef struct
{
	char type[500];
	char name[500];

} DeviceInfo;


typedef struct 
{
	int panel_id;
	int percentage;

} ProgressDialog;

#ifndef _CVI_

int GetAllDevices(void);

int IsDeviceConnected(const char* name);

#endif

int gci_thread_pool();

void display_panel_without_activation(int panel_id);

void GCI_SetPanelsMonitorAsDefaultForDialogs(int panel_id);

void GCI_MovePanelToDefaultMonitorForDialogs(int panel_id);

int GCIDialog (int parent, char title[], char * icon, int buttons, char fmt[], ...);

int GCIDialogNoButtons (int parent, char title[], char * icon, char fmt[], ...);

int GCI_MessagePopup (const char title[], const char fmt[], ...);
int GCI_ErrorPopup (const char title[], const char fmt[], ...);

int GCI_ConfirmPopup (char title[], char * icon, char fmt[], ...);

ProgressDialog* DispalyProgressDialog(char *message);

void GetFilenameFromPath(const char *filepath, char *fileName);

void GetDirectoryForFile(const char *filepath, char *directory);

void GetGCIProjectyDirectory(char *directory);

int NewFeedbackDialog(char *title, char *message, int flags);

int NewTimedFeedbackDialog(char *title, char *message, double time);

int FindAndLoadUIR(int parentPanelHandle, char *filename, int panelResourceID);

int LoadIconIntoButton(int PanelID, int ButtonID, char *dir, char *icon_name);

int FindAndLoadIconIntoButton(int PanelID, int ButtonID, char *filename);

int DisplayImageFileAtDirectory(int PanelID, int controlID, const char *dir, char *filename);

int FindAndDisplayImageFile(int PanelID, int controlID, char *filename) ;

int FindPathForFile(const char *filename, char* filepath);

// Search for file name under the system path var
int search_system_path_for_file(const char *filename, char *filepath);

int find_resource_path_relative_to_exe(const char *filename, char *fullpath); 

int find_resource_from_ini_file(const char* name, char *fullpath);

int find_resource(const char *name, char *fullpath);

int get_device_param_from_ini_file(const char* device_name, char *param, unsigned int *value);
int get_device_int_param_from_ini_file(const char* device_name, char *param, int *value);
int get_device_double_param_from_ini_file(const char* device_name, char *param, double *value);
int get_device_string_param_from_ini_file(const char* device_name, char *param, char *value);
int section_from_ini_file_exists(const char* device_name);

//int SmartResourceFind(const char *process_name, const char *resource_name, char *fullpath);

int LessLameFileSelectPopup (int panel_id, char defaultDirectory[], char defaultFileSpec[], char fileTypeList[],
							 char title[], int buttonLabel, int restrictDirectory,
							 int restrictExtension, int allowCancel, int allowMakeDirectory, char pathName[]);

void LogError(const char *device, const char *message);

int GetPrivateDataFolder(char *folder, char *path);

COLORREF CviColourToColorRef(int colour);

int ColorRefToCviColour(COLORREF colour);

int RGBToHSV( unsigned char red, unsigned char green, unsigned char blue,
							double *hue, double *satuation, double *value);

int HSVToRGB( double hue, double satuation, double value,
							unsigned char *red, unsigned char *green, unsigned char *blue);

COLORREF GetComplementaryColour(COLORREF colour);

int GetComplementaryCviColour(int colour);

void SetCtrlTextColourToComplementary(int panel, int ctrl);

void SetCellTextColourToComplementary(int panel, int ctrl, Point cell);

void get_time_string(char *time_str);

void seconds_to_friendly_time(double seconds, char *time);

int get_temp_directory(char *path);

void ShowStandaloneDeviceController(int *index, int *stress, ...);

int ShowStandaloneIniDevicesController(const char * inipath, char *type_filter, int *stress, DeviceInfo *info);

int initialise_comport(int port, int baudrate);

int close_comport(int port);

int CloseComPortWithTimeOut(int portNo, double timeout);

void get_date_as_valid_filepath_str(char *filename);

Rect SetRectRelativeToXY(Rect *rect, int x, int y);

Rect SetRectRelativeToPoint(Rect *rect, Point pt);

int gci_get_debug_level(void);

int GCI_Find_LoadPanel(int parent, const char* filename, int id);

const char* GCI_GetUIRPath(const char* filename, char *full_path);

void gettail_into_array(FILE *fp, unsigned int linenum, unsigned int line_length, char *buffer);

int GCI_formatFloatToString (float n, char *s);

#endif
