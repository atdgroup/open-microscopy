#include "cvixml.h"

#include <utility.h>
#include "fluorescent_cubes.h"
#include "fluorescent_cubes_ui.h"

#include "string_utils.h" 
#include "xml_utils.h" 

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Fluorescent cube data control.
////////////////////////////////////////////////////////////////////////////

int cube_manager_save_cube_data(FluoCubeManager* cube_manager, const char *filepath)
{
	int i, fsize, list_size;
	CVIXMLElement root = -1, cubes, cube, el=-1;
	CVIXMLDocument FluorecentCubesDocument = -1;
	FluoCube temp_cube;
	char buffer[60];
	
	list_size = ListNumItems (cube_manager->_list); 
	
	//Save all cube settings in xml format
	if (CVIXMLNewDocument ("Cubesettings", &FluorecentCubesDocument))
		return CUBE_MANAGER_ERROR;
		
	if (CVIXMLGetRootElement (FluorecentCubesDocument, &root))
		goto Error;	
	
	// cubes 
	if (CVIXMLNewElement (root, -1, "cubes", &cubes))
		goto Error;
	
	for(i=1; i <= list_size; i++) {
	
		ListGetItem (cube_manager->_list, &temp_cube, i);  
	
		// cube
		if (CVIXMLNewElement (cubes, -1, "FluoCube", &cube))
			goto Error;
	
	
		// Name
		if (newXmlSettingStr (cube, "Name", temp_cube.name) < 0)
			goto Error;

		// Position
		if (newXmlSettingInt (cube, "Position", temp_cube.position) < 0)
			goto Error;
	
		// Exc-NM
		if (newXmlSettingInt (cube, "Exc-NM", temp_cube.exc_nm) < 0)
			goto Error;
	
		// Dichroic NM
		if (newXmlSettingInt (cube, "Dichroic-NM", temp_cube.dichroic_nm) < 0)
			goto Error;
	
		// Em-min-NM
		if (newXmlSettingInt (cube, "Em-min-NM", temp_cube.emm_min_nm) < 0)
			goto Error;
	
		// Em-max-NM
		if (newXmlSettingInt (cube, "Em-max-NM", temp_cube.emm_max_nm) < 0)
			goto Error;
	
		CVIXMLDiscardElement(cube);
	}
	
	
	CVIXMLDiscardElement(cubes);
	
	// Default position
	if (CVIXMLNewElement (root, -1, "Default", &el))
		goto Error;
	str_itoa(cube_manager->_default_pos, buffer);
	if (CVIXMLSetElementValue (el, buffer))
	CVIXMLDiscardElement(el); 
	
	if (FileExists(filepath, &fsize))
		SetFileAttrs (filepath, 0, -1, -1, -1);   //clear read only flag
	
	if (CVIXMLSaveDocument (FluorecentCubesDocument, 1, filepath))
		goto Error;
	
	CVIXMLDiscardElement(root);
	CVIXMLDiscardDocument (FluorecentCubesDocument);
	SetFileAttrs (filepath, 1, -1, -1, -1);   //set read only flag
	
	return CUBE_MANAGER_SUCCESS;
	

	Error:
	
		if (el >= 0)
			CVIXMLDiscardElement(el);
		
		if (root >= 0)
			CVIXMLDiscardElement(root);
		
		if (FluorecentCubesDocument >= 0)
			CVIXMLDiscardDocument(FluorecentCubesDocument);

		return CUBE_MANAGER_ERROR;
}



int cube_manager_load_cube_file(FluoCubeManager* cube_manager, const char *filepath)
{
	int i, number_of_possible_cubes;
	CVIXMLElement root = -1, cubes_element=-1, element=-1, cube_element=-1;
	CVIXMLDocument FluorecentCubesDocument = -1;
	FluoCube cube; 
	char default_pos[10];
	
	cube_manager->number_of_present_cubes = 0;
	
	if (openXmlRoot ((char *)filepath, "Cubesettings", &FluorecentCubesDocument, &root) < 0)
		return CUBE_MANAGER_ERROR;

	ListClear(cube_manager->_list);  
	
	if(ClearListCtrl (cube_manager->_cube_table_panel, CUBE_CONF_ALL_TREE) < 0)
		return CUBE_MANAGER_ERROR;

	SetCtrlVal(cube_manager->_cube_table_panel, CUBE_CONF_FNAME, filepath);

	// cubes
	if (CVIXMLGetChildElementByTag (root, "cubes", &cubes_element))
		goto Error;
	CVIXMLGetNumChildElements (cubes_element, &number_of_possible_cubes );
	
	for(i=0; i < number_of_possible_cubes; i++) {
	
		cube.id = i + 1;
		
		if (CVIXMLGetChildElementByIndex (cubes_element, i, &cube_element))
			goto Error;
	
		// Name
		if (getXmlSettingStr (cube_element, "Name", cube.name, 10) < 0)
			goto Error;
		
		// Position
		if (getXmlSettingInt (cube_element, "Position", &cube.position) < 0)
			goto Error;
	
		if(cube.position > 0)
			cube_manager->number_of_present_cubes++;
	
		// Exc-NM
		if (getXmlSettingInt (cube_element, "Exc-NM", &cube.exc_nm) < 0)
			goto Error;
		
		// Dichroic-NM
		if (getXmlSettingInt (cube_element, "Dichroic-NM", &cube.dichroic_nm) < 0)
			goto Error;
		
		// Em-min-NM
		if (getXmlSettingInt (cube_element, "Em-min-NM", &cube.emm_min_nm) < 0)
			goto Error;
		
		// Em-max-NM
		if (getXmlSettingInt (cube_element, "Em-max-NM", &cube.emm_max_nm) < 0)
			goto Error;
		
		ListInsertItem (cube_manager->_list, &cube, END_OF_LIST);
	
		CVIXMLDiscardElement(cube_element); 
	
	}
	
	CVIXMLDiscardElement(cubes_element);
	
	// Default position
	if (CVIXMLGetChildElementByTag (root, "Default", &element) == 0) {
		CVIXMLGetElementValue (element, default_pos);
		CVIXMLDiscardElement(element);
		cube_manager->_default_pos = atoi(default_pos); 
	}
	
	cube_manager_load_all_possible_cubes_into_ui(cube_manager);

	return CUBE_MANAGER_SUCCESS;
	

	Error:
	
		if (element >= 0)
			CVIXMLDiscardElement(element);
			
		if (cubes_element >= 0)
			CVIXMLDiscardElement(cubes_element);  
			
		if (root >= 0)
			CVIXMLDiscardElement(root);
		
		if (FluorecentCubesDocument >= 0)
			CVIXMLDiscardDocument(FluorecentCubesDocument);

		return CUBE_MANAGER_ERROR;
}
