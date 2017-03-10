#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <windows.h>
#include <winbase.h>

#include "Utils.h"

#include "string_utils.h"

char* STR_DLL_CALLCONV
str_itoa(int val, char *buffer)
{
	if(buffer != NULL)
    	sprintf(buffer, "%d", val);
    
    return buffer;
}


char* STR_DLL_CALLCONV
ftoa(double val, char *buffer)
{
    if(buffer != NULL)
    	sprintf(buffer, "%.2f", val);
    
    return buffer;
}


char* STR_DLL_CALLCONV
str_strdup(const char *s)
{
	char *p;
	
	p = (char *) malloc(strlen(s) + 1);
	if (p != NULL)
		strcpy(p, s);
	return p;
}


int STR_DLL_CALLCONV
str_contains_whitespace(char *s)
{
	char *tmp;
	
	for (tmp = s; *tmp && tmp; tmp++) {
		if ( isspace(*tmp) ) {
			return 1;
		}
	}
			
	return 0;
}


char* STR_DLL_CALLCONV
str_remove_whitespace(char *s)
{
	char *tmp, *buf;

    for (tmp = s, buf = s; *tmp && tmp; tmp++)
    {
		if (!isspace(*tmp))
			*buf++ = *tmp;
	}

	*buf = '\0';

	return s;
}


char* STR_DLL_CALLCONV
str_trim_whitespace(char *s)
{
	char *tmp, *buf, after_start = 0;

    for (tmp = s, buf = s; *tmp && tmp; tmp++)
    {
		if (!isspace(*tmp) || after_start > 0) {
			after_start = 1;
			*buf++ = *tmp;
		}
	}

	*buf = '\0';

	return s;
}


char* STR_DLL_CALLCONV
str_change_whitespace(char *s, char to)
{				   
	char *tmp, *buf;

    for (tmp = s, buf = s; *tmp && tmp; tmp++)
    {
		if (!isspace(*tmp))
			*buf++ = *tmp;
		else
			*buf++ = to;
	}

	*buf = '\0';

	return s;
}

int STR_DLL_CALLCONV
str_empty(char *s)
{
	char *tmp;
	
	for (tmp = s; *tmp && tmp; tmp++) {
		if ( *tmp != 0 ) {
			return 0;
		}
	}
			
	return 1;
}


char* STR_DLL_CALLCONV
str_strlwr(char *s)
{
   if (s != NULL)
   {
      char *p;

      for (p = s; *p; ++p)
         *p = tolower(*p);
   }
   
   return s;
}


int STR_DLL_CALLCONV
str_contains_ch(char *s, char ch)
{
	char *tmp;
	
	for (tmp = s; *tmp && tmp; tmp++) {
		if ( *tmp == ch ) {
			return 1;
		}
	}
			
	return 0;
}


char* STR_DLL_CALLCONV
str_change_char(char *s, char from, char to)
{
	char *tmp, *buf;

    for (tmp = s, buf = s; *tmp && tmp; tmp++)
    {
		if (*tmp != from)
			*buf++ = *tmp;
		else
			*buf++ = to;
	}

	return s;
}


char* STR_DLL_CALLCONV
str_remove_char(char *s, char ch)
{
	char *tmp, *buf;

    for (tmp = s, buf = s; *tmp && tmp; tmp++)
    {
		if (*tmp != ch)
			*buf++ = *tmp;
	}

	*buf = '\0';

	return s;
}


int STR_DLL_CALLCONV
str_get_path_for_file(const char *start_directory, const char *name, char *path)
{
	std::string filepath = find_filepath(start_directory, name);

	if (filepath == "")
		return 0;

	strcpy(path, filepath.c_str());

	return 1;
}


void STR_DLL_CALLCONV
english_date(char *date)
{
	struct tm tim;
	time_t now;
	now = time(NULL);
	tim = *(localtime(&now));

	strftime(date, 20, "%d\\%m\\%y", &tim);
}


void STR_DLL_CALLCONV
str_get_path_for_my_documents(char *path)
{
	char name_buffer[25];
	DWORD name_buffer_size = 25;

	// Retrieve the name of the user that it currently logged in	
    GetUserName(name_buffer, &name_buffer_size); 

	// Make sure the passed in char array in empty and concatenate to it.
	memset(path, 0, 1);

	strcat(path, "C:\\Documents and Settings\\");
	strcat(path, name_buffer);
	strcat(path, "\\My Documents");
}


int STR_DLL_CALLCONV
get_file_extension(const char* name, char *ext)
{
  	if (name == NULL || ext == NULL)
  		return 0;

    const char *tmp = strrchr(name, '.');

    if(tmp == NULL)
        return 0;

    strcpy(ext, tmp);

    return 1;
}


int STR_DLL_CALLCONV
get_file_without_extension(const char* fullname, char *name)
{
	if (fullname == NULL)
  		return 0;

  	int len_without_ext = strlen(fullname);
	memset(name, 0, len_without_ext); 
	const char *tmp = fullname;

	/* Point to last char of name */
	tmp += strlen(fullname) + 1;

	while(*(--tmp) != '.')
		--len_without_ext;

  	strncpy(name, fullname, len_without_ext);

	return 1;
}

int STR_DLL_CALLCONV
get_filename_from_fullpath(const char* path, char *name)
{
	if (path == NULL)
  		return 0;

	name[0] = '\0';

	const char *tmp = path;

	int path_len = strlen(path);
	int tmp_len = path_len;
	int count = 0;

	/* Point to last char of path */
	tmp += tmp_len - 1;

	while(*tmp != '/' && *tmp != '\\') {
		tmp--;
		tmp_len--;
		count++;

		if(tmp_len < 0) {
			// No path seperator just copy filename into name
			strncpy(name, path, path_len);
			name[path_len] = '\0';
			return 0;
		}
	}

  	strncpy(name, tmp + 1, count);
	name[count] = '\0';

	return 1;
}

int STR_DLL_CALLCONV
has_file_extension(const char* name, const char *ext)
{
	char extension[10];

  	get_file_extension(name, extension);
  		
	if(strcmp(extension, ext) == 0)
		return 1;

	return 0;
}


int STR_DLL_CALLCONV
IsHex(const char *str)
{
	const char *tmp = str;

	if(!(tmp[0] == '0' && (tmp[1] == 'x' || tmp[1] == 'X')))
		return 0;
	
	tmp += 2;

	for( ; *tmp != 0 ; tmp++)
	{
		if(!((*tmp >= '0' && *tmp <= '9') || 
			(*tmp >= 'a' && *tmp <= 'z') ||
			(*tmp >= 'A' && *tmp <= 'Z')))
			return 0;
	}

	return 1;
}


int STR_DLL_CALLCONV
IsDigital(const char *str)
{
	const char *tmp = str;


	for( ; *tmp != 0 ; tmp++)
	{
		if(*tmp < '0' || *tmp > '9')
			return 0;
	}

	return 1;
}