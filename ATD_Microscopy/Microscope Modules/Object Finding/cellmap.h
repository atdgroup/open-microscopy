#ifndef __GCI_CELLMAP__
#define __GCI_CELLMAP__

typedef struct _Microscope Microscope;
typedef struct _timelapse timelapse;
typedef struct _GciCamera GciCamera; 

#include "gci_ui_module.h"
#include "signals.h"

#include "cellmap_ui.h"
#include "cell.h"
#include "celllinklist.h"

#include "RegionOfInterest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <userint.h>

////////////////////////////////////////////////////////////////////
// Module to manage cell/object map for the microfocus system
// Glenn Pierce - July 2005
////////////////////////////////////////////////////////////////////
// RJL October 2006
// When you click on an object the stage moves to position the object at the
// image centre and also implements the focal plane.
// Added functions gci_cellmap_current_cell(), gci_cellmap_next_cell(),
// gci_cellmap_get_cell(), gci_cellmap_set_cell_type() and gci_cellmap_first_cell()
////////////////////////////////////////////////////////////////////

#define CELLMAP_SUCCESS 0
#define CELLMAP_ERROR -1

typedef struct
{
  UIModule	parent;
	
  int	parent_panel;
  int   cellinfo_panel;
  int	cellmap_graph_id;
  
  int	top_offset;
  int	left_offset;

  double region_width;
  double region_height;
  double region_top;
  double region_left;
  
  double standard_deviation_x;
  double standard_deviation_y;
  
  char *cell_map_file;
  
  int	 cell_list_invalidated;
  CellLinkedList cell_list;
  
  Cell  current_cell;
  
  int	projected_array_invalidated;
  Cell *projected_cell_array;
 
  int empty;
  
  Microscope *ms;
  region_of_interest *roi; 		

} CellMap;


typedef void (*CELL_EVENT_HANDLER) (Cell *cell, void *data);
typedef void (*MAP_EVENT_HANDLER) (CellMap *map, void *data);

void gci_cellmap_mapClickedHandler(CellMap* map, void (*handler) (int cell_id, void *callback_data), void *data );

CellMap* 	gci_cellmap_new(char *title, int top, int left, int width, int height);
void 		gci_cellmap_destroy(CellMap* map);
void 		gci_cellmap_display(CellMap* map);
void 		gci_cellmap_hide(CellMap* map);
void 		gci_cellmap_close_handler(CellMap* map, MAP_EVENT_HANDLER handler, void *callback_data);
int 		gci_cellmap_add_cell(CellMap *map, Cell cell);
int 		gci_cellmap_current_cell(CellMap *map, int *type);
int 		gci_cellmap_first_cell(CellMap *map);
double 		gci_cellmap_pathlength(CellMap *map);
int 		gci_cellmap_next_cell(CellMap *map);
int 		gci_cellmap_get_cell(CellMap *map, int id);
int 		gci_cellmap_get_cell_type(CellMap *map, int id, int *type);
int 		gci_cellmap_get_nearest_cell(CellMap *map, int id);
int 		gci_cellmap_set_cell_type(CellMap* map, int id, int type);
void 		gci_cellmap_plot_cells(CellMap *map);
void		gci_cellmap_clear(CellMap *map);
void		gci_cellmap_new_file(CellMap *map, char *filepath);
int			gci_cellmap_get_region_size_pos(CellMap *map, double *top, double *left, double *width, double *height);
int			gci_cellmap_set_region_size_pos(CellMap *map, double top, double left, double width, double height);
int			gci_cellmap_set_region(CellMap *map, Rect rect);
Cell		__cdecl gci_cellmap_find_nearest_neighbour(CellMap *map, Cell *key);
void 		gci_cellmap_proprogate_shortest_path(CellMap *map, CELL_EVENT_HANDLER cell_found_handler, void *callback_data);

#endif
