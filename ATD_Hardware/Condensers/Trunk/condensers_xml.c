#include "cvixml.h"

#include <utility.h>
#include "condensers.h"
#include "condensers_ui.h"

#include "string_utils.h" 
#include "xml_utils.h" 

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Condenser data control.
////////////////////////////////////////////////////////////////////////////

int condenser_manager_save_condenser_data(CondenserManager* condenser_manager, const char *filepath)
{
	int i, fsize, list_size;
	CVIXMLElement root = -1, condensers, condenser, el=-1;
	CVIXMLDocument CondensersDocument = -1;
	Condenser temp_condenser;
	
	list_size = ListNumItems (condenser_manager->_list); 
	
	//Save all condenser settings in xml format
	if (CVIXMLNewDocument ("Condensersettings", &CondensersDocument))
		return CONDENSER_MANAGER_ERROR;
		
	if (CVIXMLGetRootElement (CondensersDocument, &root))
		goto Error;	
	
	// condensers 
	if (CVIXMLNewElement (root, -1, "condensers", &condensers))
		goto Error;
	
	for(i=1; i <= list_size; i++) {
	
		ListGetItem (condenser_manager->_list, &temp_condenser, i);  
	
		// condenser
		if (CVIXMLNewElement (condensers, -1, "Condenser", &condenser))
			goto Error;
	
		// Name
		if (newXmlSettingStr (condenser, "Name", temp_condenser.name) < 0)
			goto Error;

		// Position
		if (newXmlSettingInt (condenser, "Position", temp_condenser.position) < 0)
			goto Error;

		CVIXMLDiscardElement(condenser);
	}
	
	CVIXMLDiscardElement(condensers);
	
	if (FileExists(filepath, &fsize))
		SetFileAttrs (filepath, 0, -1, -1, -1);   //clear read only flag
	
	if (CVIXMLSaveDocument (CondensersDocument, 1, filepath))
		goto Error;
	
	CVIXMLDiscardElement(root);
	CVIXMLDiscardDocument (CondensersDocument);
	SetFileAttrs (filepath, 1, -1, -1, -1);   //set read only flag
	
	return CONDENSER_MANAGER_SUCCESS;
	

	Error:
	
		if (el >= 0)
			CVIXMLDiscardElement(el);
		
		if (root >= 0)
			CVIXMLDiscardElement(root);
		
		if (CondensersDocument >= 0)
			CVIXMLDiscardDocument(CondensersDocument);

		return CONDENSER_MANAGER_ERROR;
}



int condenser_manager_load_condenser_file(CondenserManager* condenser_manager, const char *filepath)
{
	int i, number_of_possible_condensers;
	CVIXMLElement root = -1, condensers_element=-1, element=-1, condenser_element=-1;
	CVIXMLDocument CondensersDocument = -1;
	Condenser condenser; 
	
	condenser_manager->number_of_present_condensers = 0;
	
	if (openXmlRoot ((char *)filepath, "Condensersettings", &CondensersDocument, &root) < 0)
		return CONDENSER_MANAGER_ERROR;
	
	ListClear(condenser_manager->_list);  
	
	if(ClearListCtrl (condenser_manager->_condenser_table_panel, COND_CONF_ALL_TREE) < 0)
		return CONDENSER_MANAGER_ERROR;

	SetCtrlVal(condenser_manager->_condenser_table_panel, COND_CONF_FNAME, filepath);


	// condensers
	if (CVIXMLGetChildElementByTag (root, "condensers", &condensers_element))
		goto Error;
	CVIXMLGetNumChildElements (condensers_element, &number_of_possible_condensers );

	
	for(i=0; i < number_of_possible_condensers; i++) {
	
		condenser.id = i + 1;
		
		if (CVIXMLGetChildElementByIndex (condensers_element, i, &condenser_element))
			goto Error;
	
	
		// Name
		if (getXmlSettingStr (condenser_element, "Name", condenser.name, 10) < 0)
			goto Error;

		// Position
		if (getXmlSettingInt (condenser_element, "Position", &condenser.position) < 0)
			goto Error;
		
		if(condenser.position > 0)
			condenser_manager->number_of_present_condensers++;
	
		ListInsertItem (condenser_manager->_list, &condenser, END_OF_LIST);
	
		CVIXMLDiscardElement(condenser_element); 
	
	}
	
	CVIXMLDiscardElement(condensers_element);
	
	closeXmlRoot (CondensersDocument, root);

	condenser_manager_load_all_possible_condensers_into_ui(condenser_manager);

	return CONDENSER_MANAGER_SUCCESS;
	

	Error:
	
		if (element >= 0)
			CVIXMLDiscardElement(element);
			
		if (condensers_element >= 0)
			CVIXMLDiscardElement(condensers_element);  
			
		if (root >= 0)
			CVIXMLDiscardElement(root);
		
		if (CondensersDocument >= 0)
			CVIXMLDiscardDocument(CondensersDocument);

		return CONDENSER_MANAGER_ERROR;
}
