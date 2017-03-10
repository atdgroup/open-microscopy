#include <iostream>
#include "Utils.h"
#include "string_utils.h"


//typedef int (*FILE_FOUND_FUNCTION) (char *ics_filepath);


int main()
{
	char c;
	char name[500], date[30];
	char my_doc_path[300]; 
	
	get_filename_from_fullpath("buttons/new.ico", name);
	printf("%s\n", name);

	get_filename_from_fullpath("buttons//new.ico", name);
	printf("%s\n", name);

	get_filename_from_fullpath("buttons\\new.ico", name);
	printf("%s\n", name);

	get_filename_from_fullpath("new.ico", name);
	printf("%s\n", name);

	gets(&c);

	return 0;
}