#include "FilenameUtils.h"

#include <userint.h>
#include <formatio.h>
#include <ansi_c.h>
#include "toolbox.h"
#include <utility.h>

#define DEFAULT_IMAGE_PATH	   "c:\\images\\"

//////////////////////////////////////////////////////////////////////////
//Routines for dealing with the filenames of image sequences
//
//Rosalind Locke - February 2005
////////////////////////////////////////////////////////////////////////////
//RJL 29 June 2005
//If a filename with a number entered it was wrongly removed.
//(If the last 5 digits happen to be numeric it will still be removed.)
//////////////////////////////////////////////////////////////////////////

static void DeleteSequence (char* fname)
{
    int dot, n=1, size;
    char ext[5]="", fname2[MAX_PATHNAME_LEN], fname3[MAX_PATHNAME_LEN], temp[6];
    
	//Delete a sequence of images such as myImage_00001.ics to myImage_00123.ics
    SetWaitCursor(1);
    dot = FindPattern (fname, 0, -1, ".", 0, 1);
    if (dot != -1)
        CopyBytes (ext, 0, fname, dot, strlen(fname)-dot);
    if (dot == -1) dot = strlen(fname);
    FillBytes (fname2, 0, MAX_PATHNAME_LEN, 0);
    strncpy (fname2, fname, dot);
    while (1) {
        FillBytes (fname3, 0, MAX_PATHNAME_LEN, 0);
        strcpy (fname3, fname2);
        Fmt(temp, "%s<%i[w5p0]", n); //five chars with leading zeros
        strcat(fname3, temp);
        strcat(fname3, ext);
        if (!FileExists (fname3, &size)) break;
        if (size < 0) break;
        DeleteFile (fname3);
        n++;
    }
    SetWaitCursor(0);
}

void GCI_NextFname(char* fname, int i, char* sfname)
{
    char numstr[6],ext[6];
    int dot;

    dot = FindPattern (fname, 0, -1, ".", 0, 1);
    if (dot == -1) dot = strlen(fname);
    
    Fmt(numstr, "%s<%i[w5p0]", i); //five chars with leading zeros

    //Have to insert the number string before the dot
    CopyString (ext, 0, fname, dot, strlen(fname)-dot); //keep extension
    //Ensure it's a valid image extension
    if (strcmp(ext,".pic") && strcmp(ext,".ics") && strcmp(ext,".tif") && strcmp(ext,".bmp") && strcmp(ext,".ecw"))
        strcpy (ext, ".ics");
    CopyString (sfname, 0, fname, 0, dot);
    strcat(sfname,numstr);
    strcat(sfname,ext);
}

static int CheckIfFilesExist(char* fname)
{
    char fname2[MAX_PATHNAME_LEN], ext[5]="";
    int size, dot;

    dot = FindPattern (fname, 0, -1, ".", 0, 1);
    if (dot == -1) dot = strlen(fname);
    if (dot != -1)
        CopyBytes (ext, 0, fname, dot, strlen(fname)-dot);
    FillBytes (fname2, 0, MAX_PATHNAME_LEN, 0);
    strncpy (fname2, fname, dot);
    strcat(fname2, "00001");
    strcat(fname2, ext);

    if (FileExists (fname2, &size)) {
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
        if (!ConfirmPopup("", "Files already exist. Overwrite?"))
            return -1;
        DeleteSequence (fname);
    }
    return 0;
}

//RJL 290605 - function below added
static int seqNo(char *str)
{
	unsigned int i;
	
	//If string contains all numeric digits return 1
	for (i=0; i<strlen(str); i++) {
		if (isdigit(str[i]) == 0) return 0;  //not all digits
	}
	return 1;
}
int GCI_SetFilename(char *dir, char *filename)
{
    char tempstr[6]="", ext[5]="";
    char fname[MAX_PATHNAME_LEN], fname2[MAX_PATHNAME_LEN];
    int dot;

    if (FileSelectPopup (dir, "*.ics", "*.ics;*.tif;*.bmp;*.ecw;*.pic", "", VAL_OK_BUTTON,
                         0, 0, 1, 1, fname) < 1) return -1;
   
    //If the filename already contains a sequence number remove it
    dot = FindPattern (fname, 0, -1, ".", 0, 1);
    if (dot != -1)
        CopyBytes (ext, 0, fname, dot, strlen(fname)-dot);
    if (dot == -1) dot = strlen(fname);
    CopyBytes (tempstr, 0, fname, dot-5, 5);
	//RJL 290605 - line below changed
	//if (atoi (tempstr) > 0) {
	if (seqNo(tempstr)) {
        FillBytes (fname2, 0, MAX_PATHNAME_LEN, 0);
        strncpy (fname2, fname, dot-5);
        strcat(fname2, ext);
        strcpy (fname, fname2);
    }
    
    strcpy (filename, fname);

    //Does this sequence exist?
    if (CheckIfFilesExist(fname)) return -2;  //Files exist and user doesn't want to overwrite them
    
    return 0;
}

