#include "ExceptionHandler.h"
#include "time.h"
#include "Config.h"

#include <dbghelp.h>
#include <crtdbg.h>

#include "gci_utils.h"
#include "ThreadDebug.h"

#include <utility.h>
#include <cvirte.h>		
#include <userint.h>
#include <toolbox.h>

const int NumCodeBytes = 16;	// Number of code bytes to record.
const int MaxStackDump = 3072;	// Maximum number of DWORDS in stack dumps.
const int StackColumns = 4;		// Number of columns in stack dump.

static int lock = 0;

#define	ONEK			1024
#define	SIXTYFOURK		(64*ONEK)
#define	ONEM			(ONEK*ONEK)
#define	ONEG			(ONEK*ONEK*ONEK)

BOOL CALLBACK MyMiniDumpCallback(
	PVOID                            pParam, 
	const PMINIDUMP_CALLBACK_INPUT   pInput, 
	PMINIDUMP_CALLBACK_OUTPUT        pOutput 
); 

int IsDataSectionNeeded( const char* pModuleName ); 


///////////////////////////////////////////////////////////////////////////////
// Custom minidump callback 
//

BOOL CALLBACK MyMiniDumpCallback(
	PVOID                            pParam, 
	const PMINIDUMP_CALLBACK_INPUT   pInput, 
	PMINIDUMP_CALLBACK_OUTPUT        pOutput 
) 
{
	BOOL bRet = FALSE; 


	// Check parameters 

	if( pInput == 0 ) 
		return FALSE; 

	if( pOutput == 0 ) 
		return FALSE; 


	// Process the callbacks 

	switch( pInput->CallbackType ) 
	{
		case IncludeModuleCallback: 
		{
			// Include the module into the dump 
			bRet = TRUE; 
		}
		break; 

		case IncludeThreadCallback: 
		{
			// Include the thread into the dump 
			bRet = TRUE; 
		}
		break; 

		case ModuleCallback: 
		{
			// Are data sections available for this module ? 

			if( pOutput->ModuleWriteFlags & ModuleWriteDataSeg ) 
			{
				// Yes, they are, but do we need them? 

				if( !IsDataSectionNeeded( pInput->Module.FullPath ) ) 
				{
					wprintf( L"Excluding module data sections: %s \n", pInput->Module.FullPath ); 

					pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg); 
				}
			}

			bRet = TRUE; 
		}
		break; 

		case ThreadCallback: 
		{
			// Include all thread information into the minidump 
			bRet = TRUE;  
		}
		break; 

		case ThreadExCallback: 
		{
			// Include this information 
			bRet = TRUE;  
		}
		break; 

		case MemoryCallback: 
		{
			// We do not include any information here -> return FALSE 
			bRet = FALSE; 
		}
		break; 
	}

	return bRet; 

}


///////////////////////////////////////////////////////////////////////////////
// This function determines whether we need data sections of the given module 
//

int IsDataSectionNeeded( const char* pModuleName ) 
{
	// Check parameters 
	char szFileName[GCI_MAX_PATHNAME_LEN] = ""; 

	if( pModuleName == 0 ) 
	{
		return 0; 
	}

	// Extract the module name 

	_splitpath( pModuleName, NULL, NULL, szFileName, NULL ); 


	// Compare the name with the list of known names and decide 

	// Note: For this to work, the executable name must be "mididump.exe"
	if( wcsicmp( szFileName, L"mididump" ) == 0 ) 
	{
		return 1; 
	}
	else if( wcsicmp( szFileName, L"ntdll" ) == 0 ) 
	{
		return 1; 
	}


	// Complete 

	return 0; 
}

