#include "gci_ui_module.h" 
#include "file_prefix_dialog_ui.h"
#include "gci_utils.h"  
#include "windows.h"
#include "string_utils.h"

#include <utility.h>

#include <userint.h>
#include <ansi_c.h>

static char g_default_directory[GCI_MAX_PATHNAME_LEN] = ""; 

static int CVICALLBACK OnBrowseButtonClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
	{
		case EVENT_COMMIT:
        {
            char directory[500];
            int file_size = 0;
			
			if(!FileExists(g_default_directory, &file_size))
				MakeDir(g_default_directory);

            if(DirSelectPopup (g_default_directory, "File Save Directory", 1, 1, directory) == 0)
				return 0;
            
			if(!FileExists(directory, &file_size))
				MakeDir(directory);
			
			if(directory[strlen(directory)-1] != '\\')
				strcat(directory, "\\");

            SetCtrlVal(panel, PREFIX_PNL_SAVE_DIR, directory);
            
			strncpy(g_default_directory, directory, GCI_MAX_PATHNAME_LEN - 1);

            break;
        }
    }
    
    return 0;
    
}        
        
int FilePrefixSave_EraseLastUsedEntries(int parent_panel)
{
	char reg_key[500] = "", panel_define[100] = "";

	GetPanelAttribute(parent_panel, ATTR_CONSTANT_NAME, panel_define);

	sprintf(reg_key, "software\\GCI\\Microscope\\PanelsDetails\\FilePrefixDialog\\%s\\", panel_define);

	// Lets save the filename for next time
	RegWriteString (REGKEY_HKCU, reg_key, "FilePrefix", "");

	// Lets save the directory for next time
	RegWriteString (REGKEY_HKCU, reg_key, "Last Directory", "");

	// Erase the default entry as well
	sprintf(reg_key, "software\\GCI\\Microscope\\PanelsDetails\\FilePrefixDialog\\DEFAULT\\");

	// Lets save the filename for next time
	RegWriteString (REGKEY_HKCU, reg_key, "FilePrefix", "");

	// Lets save the directory for next time
	RegWriteString (REGKEY_HKCU, reg_key, "Last Directory", "");

	return 0;
}

// Returns 0 On success
// -1 on Cancel
// -2 on other errors
int FilePrefixSaveDialog(int parent_panel, char defaultDirectory[], char output_dir[], char output_filename[])
{
    int file_size, panel_id = 0, pnl = 0, ctrl = 0, left, top, real_string_size;
    char fullpath[GCI_MAX_PATHNAME_LEN] = "", reg_key[500] = "";
    char panel_define[100] = "";
	FILE *fp;

    strncpy(output_dir, defaultDirectory, GCI_MAX_PATHNAME_LEN - 1);
	strncpy(g_default_directory, defaultDirectory, GCI_MAX_PATHNAME_LEN - 1);
	strncpy(output_filename, "Sequence_Time%t_Point%n.ics", GCI_MAX_PATHNAME_LEN - 1);
    strncpy(panel_define, "DEFAULT", 100);

    if(find_resource("file_prefix_dialog_ui.uir", fullpath) < 0)
        return -2;
        				
	if(parent_panel > 0)
		GetPanelAttribute(parent_panel, ATTR_CONSTANT_NAME, panel_define);

	sprintf(reg_key, "software\\GCI\\Microscope\\PanelsDetails\\FilePrefixDialog\\%s\\", panel_define);

	// Lets save the filename for next time
	RegReadString (REGKEY_HKCU, reg_key, "FilePrefix", output_filename, 500, &real_string_size);
        
	// Lets save the directory for next time
	RegReadString (REGKEY_HKCU, reg_key, "Last Directory", output_dir, 500, &real_string_size);
    
	if(output_dir[strlen(output_dir)-1] != '\\')
		strcat(output_dir, "\\");

    panel_id = LoadPanel(0, fullpath, PREFIX_PNL);    

	if(!FileExists(output_dir, &file_size))
		MakeDir(output_dir);
	
    SetCtrlVal(panel_id, PREFIX_PNL_SAVE_DIR, output_dir);
    SetCtrlVal(panel_id, PREFIX_PNL_FILENAME, output_filename);

	if(parent_panel > 0) {
		GetPanelAttribute (parent_panel, ATTR_LEFT, &left);
		GetPanelAttribute (parent_panel, ATTR_TOP, &top);
		SetPanelPos (panel_id, top+10, left+10);
	}
	else {

		SetPanelAttribute(panel_id, ATTR_LEFT, VAL_AUTO_CENTER);           
		SetPanelAttribute(panel_id, ATTR_TOP, VAL_AUTO_CENTER);
	}

    if(InstallCtrlCallback (panel_id, PREFIX_PNL_BROWSE, OnBrowseButtonClicked, NULL) < 0)
        return -2;  

    SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
	DisplayPanel(panel_id);
	
	while (1) {
		
        ProcessSystemEvents();
			
		GetUserEvent (0, &pnl, &ctrl);
		
        if (pnl != panel_id)
            continue;
        
		if (ctrl == PREFIX_PNL_OK) {
         
			// Get the chosen Values
			GetCtrlVal(panel_id, PREFIX_PNL_SAVE_DIR, output_dir);
			GetCtrlVal(panel_id, PREFIX_PNL_FILENAME, output_filename);

			if(!FileExists(output_dir, &file_size))
				MakeDir(output_dir);

			// Try to save a test image to the requested directory
			sprintf(fullpath, "%sText.txt", output_dir);
			fp = fopen(fullpath, "w");

			if(fp == NULL) {
				GCI_MessagePopup("Error", "Unable to save to the requested directory");
				continue;
			}

			if(fp != NULL)
				fclose(fp);
			
			DeleteFile(fullpath);

			// Check filename has an extension
			if(strrchr(output_filename, '.') == NULL) {
				GCI_MessagePopup("Error", "Filename has no extension");
				continue;
			}

			break;
		}
                
		if (ctrl == PREFIX_PNL_CANCEL) {
            DiscardPanel(panel_id);
			return -1;
		}
	}

	// Lets save the filename for next time
	RegWriteString (REGKEY_HKCU, reg_key, "FilePrefix", output_filename);

	// Lets save the directory for next time
	RegWriteString (REGKEY_HKCU, reg_key, "Last Directory", output_dir);

    DiscardPanel(panel_id);  

    return 0;
}

