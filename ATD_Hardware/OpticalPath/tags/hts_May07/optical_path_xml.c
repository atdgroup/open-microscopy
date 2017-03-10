#include "cvixml.h"

#include <utility.h>
#include "optical_path.h"
#include "optical_path_ui.h"

#include "string_utils.h" 
#include "xml_utils.h" 

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Optical path data control.
////////////////////////////////////////////////////////////////////////////

int optical_path_manager_save_optical_path_data(OpticalPathManager* optical_path_manager, const char *filepath)
{
	int i, fsize, list_size;
	CVIXMLElement root = -1, optical_paths, optical_path, el=-1;
	CVIXMLDocument OpticalPathDocument = -1;
	OpticalPath temp_optical_path;
	
	list_size = ListNumItems (optical_path_manager->_list); 
	
	//Save all optical_path settings in xml format
	if (CVIXMLNewDocument ("OpticalPathsettings", &OpticalPathDocument))
		return OPTICAL_PATH_MANAGER_ERROR;
		
	if (CVIXMLGetRootElement (OpticalPathDocument, &root))
		goto Error;	
	
	// optical_paths 
	if (CVIXMLNewElement (root, -1, "optical_paths", &optical_paths))
		goto Error;
	
	for(i=1; i <= list_size; i++) {
	
		ListGetItem (optical_path_manager->_list, &temp_optical_path, i);  
	
		// optical_path
		if (CVIXMLNewElement (optical_paths, -1, "OpticalPath", &optical_path))
			goto Error;
	
		// Name
		if (newXmlSettingStr (optical_path, "Name", temp_optical_path.name) < 0)
			goto Error;

		// Position
		if (newXmlSettingInt (optical_path, "Position", temp_optical_path.position) < 0)
			goto Error;

		CVIXMLDiscardElement(optical_path);
	}
	
	CVIXMLDiscardElement(optical_paths);
	
	if (FileExists(filepath, &fsize))
		SetFileAttrs (filepath, 0, -1, -1, -1);   //clear read only flag
	
	if (CVIXMLSaveDocument (OpticalPathDocument, 1, filepath))
		goto Error;
	
	CVIXMLDiscardElement(root);
	CVIXMLDiscardDocument (OpticalPathDocument);
	SetFileAttrs (filepath, 1, -1, -1, -1);   //set read only flag
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
	

	Error:
	
		if (el >= 0)
			CVIXMLDiscardElement(el);
		
		if (root >= 0)
			CVIXMLDiscardElement(root);
		
		if (OpticalPathDocument >= 0)
			CVIXMLDiscardDocument(OpticalPathDocument);

		return OPTICAL_PATH_MANAGER_ERROR;
}



int optical_path_manager_load_optical_path_file(OpticalPathManager* optical_path_manager, const char *filepath)
{
	int i, number_of_possible_optical_paths;
	CVIXMLElement root = -1, optical_paths_element=-1, element=-1, optical_path_element=-1;
	CVIXMLDocument OpticalPathDocument = -1;
	OpticalPath optical_path; 
	
	optical_path_manager->number_of_present_optical_paths = 0;
	
	if (openXmlRoot ((char *)filepath, "OpticalPathsettings", &OpticalPathDocument, &root) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
	
	ListClear(optical_path_manager->_list);  
	
	if(ClearListCtrl (optical_path_manager->_optical_path_table_panel, PATH_CONF_ALL_TREE) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;

	SetCtrlVal(optical_path_manager->_optical_path_table_panel, PATH_CONF_FNAME, filepath);


	// optical_paths
	if (CVIXMLGetChildElementByTag (root, "optical_paths", &optical_paths_element))
		goto Error;
	CVIXMLGetNumChildElements (optical_paths_element, &number_of_possible_optical_paths );

	
	for(i=0; i < number_of_possible_optical_paths; i++) {
	
		optical_path.id = i + 1;
		
		if (CVIXMLGetChildElementByIndex (optical_paths_element, i, &optical_path_element))
			goto Error;
	
	
		// Name
		if (getXmlSettingStr (optical_path_element, "Name", optical_path.name, 30) < 0)
			goto Error;

		// Position
		if (getXmlSettingInt (optical_path_element, "Position", &optical_path.position) < 0)
			goto Error;
		
		if(optical_path.position > 0)
			optical_path_manager->number_of_present_optical_paths++;
	
		ListInsertItem (optical_path_manager->_list, &optical_path, END_OF_LIST);
	
		CVIXMLDiscardElement(optical_path_element); 
	
	}
	
	CVIXMLDiscardElement(optical_paths_element);
	
	closeXmlRoot (OpticalPathDocument, root);

	optical_path_manager_load_all_possible_optical_paths_into_ui(optical_path_manager);

	return OPTICAL_PATH_MANAGER_SUCCESS;
	

	Error:
	
		if (element >= 0)
			CVIXMLDiscardElement(element);
			
		if (optical_paths_element >= 0)
			CVIXMLDiscardElement(optical_paths_element);  
			
		if (root >= 0)
			CVIXMLDiscardElement(root);
		
		if (OpticalPathDocument >= 0)
			CVIXMLDiscardDocument(OpticalPathDocument);

		return OPTICAL_PATH_MANAGER_ERROR;
}
