/*
Copyright (c) 2000 by Nicolas Devillard.
MIT License

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
/*
/*-------------------------------------------------------------------------*/
/**
   @file    iniparser.c
   @author  N. Devillard
   @date    Mar 2000
   @version $Revision: 2.17 $
   @brief   Parser for ini files.
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: iniparser.c,v 2.17 2007-05-27 13:03:43 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2007-05-27 13:03:43 $
    $Revision: 2.17 $
*/

/*---------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/

#include "iniparser.h"
#include "strlib.h"

#define ASCIILINESZ         1024
#define INI_INVALID_KEY     ((char*)-1)

#ifndef VS_COMPILER

// Standard C NAN definition for very old compilers.
#define NAN (0./0.)
#define INF (1./0.)
#define NEG_INF (-1./0.)

#else

/* These are classes, what use are these?
#define _FPCLASS_SNAN   0x0001  //* signaling NaN 
#define _FPCLASS_PINF   0x0200  //* positive infinity 
#define _FPCLASS_NINF   0x0004  //* negative infinity 

#define NAN _FPCLASS_SNAN
#define INF _FPCLASS_PINF
#define NEG_INF _FPCLASS_NINF
*/

#define NAN sqrt(-1.0)
#define INF DBL_MAX+1.0E308
#define NEG_INF -DBL_MAX-1.0E308

#endif
/*---------------------------------------------------------------------------
                        Private to this module
 ---------------------------------------------------------------------------*/