// Returns 0 On success
// -1 on Cancel
// -2 on other errors
int SimpleFilePrefixSaveDialog(int parent_panel, char defaultDirectory[], char output_dir[], char prefix[], char ext[])
{
    int err, file_size = 0, panel_id = 0, pnl = 0, ctrl = 0, left, top, real_string_size;
    char fullpath[GCI_MAX_PATHNAME_LEN] = "", panel_define[100] = "";
    char last_char, reg_key[500] = "";

    strncpy(output_dir, defaultDirectory, GCI_MAX_PATHNAME_LEN - 1);
	strncpy(g_default_directory, defaultDirectory, GCI_MAX_PATHNAME_LEN - 1);
	strncpy(prefix, "Test", GCI_MAX_PATHNAME_LEN - 1);
	strncpy(panel_define, "DEFAULT", 100);
    
    if(find_resource("file_prefix_dialog_ui.uir", fullpath) < 0)
        goto PREFIX_DLG_ERROR;
        
	if(parent_panel > 0)
		GetPanelAttribute(parent_panel, ATTR_CONSTANT_NAME, panel_define);

	sprintf(reg_key, "software\\GCI\\Microscope\\PanelsDetails\\FilePrefixDialog\\%s\\", panel_define);

	// Lets save the filename for next time
	RegReadString (REGKEY_HKCU, reg_key, "FilePrefix", prefix, 500, &real_string_size);

	// Lets save the directory for next time
	RegReadString (REGKEY_HKCU, reg_key, "Last Directory", output_dir, 500, &real_string_size);
        
	if(output_dir[strlen(output_dir)-1] != '\\')
		strcat(output_dir, "\\");

    panel_id = LoadPanel(0, fullpath, SMP_PFP);    

    SetCtrlVal(panel_id, SMP_PFP_SAVE_DIR, output_dir);
    SetCtrlVal(panel_id, SMP_PFP_FILENAME, prefix);
	
	if(parent_panel > 0) {
		GetPanelAttribute (parent_panel, ATTR_LEFT, &left);
		GetPanelAttribute (parent_panel, ATTR_TOP, &top);
		SetPanelPos (panel_id, top+10, left+10);
	}
	else {

		SetPanelAttribute(panel_id, ATTR_LEFT, VAL_AUTO_CENTER);           
		SetPanelAttribute(panel_id, ATTR_TOP, VAL_AUTO_CENTER);
	}
	
    if(InstallCtrlCallback (panel_id, SMP_PFP_BROWSE, OnBrowseButtonClicked, defaultDirectory) < 0)
        goto PREFIX_DLG_ERROR;

    SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
	DisplayPanel(panel_id);
	
	while (1) {
		
        ProcessSystemEvents();
			
		GetUserEvent (0, &pnl, &ctrl);
		
        if (pnl != panel_id)
            continue;
        
		if (ctrl == SMP_PFP_OK)
            break;
                
        if (ctrl == SMP_PFP_CANCEL) {
            DiscardPanel(panel_id);
			return -1;
		}
	}

	GetCtrlVal(panel_id, PREFIX_PNL_SAVE_DIR, output_dir);

	if(!FileExists(output_dir, &file_size)) {

		err = MakeDir(output_dir);

		switch(err)
		{	
			case -1:
			{
				GCI_MessagePopup("Error", "Could create directory. That path does not exist");
				goto PREFIX_DLG_ERROR;
			}

			case -6:
			{
				GCI_MessagePopup("Error", "Could create directory. Do you have permissions?");
				goto PREFIX_DLG_ERROR;
			}

			case -9:
			{
				GCI_MessagePopup("Error", "Directory or file already exists with same pathname.");
				goto PREFIX_DLG_ERROR;
			}

			case 0:
			{
				break;
			}
		}
	}

    // Get the chosen Values
    GetCtrlVal(panel_id, SMP_PFP_SAVE_DIR, output_dir);
    GetCtrlVal(panel_id, SMP_PFP_FILENAME, prefix);
	GetCtrlVal(panel_id, SMP_PFP_EXT_RING, ext); 
	
	last_char = output_dir[strlen(output_dir) - 1];

	if(last_char != '\\')
		strcat(output_dir, "\\");
	
	// Lets save the filename for next time
	RegWriteString (REGKEY_HKCU, reg_key, "FilePrefix", prefix);

	// Lets save the directory for next time
	RegWriteString (REGKEY_HKCU, reg_key, "Last Directory", output_dir);

    DiscardPanel(panel_id);   
    return 0;

    PREFIX_DLG_ERROR:
    
		if(panel_id > 0)
			DiscardPanel(panel_id);

        return -2;
}

