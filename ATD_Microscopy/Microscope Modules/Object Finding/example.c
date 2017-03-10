#include <utility.h>
#include <userint.h>
#include "cellmap_ui.h"
#include "string_utils.h"
#include "cellmap.h"


static void on_cell_found(Cell *cell, void *callback_data)
{
	//printf("Cell Found id=%d type=%d x=%lf y=%lf \n", cell->id, cell->type, cell->x, cell->y);	
	
	Delay(0.5);
	
	return;
}


static void on_map_close(CellMap *map, void *callback_data)
{
	QuitUserInterface(0);
	
	return;
}


int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)  	
{
	CellMap* map;

	if (InitCVIRTE (hInstance, 0, 0) == 0)
		return -1;	// out of memory        

	map = gci_cellmap_new("Cellmap", 50, 50, 400, 400);

	gci_cellmap_close_handler(map, on_map_close, NULL);

	gci_cellmap_load_file(map); 

	gci_cellmap_display(map);

	//gci_cellmap_proprogate_shortest_path(map, on_cell_found, NULL);

	RunUserInterface(); 
	
	gci_cellmap_destroy(map);
	
	return 0;
}
