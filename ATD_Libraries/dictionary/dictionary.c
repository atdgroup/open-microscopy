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
   @file	dictionary.c
   @author	N. Devillard
   @date	Aug 2000
   @version	$Revision: 1.25 $
   @brief	Implements a dictionary for string variables.

   This module implements a simple dictionary object, i.e. a list
   of string/string associations. This object is useful to store e.g.
   informations retrieved from a configuration file (ini files).
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: dictionary.c,v 1.25 2007-05-27 13:03:43 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2007-05-27 13:03:43 $
	$Revision: 1.25 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "dictionary.h"


/** Maximum value size for integers and doubles. */
#define MAXVALSZ	1024

/** Minimal allocated number of entries in a dictionary */
#define DICTMINSZ	128

/** Invalid key token */
#define DICT_INVALID_KEY    ((char*)-1)


/*---------------------------------------------------------------------------
  							Private functions
 ---------------------------------------------------------------------------*/

/* Doubles the allocated size associated to a pointer */
/* 'size' is the current allocated size. */
static void * mem_double(void * ptr, int size)
{
    void    *   newptr ;
 
    newptr = calloc(2*size, 1);
    memcpy(newptr, ptr, size);
    free(ptr);
    return newptr ;
}


/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	Compute the hash key for a string.
  @param	key		Character string to use for key.
  @return	1 unsigned int on at least 32 bits.

  This hash function has been taken from an Article in Dr Dobbs Journal.
  This is normally a collision-free function, distributing keys evenly.
  The key is stored anyway in the struct so that collision can be avoided
  by comparing the key itself in last resort.
 */
/*--------------------------------------------------------------------------*/