EXCEPTION_ACTION ShowExceptionDialog (int parent, char* message)
{
    int panel, pnl, ctrl, close_button_id, debug_button_id, label_title_id, textbox_id;

    // Create the panel
    panel = NewPanel (parent, "Unrecoverable microscope error", 253, 415, 193, 400);
    SetPanelAttribute (panel, ATTR_CONSTANT_NAME, "EXC_PANEL");
  
    SetPanelAttribute (panel, ATTR_SCROLL_BAR_COLOR, 0XD4D0C8);
    SetPanelAttribute (panel, ATTR_BACKCOLOR, VAL_LT_GRAY);
	SetPanelAttribute (panel, ATTR_TITLEBAR_STYLE, VAL_WINDOWS_STYLE);
    SetPanelAttribute (panel, ATTR_SIZABLE, 0);

    SetPanelAttribute (panel, ATTR_CAN_MINIMIZE, 0);
    SetPanelAttribute (panel, ATTR_CAN_MAXIMIZE, 0);
    SetPanelAttribute (panel, ATTR_MIN_HEIGHT_FOR_SCALING, 0);
    SetPanelAttribute (panel, ATTR_MIN_WIDTH_FOR_SCALING, 0);
    
	SetPanelAttribute (panel, ATTR_FLOATING, VAL_FLOAT_ALWAYS);

	debug_button_id = NewCtrl (panel, CTRL_SQUARE_COMMAND_BUTTON, "Debug", 166, 250);
    SetCtrlAttribute (panel, debug_button_id, ATTR_CONSTANT_NAME, "DEBUG");

    SetCtrlAttribute (panel, debug_button_id, ATTR_CTRL_MODE, VAL_HOT);
    SetCtrlAttribute (panel, debug_button_id, ATTR_LABEL_COLOR, VAL_WHITE);
    SetCtrlAttribute (panel, debug_button_id, ATTR_LABEL_FONT, "Arial");

    SetCtrlAttribute (panel, debug_button_id, ATTR_DFLT_VALUE, 0);
    SetCtrlAttribute (panel, debug_button_id, ATTR_CMD_BUTTON_COLOR, 0X1E57AE);
    SetCtrlAttribute (panel, debug_button_id, ATTR_AUTO_SIZING, VAL_NEVER_AUTO_SIZE);
    SetCtrlAttribute (panel, debug_button_id, ATTR_HEIGHT, 22);
    SetCtrlAttribute (panel, debug_button_id, ATTR_WIDTH, 64);

    close_button_id = NewCtrl (panel, CTRL_SQUARE_COMMAND_BUTTON, "Close", 166, 322);
    SetCtrlAttribute (panel, close_button_id, ATTR_CONSTANT_NAME, "CLOSE");

    SetCtrlAttribute (panel, close_button_id, ATTR_CTRL_MODE, VAL_HOT);
    SetCtrlAttribute (panel, close_button_id, ATTR_LABEL_COLOR, VAL_WHITE);
    SetCtrlAttribute (panel, close_button_id, ATTR_LABEL_FONT, "Arial");

    SetCtrlAttribute (panel, close_button_id, ATTR_DFLT_VALUE, 0);
    SetCtrlAttribute (panel, close_button_id, ATTR_CMD_BUTTON_COLOR, 0X1E57AE);
    SetCtrlAttribute (panel, close_button_id, ATTR_AUTO_SIZING, VAL_NEVER_AUTO_SIZE);
    SetCtrlAttribute (panel, close_button_id, ATTR_HEIGHT, 22);
    SetCtrlAttribute (panel, close_button_id, ATTR_WIDTH, 64);

    // Build control: label_title_id
    label_title_id = NewCtrl (panel, CTRL_TEXT_MSG, "A unrecoverable microscope error has occured.", 2, 14);
    SetCtrlAttribute (panel, label_title_id, ATTR_CONSTANT_NAME, "TEXTMSG");
    SetCtrlAttribute (panel, label_title_id, ATTR_SHORTCUT_KEY, 0);
    SetCtrlAttribute (panel, label_title_id, ATTR_CTRL_MODE, VAL_INDICATOR);
    SetCtrlAttribute (panel, label_title_id, ATTR_TEXT_FONT, "Arial");
    SetCtrlAttribute (panel, label_title_id, ATTR_TEXT_BOLD, 1);
    SetCtrlAttribute (panel, label_title_id, ATTR_TEXT_BGCOLOR, VAL_LT_GRAY);
    SetCtrlAttribute (panel, label_title_id, ATTR_DFLT_VALUE, "A unrecoverable microscope error has occured.");
    SetCtrlAttribute (panel, label_title_id, ATTR_HEIGHT, 16);
    SetCtrlAttribute (panel, label_title_id, ATTR_WIDTH, 269);
    SetCtrlAttribute (panel, label_title_id, ATTR_SIZE_TO_TEXT, 1);
    SetCtrlAttribute (panel, label_title_id, ATTR_TEXT_JUSTIFY, VAL_LEFT_JUSTIFIED);

    textbox_id = NewCtrl (panel, CTRL_TEXT_BOX_LS, "", 19, 12);
    SetCtrlAttribute (panel, textbox_id, ATTR_CONSTANT_NAME, "ERROR_MSG");
    SetCtrlAttribute (panel, textbox_id, ATTR_SHORTCUT_KEY, 0);
    SetCtrlAttribute (panel, textbox_id, ATTR_FRAME_COLOR, 0XD4D0C8);
    SetCtrlAttribute (panel, textbox_id, ATTR_SCROLL_BARS, VAL_NO_SCROLL_BARS);
    SetCtrlAttribute (panel, textbox_id, ATTR_SCROLL_BAR_COLOR, 0XD4D0C8);
    SetCtrlAttribute (panel, textbox_id, ATTR_CTRL_MODE, VAL_INDICATOR);
    SetCtrlAttribute (panel, textbox_id, ATTR_LABEL_WIDTH, 9);
    SetCtrlAttribute (panel, textbox_id, ATTR_LABEL_HEIGHT, 15);
    SetCtrlAttribute (panel, textbox_id, ATTR_LABEL_BGCOLOR, 0XD4D0C8);
    SetCtrlAttribute (panel, textbox_id, ATTR_TEXT_FONT, "Arial");
    SetCtrlAttribute (panel, textbox_id, ATTR_TEXT_POINT_SIZE, 12);
    SetCtrlAttribute (panel, textbox_id, ATTR_TEXT_BGCOLOR, VAL_OFFWHITE);
    SetCtrlAttribute (panel, textbox_id, ATTR_WRAP_MODE, VAL_WORD_WRAP);
    SetCtrlAttribute (panel, textbox_id, ATTR_HEIGHT, 143);
    SetCtrlAttribute (panel, textbox_id, ATTR_WIDTH, 375);
    SetCtrlAttribute (panel, textbox_id, ATTR_LABEL_TOP, -2);
    SetCtrlAttribute (panel, textbox_id, ATTR_LABEL_LEFT, 12);
    SetCtrlAttribute (panel, textbox_id, ATTR_LABEL_SIZE_TO_TEXT, 1);
    SetCtrlAttribute (panel, textbox_id, ATTR_TEXT_JUSTIFY, VAL_LEFT_JUSTIFIED);
	SetCtrlVal (panel, textbox_id, message);

	DisplayPanel(panel);

	while (1) {
			
		ProcessSystemEvents();
			
		GetUserEvent (0, &pnl, &ctrl);
		
		if (pnl == panel) {
			if (ctrl == close_button_id) {
				DiscardPanel(panel);	
				return EXCEPTION_QUIT;
			}

			if(ctrl == debug_button_id) {
				DiscardPanel(panel);			
				return EXCEPTION_DEBUG; 
			}
		}
	}

    return EXCEPTION_QUIT;
}