static void
time_str(char *time_str)
{
	time_t t;
	struct tm *tmp;

	t = time(NULL);

	tmp = localtime(&t);

	// NB windows does not allow ':' in an filename
	sprintf(time_str, "%02dh%02dm%02ds", tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
}

static void
datetime_str(char *datetime)
{
	char date[100] = "";
	char time_buf[30] = "";
	struct tm tim;
	time_t now;
	now = time(NULL);
	tim = *(localtime(&now));

	time_str(time_buf);

	strftime(date, 20, "%d_%m_%y", &tim);

	sprintf(datetime, "%s_%s", date, time_buf);
}

int FilePrefixParseString(char *file_string, int sequence_number, char *output)
{
	// Ok we search the string for % char's
	
   	char *tmp = file_string, ch, *tmp_out;

	output[0] = 0;
	tmp_out = output;
	
	for (; *tmp != '\0' && tmp; tmp++) {
		
		if ( *tmp == '%' ) {
			
			// Ok will now get the next char
			ch = *(tmp + 1);
			
			switch(ch)
			{
				case 't':
				{
					char time[10];
					
					// Get the current time
					time_str(time);
					sprintf(tmp_out, "%s", time);
					tmp_out+=strlen(tmp_out);
					tmp+=1;
					
					break;
				}
				
				case 'n':
				{
					// Get the current sequence_number
					sprintf(tmp_out, "%i", sequence_number);
					tmp_out+=strlen(tmp_out);
					tmp+=1;
					
					break;
				}
				case 'd':
				{
					// Get the current date
					char datetime[200];
					
					datetime_str(datetime);  

					sprintf(tmp_out, "%s", datetime);
					tmp_out+=strlen(tmp_out);
					tmp+=1;
					
					break;
				}
			}
		}
		else {
			// Copy the one character
			*tmp_out++ = *tmp;
		}
	}
		
	*tmp_out = '\0';
	
	return 0;
}