/* Private: add an entry to the dictionary */
static void iniparser_add_entry(
    dictionary * d,
    char * sec,
    char * key,
    char * val)
{
    char longkey[2*ASCIILINESZ+1];

    /* Make a key as section:keyword */
    if (key!=NULL) {
        sprintf(longkey, "%s:%s", sec, key);
    } else {
        strcpy(longkey, sec);
    }

    /* Add (key,val) to dictionary */
    dictionary_set(d, longkey, val);
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Get number of sections in a dictionary
  @param    d   Dictionary to examine
  @return   int Number of sections found in dictionary

  This function returns the number of sections found in a dictionary.
  The test to recognize sections is done on the string stored in the
  dictionary: a section name is given as "section" whereas a key is
  stored as "section:key", thus the test looks for entries that do not
  contain a colon.

  This clearly fails in the case a section name contains a colon, but
  this should simply be avoided.

  This function returns -1 in case of error.
 */
/*--------------------------------------------------------------------------*/

int iniparser_getnsec(dictionary * d)
{
    int i ;
    int nsec ;

    if (d==NULL) return -1 ;
    nsec=0 ;
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        if (strchr(d->key[i], ':')==NULL) {
            nsec ++ ;
        }
    }
    return nsec ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Get name for section n in a dictionary.
  @param    d   Dictionary to examine
  @param    n   Section number (from 0 to nsec-1).
  @return   Pointer to char string

  This function locates the n-th section in a dictionary and returns
  its name as a pointer to a string statically allocated inside the
  dictionary. Do not free or modify the returned string!

  This function returns NULL in case of error.
 */
/*--------------------------------------------------------------------------*/

char * iniparser_getsecname(dictionary * d, int n)
{
    int i ;
    int foundsec ;

    if (d==NULL || n<0) return NULL ;
    foundsec=0 ;
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        if (strchr(d->key[i], ':')==NULL) {
            foundsec++ ;
            if (foundsec>n)
                break ;
        }
    }
    if (foundsec<=n) {
        return NULL ;
    }
    return d->key[i] ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a dictionary to an opened file pointer.
  @param    d   Dictionary to dump.
  @param    f   Opened file pointer to dump to.
  @return   void

  This function prints out the contents of a dictionary, one element by
  line, onto the provided file pointer. It is OK to specify @c stderr
  or @c stdout as output files. This function is meant for debugging
  purposes mostly.
 */
/*--------------------------------------------------------------------------*/
void iniparser_dump(dictionary * d, FILE * f)
{
    int     i ;

    if (d==NULL || f==NULL) return ;
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        if (d->val[i]!=NULL) {
            fprintf(f, "[%s]=[%s]\n", d->key[i], d->val[i]);
        } else {
            fprintf(f, "[%s]=UNDEF\n", d->key[i]);
        }
    }
    return ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Save a dictionary to a loadable ini file
  @param    d   Dictionary to dump
  @param    f   Opened file pointer to dump to
  @return   void

  This function dumps a given dictionary into a loadable ini file.
  It is Ok to specify @c stderr or @c stdout as output files.
 */
/*--------------------------------------------------------------------------*/

void iniparser_dump_ini(dictionary * d, FILE * f)
{
    int     i, j ;
    char    keym[ASCIILINESZ+1];
    int     nsec ;
    char *  secname ;
    int     seclen ;

    if (d==NULL || f==NULL) return ;

    nsec = iniparser_getnsec(d);
    if (nsec<1) {
        /* No section in file: dump all keys as they are */
        for (i=0 ; i<d->size ; i++) {
            if (d->key[i]==NULL)
                continue ;
            fprintf(f, "%s = %s\n", d->key[i], d->val[i]);
        }
        return ;
    }
    for (i=0 ; i<nsec ; i++) {
        secname = iniparser_getsecname(d, i) ;
        seclen  = (int)strlen(secname);
        fprintf(f, "\n[%s]\n", secname);
        sprintf(keym, "%s:", secname);
        for (j=0 ; j<d->size ; j++) {
            if (d->key[j]==NULL)
                continue ;
            if (!strncmp(d->key[j], keym, seclen+1)) {
                fprintf(f,
                        "%-30s = %s\n",
                        d->key[j]+seclen+1,
                        d->val[j] ? d->val[j] : "");
            }
        }
    }
    fprintf(f, "\n");
    return ;
}




/*-------------------------------------------------------------------------*/
/**
  @brief	Get the string associated to a key, return NULL if not found
  @param    d   Dictionary to search
  @param    key Key string to look for
  @return   pointer to statically allocated character string, or NULL.

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  NULL is returned.
  The returned char pointer is pointing to a string allocated in
  the dictionary, do not free or modify it.

  This function is only provided for backwards compatibility with 
  previous versions of iniparser. It is recommended to use
  iniparser_getstring() instead.
 */
/*--------------------------------------------------------------------------*/
char * iniparser_getstr(dictionary * d, const char * key)
{
    return iniparser_getstring(d, key, NULL);
}


char * iniparser_getstr_from_section(dictionary * d, const char *section, const char * key)
{
	char buffer[100] = "";

	sprintf(buffer, "%s:%s", section, key);

    return iniparser_getstring(d, buffer, NULL);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key
  @param    d       Dictionary to search
  @param    key     Key string to look for
  @param    def     Default value to return if key not found.
  @return   pointer to statically allocated character string

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the pointer passed as 'def' is returned.
  The returned char pointer is pointing to a string allocated in
  the dictionary, do not free or modify it.
 */
/*--------------------------------------------------------------------------*/
char * iniparser_getstring(dictionary * d, const char * key, char * def)
{
    char *lc_key = NULL, tmp[50] = "";
    char * sval ;

    if (d==NULL || key==NULL)
        return def ;

   	// if (!(lc_key = strduplicate(strlwc(key)))) {
	//	    return NULL;
   	// }
		
	if (!(lc_key = strtolwr(key, tmp))) {
	    return NULL;
   	}
		
    sval = dictionary_get(d, lc_key, def);
  
    return sval ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to an int
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   integer

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.

  Supported values for integers include the usual C notation
  so decimal, octal (starting with 0) and hexadecimal (starting with 0x)
  are supported. Examples:

  "42"      ->  42
  "042"     ->  34 (octal -> decimal)
  "0x42"    ->  66 (hexa  -> decimal)

  Warning: the conversion may overflow in various ways. Conversion is
  totally outsourced to strtol(), see the associated man page for overflow
  handling.

  Credits: Thanks to A. Becker for suggesting strtol()
 */
/*--------------------------------------------------------------------------*/
int iniparser_getint(dictionary * d, const char * key, int notfound)
{
    char    *   str ;

    str = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (str==INI_INVALID_KEY) return notfound ;
    return (int)strtol(str, NULL, 0);
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to a double
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   double

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.
 */
/*--------------------------------------------------------------------------*/
double iniparser_getdouble(dictionary * d, char * key, double notfound)
{
    char    *   str ;
	double      val ;

    str = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (str==INI_INVALID_KEY) return notfound ;

	if (strcmp(str, "+Inf")==0) val=INF;
	else if (strcmp(str, "1.#INF")==0) val=INF;
	else if (strcmp(str, "1.#INF00")==0) val=INF;
	else if (strcmp(str, "-Inf")==0) val=NEG_INF;
	else if (strcmp(str, "-1.#INF")==0) val=NEG_INF;
	else if (strcmp(str, "-1.#INF00")==0) val=NEG_INF;
	else if (strcmp(str, "NaN")==0) val=NAN;
	else if (strcmp(str, "1.#IND00")==0) val=NAN;
	else if (strcmp(str, "-1.#IND00")==0) val=NAN;
	else val = atof(str);

    return val;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to a boolean
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   integer

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.

  A true boolean is found if one of the following is matched:

  - A string starting with 'y'
  - A string starting with 'Y'
  - A string starting with 't'
  - A string starting with 'T'
  - A string starting with '1'

  A false boolean is found if one of the following is matched:

  - A string starting with 'n'
  - A string starting with 'N'
  - A string starting with 'f'
  - A string starting with 'F'
  - A string starting with '0'

  The notfound value returned if no boolean is identified, does not
  necessarily have to be 0 or 1.
 */
/*--------------------------------------------------------------------------*/
int iniparser_getboolean(dictionary * d, const char * key, int notfound)
{
    char    *   c ;
    int         ret ;

    c = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (c==INI_INVALID_KEY) return notfound ;
    if (c[0]=='y' || c[0]=='Y' || c[0]=='1' || c[0]=='t' || c[0]=='T') {
        ret = 1 ;
    } else if (c[0]=='n' || c[0]=='N' || c[0]=='0' || c[0]=='f' || c[0]=='F') {
        ret = 0 ;
    } else {
        ret = notfound ;
    }
    return ret;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Finds out if a given entry exists in a dictionary
  @param    ini     Dictionary to search
  @param    entry   Name of the entry to look for
  @return   integer 1 if entry exists, 0 otherwise

  Finds out if a given entry exists in the dictionary. Since sections
  are stored as keys with NULL associated values, this is the only way
  of querying for the presence of sections in a dictionary.
 */
/*--------------------------------------------------------------------------*/

int iniparser_find_entry(
    dictionary  *   ini,
    char        *   entry
)
{
    int found=0 ;
    if (iniparser_getstring(ini, entry, INI_INVALID_KEY)!=INI_INVALID_KEY) {
        found = 1 ;
    }
    return found ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Set an entry in a dictionary.
  @param    ini     Dictionary to modify.
  @param    entry   Entry to modify (entry name)
  @param    val     New value to associate to the entry.
  @return   int 0 if Ok, -1 otherwise.

  If the given entry can be found in the dictionary, it is modified to
  contain the provided value. If it cannot be found, -1 is returned.
  It is Ok to set val to NULL.
 */
/*--------------------------------------------------------------------------*/

int iniparser_setstr(dictionary * ini, char * entry, char * val)
{
	char tmp[ASCIILINESZ+1];
	
	tmp[0] = 0;
	
    dictionary_set(ini, strtolwr(entry, tmp), val);
    return 0 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Delete an entry in a dictionary
  @param    ini     Dictionary to modify
  @param    entry   Entry to delete (entry name)
  @return   void

  If the given entry can be found, it is deleted from the dictionary.
 */
/*--------------------------------------------------------------------------*/
void iniparser_unset(dictionary * ini, char * entry)
{
	char tmp[ASCIILINESZ+1];
	
	tmp[0] = 0;
	
    dictionary_unset(ini, strtolwr(entry, tmp));
}






/*-------------------------------------------------------------------------*/
/**
  @brief    Parse an ini file string and return an allocated dictionary object
  @param    ininame Name of the ini file to read.
  @return   Pointer to newly allocated dictionary

  This is the parser for ini file strings. The string
  is the format name=value1\0name2=value2\0\0
  Ie the same format that microsoft uses when writing ini sections. 

  The returned dictionary must be freed using iniparser_freedict().
 */
/*--------------------------------------------------------------------------*/
dictionary * iniparser_load_from_string(char* sec, char * ini_string)
{
	dictionary *d ;    
	char *tmp = ini_string, *val;
	char key[50], *tmp_key;
	
	/*
     * Initialize a new dictionary entry
     */
    if (!(d = dictionary_new(0)))
	    return NULL;
	
	tmp_key = key;
	
	// While we have not found \0\0
	for (; (*tmp != '\0') && (*(tmp + 1) != '\0'); tmp++) {
		
		*tmp_key = *tmp;
		
		if ( *tmp == '=' ) {
	
			// Remove '=' from key
			*tmp_key = '\0';
			
			// Ok will now get the next char
			val = tmp + 1;
			
			iniparser_add_entry(d, sec, key, val);
			memset(key, 0, 1);
			tmp_key = key;
			tmp += strlen(val) + 1;
			continue;
		}
	
		tmp_key++;
	}
	
	return d;
}
	


/*-------------------------------------------------------------------------*/
/**
  @brief    Parse an ini file and return an allocated dictionary object
  @param    ininame Name of the ini file to read.
  @return   Pointer to newly allocated dictionary

  This is the parser for ini files. This function is called, providing
  the name of the file to be read. It returns a dictionary object that
  should not be accessed directly, but through accessor functions
  instead.

  The returned dictionary must be freed using iniparser_freedict().
 */
/*--------------------------------------------------------------------------*/

dictionary * iniparser_load(const char * ininame)
{
    dictionary  *   d ;
	char		tmp[ASCIILINESZ+1];
	char		tmp2[ASCIILINESZ+1];  
    char        lin[ASCIILINESZ+1];
    char        sec[ASCIILINESZ+1];
    char        key[ASCIILINESZ+1];
    char        val[ASCIILINESZ+1];
    char    *   where ;
    FILE    *   ini ;
    int         lineno ;

    if ((ini=fopen(ininame, "r"))==NULL) {
        return NULL ;
    }

    sec[0]=0;
	tmp[0]=0;
	tmp2[0]=0;
	
    /*
     * Initialize a new dictionary entry
     */
    if (!(d = dictionary_new(0))) {
	    fclose(ini);
	    return NULL;
    }
    lineno = 0 ;
    while (fgets(lin, ASCIILINESZ, ini)!=NULL) {
        lineno++ ;
        where = strskp(lin); /* Skip leading spaces */
        if (*where==';' || *where=='#' || *where==0)
            continue ; /* Comment lines */
        else {
            if (sscanf(where, "[%[^]]", sec)==1) {
                /* Valid section name */
                strcpy(sec, strtolwr(sec, tmp));
                iniparser_add_entry(d, sec, NULL, NULL);
            } else if (sscanf (where, "%[^=] = \"%[^\"]\"", key, val) == 2
                   ||  sscanf (where, "%[^=] = '%[^\']'",   key, val) == 2
                   ||  sscanf (where, "%[^=] = %[^;#]",     key, val) == 2) {
               
				strcrop_r(key, tmp);
				strtolwr(tmp, tmp2);
				strcpy(key, tmp2); 
                /*
                 * sscanf cannot handle "" or '' as empty value,
                 * this is done here
                 */
                if (!strcmp(val, "\"\"") || !strcmp(val, "''")) {
                    val[0] = (char)0;
                } else {
                    strcpy(val, strcrop_r(val, tmp));
                }
                iniparser_add_entry(d, sec, key, val);
				
				if(val== 0 || strcmp(key, "node:") == 0)
					tmp[0]=0;
				
            }
        }
    }
    fclose(ini);
    return d ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Parse an ini file and return an allocated dictionary object
  @param    ininame Name of the ini file to read.
  @return   Pointer to newly allocated dictionary

  This is the parser for ini files. This function is called, providing
  the name of the file to be read. It returns a dictionary object that
  should not be accessed directly, but through accessor functions
  instead.

  The returned dictionary must be freed using iniparser_freedict().
 */
/*--------------------------------------------------------------------------*/

dictionary* iniparser_load_section_into_dictionary(dictionary* d, const char * ininame, const char *section)
{
    //dictionary  *   d ;
	char		tmp[ASCIILINESZ+1];
	char		tmp2[ASCIILINESZ+1];  
    char        lin[ASCIILINESZ+1];
    char        sec[ASCIILINESZ+1];
    char        key[ASCIILINESZ+1];
    char        val[ASCIILINESZ+1];
    char    *   where ;
    FILE    *   ini ;
    int         lineno ;
	int			section_found = 0;

    if ((ini=fopen(ininame, "r"))==NULL) {
        return NULL ;
    }

    sec[0]=0;
	tmp[0]=0;
	tmp2[0]=0;
	
    /*
     * Initialize a new dictionary entry
     */
 //   if (!(d = dictionary_new(0))) {
	//    fclose(ini);
	//    return NULL;
   // }

	if(d == NULL)
		return NULL;

    lineno = 0 ;
    while (fgets(lin, ASCIILINESZ, ini)!=NULL) {
        lineno++ ;
        where = strskp(lin); /* Skip leading spaces */
        if (*where==';' || *where=='#' || *where==0)
            continue ; /* Comment lines */
        else {
            if (sscanf(where, "[%[^]]", sec)==1) {
                /* Valid section name */
                strcpy(sec, strtolwr(sec, tmp));

				// We have found the required section
				if(strcmp(sec, strtolwr(section, tmp2)) == 0)
					section_found = 1;
				else
					section_found = 0;

            } else if (sscanf (where, "%[^=] = \"%[^\"]\"", key, val) == 2
                   ||  sscanf (where, "%[^=] = '%[^\']'",   key, val) == 2
                   ||  sscanf (where, "%[^=] = %[^;#]",     key, val) == 2) {
               
			    // We are not in the valid section move to next line.
			    if(section_found == 0)
					continue;

				strcrop_r(key, tmp);
				strtolwr(tmp, tmp2);
				strcpy(key, tmp2); 
                /*
                 * sscanf cannot handle "" or '' as empty value,
                 * this is done here
                 */
                if (!strcmp(val, "\"\"") || !strcmp(val, "''")) {
                    val[0] = (char)0;
                } else {
                    strcpy(val, strcrop_r(val, tmp));
                }

				/* Add (key,val) to dictionary */
				dictionary_set(d, key, val);
				
				if(val== 0 || strcmp(key, "node:") == 0)
					tmp[0]=0;
				
            }
        }
    }
    fclose(ini);
    return d ;
}

// Added by glenn pierce save dictionary to ini format
void iniparser_save(dictionary * d, FILE * out)
{
	int		i ;

	if (d==NULL || out==NULL) return ;
	if (d->n<1) {
		fprintf(out, "empty dictionary\n");
		return ;
	}
	for (i=0 ; i<d->size ; i++) {
        if (d->key[i]) {
			
			if(d->val[i] == NULL) // print section
				fprintf(out, "[%s]\n", d->key[i]);     	
			else
            	fprintf(out, "%s = %s\n", d->key[i], d->val[i]);
        }
	}
	return ;
}

// Added by Paul Barber to save dictionary to ini format with quotes around values to cope with Inf etc. June 2013
int iniparser_save_file(dictionary * d, const char *ininame)
{
	int		i ;
	FILE  * out = NULL;

	if (d==NULL) return -1;

	out = fopen(ininame, "w");

	if (out==NULL) return -2;

	if (d->n<1) {
		fprintf(out, "empty dictionary\n");
		return 1; // Warning
	}
	for (i=0 ; i<d->size ; i++) {
        if (d->key[i]) {
			
			if(d->val[i] == NULL) // print section
				fprintf(out, "[%s]\n", d->key[i]);     	
			else
            	fprintf(out, "%s = \"%s\"\n", d->key[i], d->val[i]);
        }
	}
	
	fclose(out);

	return 0;
}
/*-------------------------------------------------------------------------*/
/**
  @brief    Free all memory associated to an ini dictionary
  @param    d Dictionary to free
  @return   void

  Free all memory associated to an ini dictionary.
  It is mandatory to call this function before the dictionary object
  gets out of the current context.
 */
/*--------------------------------------------------------------------------*/

void iniparser_freedict(dictionary * d)
{
    dictionary_del(d);
}

/* vim: set ts=4 et sw=4 tw=75 */
