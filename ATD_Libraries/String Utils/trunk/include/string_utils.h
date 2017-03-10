#ifndef __STRING_UTILS__
#define __STRING_UTILS__

#ifdef __cplusplus
extern "C" {
#endif

#define STR_DLL_CALLCONV __stdcall
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FREEIMAGE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// STR_DLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef STRINGUTILS_EXPORTS
#define STR_DLL_API __declspec(dllexport)
#else
#define STR_DLL_API __declspec(dllimport)
#endif

STR_DLL_API char* STR_DLL_CALLCONV
str_itoa(int val, char *buffer);

STR_DLL_API char* STR_DLL_CALLCONV
ftoa(double val, char *buffer);

STR_DLL_API char* STR_DLL_CALLCONV
str_strdup(const char *s);

STR_DLL_API char* STR_DLL_CALLCONV
str_remove_whitespace(char *s);

STR_DLL_API int STR_DLL_CALLCONV
str_contains_whitespace(char *s);

STR_DLL_API int STR_DLL_CALLCONV
str_contains_ch(char *s, char ch);

STR_DLL_API char* STR_DLL_CALLCONV
str_trim_whitespace(char *s);

STR_DLL_API char* STR_DLL_CALLCONV
str_change_whitespace(char *s, char to);

STR_DLL_API int STR_DLL_CALLCONV
str_empty(char *s);

STR_DLL_API char* STR_DLL_CALLCONV
str_strlwr(char *s);

STR_DLL_API char* STR_DLL_CALLCONV
str_change_char(char *s, char from, char to);

STR_DLL_API char* STR_DLL_CALLCONV
str_remove_char(char *s, char ch);

STR_DLL_API int STR_DLL_CALLCONV
get_filename_from_fullpath(const char* path, char *name);

STR_DLL_API int STR_DLL_CALLCONV
str_get_path_for_file(const char *start_directory, const char *name, char *path);

STR_DLL_API void STR_DLL_CALLCONV
english_date(char* date);

STR_DLL_API void STR_DLL_CALLCONV
str_get_path_for_my_documents(char *path);

STR_DLL_API int STR_DLL_CALLCONV
get_file_extension(const char* name, char *ext);

STR_DLL_API int STR_DLL_CALLCONV
get_file_without_extension(const char* fullname, char *name);

STR_DLL_API int STR_DLL_CALLCONV
has_file_extension(const char* name, const char *ext);

STR_DLL_API int STR_DLL_CALLCONV
IsHex(const char *str);

STR_DLL_API int STR_DLL_CALLCONV
IsDigital(const char *str);


#ifdef __cplusplus
}
#endif

#endif


