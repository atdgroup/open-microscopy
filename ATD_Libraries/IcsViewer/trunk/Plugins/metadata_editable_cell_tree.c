/******************************************************************************
This is an example program that shows how you can use a string control to edit
tree control cells that aren't in the first column.
Double click on the cell to edit the cell string.
Limitations:
- Doesn't work well when ATTR_SIZE_MODE of the tree item is 
	VAL_USE_EXPLICIT_SIZE or when a tree row has a cell with text that is a 
	different size than other cells on that row.
*******************************************************************************/

#include "icsviewer_uir.h"
#include "metadata_plugin.h"

#define kCornerRight  (1<<0)
#define kCornerBottom (1<<1)

void 	ConvertToEditableTreeCells (int panel, int tree, int string);
void 	EditTreeCell (int panel, int tree, int string, int index, int col);
void 	EndTreeCellEdit (int panel, int tree, int string);
void 	UpdateStringCtrl (int panel, int tree, int string, int index, int col, int toString);
int 	GetTreeColumnLeft (int panel, int tree, int col);
int 	GetTreeItemTop (int panel, int tree, int index);
int 	GetStringWidthForColumn (int panel, int tree, int col, int colLeft);