unsigned dictionary_hash(const char * key)
{
	int			len ;
	unsigned	hash ;
	int			i ;

	len = strlen(key);
	for (hash=0, i=0 ; i<len ; i++) {
		hash += (unsigned)key[i] ;
		hash += (hash<<10);
		hash ^= (hash>>6) ;
	}
	hash += (hash <<3);
	hash ^= (hash >>11);
	hash += (hash <<15);
	return hash ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Create a new dictionary object.
  @param	size	Optional initial size of the dictionary.
  @return	1 newly allocated dictionary objet.

  This function allocates a new dictionary object of given size and returns
  it. If you do not know in advance (roughly) the number of entries in the
  dictionary, give size=0.
 */
/*--------------------------------------------------------------------------*/

dictionary * dictionary_new(int size)
{
	dictionary	*	d ;

	/* If no size was specified, allocate space for DICTMINSZ */
	if (size<DICTMINSZ) size=DICTMINSZ ;

	if (!(d = (dictionary *)calloc(1, sizeof(dictionary)))) {
		return NULL;
	}
	d->size = size ;
	d->val  = (char **)calloc(size, sizeof(char*));
	d->key  = (char **)calloc(size, sizeof(char*));
	d->hash = (unsigned int *)calloc(size, sizeof(unsigned));
	return d ;
}


char* dictionary_get_section_key(char *buffer, const char *section, const char* key)
{
	memset(buffer, 0, 1);
	
	sprintf(buffer, "%s:%s", section, key);
	
	return buffer;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Delete a dictionary object
  @param	d	dictionary object to deallocate.
  @return	void

  Deallocate a dictionary object and all memory associated to it.
 */
/*--------------------------------------------------------------------------*/

void dictionary_del(dictionary * d)
{
	int		i ;

	if (d==NULL) return ;
	for (i=0 ; i<d->size ; i++) {
		if (d->key[i]!=NULL)
			free(d->key[i]);
		if (d->val[i]!=NULL)
			free(d->val[i]);
	}
	free(d->val);
	free(d->key);
	free(d->hash);
	free(d);
	return ;
}


dictionary * dictionary_clone(dictionary *d)
{
	int i;
	dictionary *copy = NULL;

	if (d==NULL)
		return NULL;
	
	if (d->n<1)
		return NULL;

	copy = dictionary_new(d->size);

	for (i=0 ; i<d->size ; i++) {
        if (d->key[i]) {
			dictionary_set(copy, d->key[i],  d->val[i]);
        }
	}

	return copy;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Get a value from a dictionary.
  @param	d		dictionary object to search.
  @param	key		Key to look for in the dictionary.
  @param    def     Default value to return if key not found.
  @return	1 pointer to internally allocated character string.

  This function locates a key in a dictionary and returns a pointer to its
  value, or the passed 'def' pointer if no such key can be found in
  dictionary. The returned character pointer points to data internal to the
  dictionary object, you should not try to free it or modify it.
 */
/*--------------------------------------------------------------------------*/
char * dictionary_get(dictionary * d, const char * key, char * def)
{
	unsigned	hash ;
	int			i ;
	char lc_key[50] = "";
	
	strtolwr(key, lc_key); 
	
	hash = dictionary_hash(lc_key);
	for (i=0 ; i<d->size ; i++) {
        if (d->key==NULL)
            continue ;
        /* Compare hash */
		if (hash==d->hash[i]) {
            /* Compare string, to avoid hash collisions */
            if (!strcmp(lc_key, d->key[i])) {
				return d->val[i] ;
			}
		}
	}
	return def ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Get a value from a dictionary, as a char.
  @param	d		dictionary object to search.
  @param	key		Key to look for in the dictionary.
  @param	def		Default value for the key if not found.
  @return 	char	

  This function locates a key in a dictionary using dictionary_get,
  and returns the first char of the found string.
 */
/*--------------------------------------------------------------------------*/
char dictionary_getchar(dictionary * d, const char * key, char def)
{
	char * v ;

	if ((v=dictionary_get(d,key,DICT_INVALID_KEY))==DICT_INVALID_KEY) {
		return def ;
	} else {
		return v[0] ;
	}
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Get a value from a dictionary, as an int.
  @param	d		dictionary object to search.
  @param	key		Key to look for in the dictionary.
  @param	def		Default value for the key if not found.
  @return	int

  This function locates a key in a dictionary using dictionary_get,
  and applies atoi on it to return an int. If the value cannot be found
  in the dictionary, the default is returned.
 */
/*--------------------------------------------------------------------------*/
int dictionary_getint(dictionary * d, const char * key, int def)
{
	char * v ;

	if ((v=dictionary_get(d,key,DICT_INVALID_KEY))==DICT_INVALID_KEY) {
		return def ;
	} else {
		return atoi(v);
	}
}

unsigned long dictionary_getulong(dictionary * d, const char * key, unsigned long def)
{
	char * v ;

	if ((v=dictionary_get(d,key,DICT_INVALID_KEY))==DICT_INVALID_KEY) {
		return def ;
	} else {
		return strtoul (v, NULL, 0);
	}
}

/*-------------------------------------------------------------------------*/
/**
  @brief		Get a value from a dictionary, as a double.
  @param	d		dictionary object to search.
  @param	key		Key to look for in the dictionary.
  @param	def		Default value for the key if not found.
  @return	double

  This function locates a key in a dictionary using dictionary_get,
  and applies atof on it to return a double. If the value cannot be found
  in the dictionary, the default is returned.
 */
/*--------------------------------------------------------------------------*/
double dictionary_getdouble(dictionary * d, const char * key, double def)
{
	char * v ;

	if ((v=dictionary_get(d,key,DICT_INVALID_KEY))==DICT_INVALID_KEY) {
		return def ;
	} else {
		return atof(v);
	}
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Set a value in a dictionary.
  @param	d		dictionary object to modify.
  @param	key		Key to modify or add.
  @param	val 	Value to add.
  @return	void

  If the given key is found in the dictionary, the associated value is
  replaced by the provided one. If the key cannot be found in the
  dictionary, it is added to it.

  It is Ok to provide a NULL value for val, but NULL values for the dictionary
  or the key are considered as errors: the function will return immediately
  in such a case.

  Notice that if you dictionary_set a variable to NULL, a call to
  dictionary_get will return a NULL value: the variable will be found, and
  its value (NULL) is returned. In other words, setting the variable
  content to NULL is equivalent to deleting the variable from the
  dictionary. It is not possible (in this implementation) to have a key in
  the dictionary without value.
 */
/*--------------------------------------------------------------------------*/

void dictionary_set(dictionary * d, const char * key, char * val)
{
	int			i ;
	char		lc_key[100] = "";    
	unsigned	hash ;

	if (d==NULL || key==NULL) return ;
	
	strtolwr(key, lc_key); 
	
	/* Compute hash for this lc_key */
	hash = dictionary_hash(lc_key) ;
	/* Find if value is already in blackboard */
	if (d->n>0) {
		for (i=0 ; i<d->size ; i++) {
            if (d->key[i]==NULL)
                continue ;
			if (hash==d->hash[i]) { /* Same hash value */
				if (!strcmp(lc_key, d->key[i])) {	 /* Same lc_key */
					/* Found a value: modify and return */
					if (d->val[i]!=NULL)
						free(d->val[i]);
                    d->val[i] = val ? strduplicate(val) : NULL ;
                    /* Value has been modified: return */
					return ;
				}
			}
		}
	}
	/* Add a new value */
	/* See if dictionary needs to grow */
	if (d->n==d->size) {

		/* Reached maximum size: reallocate blackboard */
		d->val  = (char **)mem_double(d->val,  d->size * sizeof(char*)) ;
		d->key  = (char **)mem_double(d->key,  d->size * sizeof(char*)) ;
		d->hash = (unsigned int *)mem_double(d->hash, d->size * sizeof(unsigned)) ;

		/* Double size */
		d->size *= 2 ;
	}

    /* Insert lc_key in the first empty slot */
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL) {
            /* Add lc_key here */
            break ;
        }
    }
	/* Copy lc_key */
	d->key[i]  = strduplicate(lc_key);
    d->val[i]  = val ? strduplicate(val) : NULL ;
	d->hash[i] = hash;
	d->n ++ ;
	return ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Delete a key in a dictionary
  @param	d		dictionary object to modify.
  @param	key		Key to remove.
  @return   void

  This function deletes a key in a dictionary. Nothing is done if the
  key cannot be found.
 */
/*--------------------------------------------------------------------------*/
void dictionary_unset(dictionary * d, const char * key)
{
	unsigned	hash ;
	int			i ;

	if (key == NULL) {
		return;
	}

	hash = dictionary_hash(key);
	for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        /* Compare hash */
		if (hash==d->hash[i]) {
            /* Compare string, to avoid hash collisions */
            if (!strcmp(key, d->key[i])) {
                /* Found key */
                break ;
			}
		}
	}
    if (i>=d->size)
        /* Key not found */
        return ;

    free(d->key[i]);
    d->key[i] = NULL ;
    if (d->val[i]!=NULL) {
        free(d->val[i]);
        d->val[i] = NULL ;
    }
    d->hash[i] = 0 ;
    d->n -- ;
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Set a key in a dictionary, providing an int.
  @param	d		Dictionary to update.
  @param	key		Key to modify or add
  @param	val		Integer value to store (will be stored as a string).
  @return	void

  This helper function calls dictionary_set() with the provided integer
  converted to a string using %d.
 */
/*--------------------------------------------------------------------------*/


void dictionary_setint(dictionary * d, const char * key, int val)
{
	char	sval[MAXVALSZ];
	sprintf(sval, "%d", val);
	dictionary_set(d, key, sval);
}

void dictionary_setulong(dictionary * d, const char * key, unsigned long val)
{
	char	sval[MAXVALSZ];
	sprintf(sval, "%lu", val);
	dictionary_set(d, key, sval);
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Set a key in a dictionary, providing a double.
  @param	d		Dictionary to update.
  @param	key		Key to modify or add
  @param	val		Double value to store (will be stored as a string).
  @return	void

  This helper function calls dictionary_set() with the provided double
  converted to a string using %g.
 */
/*--------------------------------------------------------------------------*/


void dictionary_setdouble(dictionary * d, const char * key, double val)
{
	char	sval[MAXVALSZ];
	sprintf(sval, "%g", val);
	dictionary_set(d, key, sval);
}

void dictionary_setdouble_high_precision(dictionary * d, const char * key, double val)
{
	char	sval[MAXVALSZ];
	sprintf(sval, "%f", val);
	dictionary_set(d, key, sval);
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Dump a dictionary to an opened file pointer.
  @param	d	Dictionary to dump
  @param	f	Opened file pointer.
  @return	void

  Dumps a dictionary onto an opened file pointer. Key pairs are printed out
  as @c [Key]=[Value], one per line. It is Ok to provide stdout or stderr as
  output file pointers.
 */
/*--------------------------------------------------------------------------*/

void dictionary_dump(dictionary * d, FILE * out)
{
	int		i ;

	if (d==NULL || out==NULL) return ;
	if (d->n<1) {
		fprintf(out, "empty dictionary\n");
		return ;
	}
	for (i=0 ; i<d->size ; i++) {
        if (d->key[i]) {
            fprintf(out, "%20s\t[%s]\n",
                    d->key[i],
                    d->val[i] ? d->val[i] : "UNDEF");
        }
	}
	return ;
}


void dictionary_foreach(dictionary * d, DICTIONARY_KEYVAL_CALLBACK callback, void *callback_data)
{
	int		i ;

	if (d==NULL)
		return ;
	
	if (d->n<1)
		return;

	for (i=0 ; i<d->size ; i++) {
        if (d->key[i]) {
			callback(d, d->key[i], d->val[i], callback_data); 
        }
	}
	return ;
}


/* Example code */
#ifdef TESTDIC
#define NVALS 20000
int main(int argc, char *argv[])
{
	dictionary	*	d ;
	char	*	val ;
	int			i ;
	char		cval[90] ;

	/* allocate blackboard */
	printf("allocating...\n");
	d = dictionary_new(0);
	
	/* Set values in blackboard */
	printf("setting %d values...\n", NVALS);
	for (i=0 ; i<NVALS ; i++) {
		sprintf(cval, "%04d", i);
		dictionary_set(d, cval, "salut");
	}
	printf("getting %d values...\n", NVALS);
	for (i=0 ; i<NVALS ; i++) {
		sprintf(cval, "%04d", i);
		val = dictionary_get(d, cval, DICT_INVALID_KEY);
		if (val==DICT_INVALID_KEY) {
			printf("cannot get value for key [%s]\n", cval);
		}
	}
    printf("unsetting %d values...\n", NVALS);
	for (i=0 ; i<NVALS ; i++) {
		sprintf(cval, "%04d", i);
		dictionary_unset(d, cval);
	}
    if (d->n != 0) {
        printf("error deleting values\n");
    }

	printf("deallocating...\n");
	dictionary_del(d);
	return 0 ;
}
#endif
/* vim: set ts=4 et sw=4 tw=75 */
