%module FluoCubes
%{
#include "gci_dummy_camera.h"
%}

%module FluoCube
struct FluoCube {

	char	name[10];
	int		id;
	int		position;
	int		exc_nm;
	int		dichroic_nm;
	int		emm_min_nm;
	int		emm_max_nm;
};

%module FluoCubeManager
struct FluoCubeManager {
 
  CAObjHandle hCube;
  FluoCubeManagerVtbl *lpVtbl;
 
  char  	*_name;
  char  	*_description;
  char 		*_data_file;
 
  ListType	 _list;
  
  int		 number_of_present_cubes;
  int		 _required_position;
  int		 _current_position;
  int		 _default_pos;
  int	 	 _i2c_port;
  int		 _i2c_bus;
  int		 _i2c_address;
  int 	 	 _lock;
  int     	 _timer;
  int	 	 _main_ui_panel;
  int	 	 _cube_table_panel;
  int	 	 _details_ui_panel;
 
  int		 _mounted;
  int		 _data_modified;
  
  signal_table signal_table;
  
  void (*_error_handler) (char *error_string, FluoCubeManager* cube_manager);  
};


%extend FluoCubeManager
{	
	FluoCubeManager() {
		Microscope* ms = get_microscope();
		return get_fluor_cube(ms);
	}
	
	int GetDescription() {
		char description[500];
		cube_manager_get_description(self, description); 
		return description;
	}
	
	int GetName() {
		char name[500];
		cube_manager_get_name(self, name); 
		return name;
	}

	int GotoDefaultPosition()
	{
		return cube_manager_goto_default_position(self);
	}

	int GetNumberOfCubes()
	{
		int number;
		cube_manager_get_number_of_cubes(self, &number);
		return number;
	}

	int GetCurrentCubePosition()
	{
		int pos;
		cube_manager_get_current_cube_position(self, &pos);
		return pos;
	}

/*
	int GetCurrentCube()
	{
		int pos;
		cube_manager_get_current_cube_position(self, &pos);
		return pos;
	}
	

int  cube_manager_get_current_cube(FluoCubeManager* cube_manager, FluoCube *cube);
int  cube_manager_get_cube_for_position(FluoCubeManager* cube_manager, int position, FluoCube* dst_cube);
int  cube_manager_move_to_position(FluoCubeManager* cube_manager, int position);
int  cube_manager_move_to_empty_position(FluoCubeManager* cube_manager);

int  cube_manager_display_main_ui (FluoCubeManager* cube_manager);
int  cube_manager_hide_main_ui (FluoCubeManager* cube_manager);
int  cube_manager_is_main_ui_visible (FluoCubeManager* cube_manager);
*/
	
	
	
	
	
	
	
	
	
	
	
};



