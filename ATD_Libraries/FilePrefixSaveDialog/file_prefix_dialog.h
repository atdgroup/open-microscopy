#ifndef __FILE_PREFIX_SAVE_DIALOG__
#define __FILE_PREFIX_SAVE_DIALOG__

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

int FilePrefixSave_EraseLastUsedEntries(int parent_panel);
int SimpleFilePrefixSaveDialog(int parent_panel, char defaultDirectory[], char output_dir[], char prefix[], char ext[]);
int FilePrefixSaveDialog(int parent_panel, char defaultDirectory[], char output_dir[], char output_filename[]);
int FilePrefixParseString(char *file_string, int sequence_number, char *output);

#endif