int CreateMiniDump(EXCEPTION_POINTERS* pep, DWORD dwExpCode)
{
	// Open the file 
	int validFile, success = 0;
	char buffer[1000] = "";
	char dump_filepath[GCI_MAX_PATHNAME_LEN] = "";
	char dt[50] = "";
	EXCEPTION_ACTION action;
	HANDLE hFile;

	if(lock == 0)
		GciCmtNewLock("CreateMiniDumpLock", 0, &lock);

	GciCmtGetLock(lock);

	get_date_as_valid_filepath_str(dt);

	sprintf(dump_filepath, "%s\\dump_%s.dmp", RUNTIME_OUTPUT_DIRECTORY, dt);
	
	hFile = CreateFile(dump_filepath, GENERIC_READ | GENERIC_WRITE, 
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

	validFile = ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE );

	if(validFile) 
	{
		// Create the minidump 

		MINIDUMP_EXCEPTION_INFORMATION mdei; 
		MINIDUMP_CALLBACK_INFORMATION mci; 
		MINIDUMP_TYPE mdt;
		BOOL rv;

		mdei.ThreadId           = GetCurrentThreadId(); 
		mdei.ExceptionPointers  = pep; 
		mdei.ClientPointers     = FALSE; 

		mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback; 
		mci.CallbackParam       = 0; 

		mdt       = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory | 
		                                          MiniDumpWithDataSegs | 
		                                          MiniDumpWithHandleData | 
		                                          MiniDumpWithUnloadedModules ); 

		rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), 
			hFile, mdt, (pep != 0) ? &mdei : 0, 0, &mci ); 

		if( !rv ) 
			GCI_MessagePopup("Error", "MiniDumpWriteDump failed"); 
	
		// Close the file 

		CloseHandle( hFile ); 

		sprintf(buffer, "A log of this error has been saved to\n%s."
						"\n\nPlease email this information to paul.barber@oncology.ox.ac.uk",
						dump_filepath);

		action = ShowExceptionDialog (0, buffer);

		success = 1;
	}

	if(success == 0) 
	{
		GCI_MessagePopup("Error", "MiniDumpWriteDump failed"); 
	}

	GciCmtReleaseLock(lock);

	// return the magic value which tells Win32 that this handler didn't
	// actually handle the exception - so that things will proceed as per
	// normal.
	if(action == EXCEPTION_DEBUG)
		return EXCEPTION_CONTINUE_SEARCH;
	
	return EXCEPTION_EXECUTE_HANDLER;
}
