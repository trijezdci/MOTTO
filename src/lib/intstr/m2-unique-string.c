/* M2C Modula-2 Compiler & Translator
 * Copyright (c) 2015 Benjamin Kowarsch
 *
 * @synopsis
 *
 * M2C is a compiler and translator for the classic Modula-2 programming
 * language as described in the 3rd and 4th editions of Niklaus Wirth's
 * book "Programming in Modula-2" (PIM) published by Springer Verlag.
 *
 * In compiler mode, M2C compiles Modula-2 source via C to object files or
 * executables using the host system's resident C compiler and linker.
 * In translator mode, it translates Modula-2 source to C source.
 *
 * Further information at http://savannah.nongnu.org/projects/m2c/
 *
 * @file
 *
 * m2-unique-string.h
 *
 * Implementation of M2C unique string module.
 *
 * The module provides a dynamic string ADT managed in an internal
 * global string repository to avoid duplicate string allocation.
 *
 * @license
 *
 * M2C is free software: you can redistribute and/or modify it under the
 * terms of the GNU Lesser General Public License (LGPL) either version 2.1
 * or at your choice version 3 as published by the Free Software Foundation.
 *
 * M2C is distributed in the hope that it will be useful,  but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  Read the license for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with m2c.  If not, see <https://www.gnu.org/copyleft/lesser.html>.
 */

#include "m2-unique-string.h"

#include "hash.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>


/* --------------------------------------------------------------------------
 * Defaults
 * ----------------------------------------------------------------------- */

#define M2C_STRING_REPO_DEFAULT_BUCKET_COUNT 2011


/* --------------------------------------------------------------------------
 * private type m2c_hash_t
 * --------------------------------------------------------------------------
 * unsigned integer type representing a 32-bit hash value.
 * ----------------------------------------------------------------------- */

typedef uint32_t m2c_hash_t;


/* --------------------------------------------------------------------------
 * hidden type m2c_string_struct_t
 * --------------------------------------------------------------------------
 * record type representing a dynamic string object.
 * ----------------------------------------------------------------------- */

struct m2c_string_struct_t {
  /* ref_count */ uint_t ref_count;
  /* length */ uint_t length;
  /* char_array */ char char_array[];
};

typedef struct m2c_string_struct_t m2c_string_struct_t;


/* --------------------------------------------------------------------------
 * private types m2c_string_repo_entry_t and m2c_string_repo_entry_s
 * --------------------------------------------------------------------------
 * pointer and record type representing a string repository entry.
 * ----------------------------------------------------------------------- */

typedef struct m2c_string_repo_entry_s *m2c_string_repo_entry_t;

struct m2c_string_repo_entry_s {
  /* key */ m2c_hash_t key;
  /* str */ m2c_string_t str;
  /* next */ m2c_string_repo_entry_t next;
};

typedef struct m2c_string_repo_entry_s m2c_string_repo_entry_s;


/* --------------------------------------------------------------------------
 * private types m2c_string_repo_t and m2c_string_repo_s
 * --------------------------------------------------------------------------
 * pointer and record type representing the string repository.
 * ----------------------------------------------------------------------- */

typedef struct m2c_string_repo_s *m2c_string_repo_t;

struct m2c_string_repo_s {
  /* entry_count */ uint_t entry_count;
  /* bucket_count */ uint_t bucket_count;
  /* bucket */ m2c_string_repo_entry_t bucket[];
};

typedef struct m2c_string_repo_s m2c_string_repo_s;


/* --------------------------------------------------------------------------
 * private variable repository
 * --------------------------------------------------------------------------
 * pointer to global string repository.
 * ----------------------------------------------------------------------- */

static m2c_string_repo_t repository = NULL;


/* --------------------------------------------------------------------------
 * Forward declarations
 * ----------------------------------------------------------------------- */

static m2c_string_t new_string_from_string (char *str, uint_t length);

static m2c_string_t new_string_from_slice
  (char *str, uint_t offset, uint_t length);

static m2c_string_t new_string_by_appending
  (char *str, char *append_str, uint_t str_len, uint_t append_str_len);

static bool matches_str_and_len
  (m2c_string_t str, const char *cmpstr, uint_t length);

static m2c_string_repo_entry_t new_repo_entry
  (m2c_string_t str, m2c_hash_t key);

static m2c_string_t remove_repo_entry (m2c_string_t str, m2c_hash_t key);

static inline m2c_hash_t key_for_string (m2c_string_t str);


/* --------------------------------------------------------------------------
 * procedure m2c_init_string_repository(size, status)
 * --------------------------------------------------------------------------
 * Allocates and initialises global string repository.  Parameter size
 * determines the number of buckets of the repository's internal hash table.
 * If size is zero, value M2C_STRING_REPO_DEFAULT_BUCKET_COUNT is used.
 *
 * pre-conditions:
 * o  global repository must be uninitialised upon entry
 * o  parameter size may be zero upon entry
 * o  parameter status may be NULL upon entry
 *
 * post-conditions:
 * o  global string repository is allocated and intialised.
 * o  M2C_STRING_STATUS_SUCCESS is passed back in status, unless NULL
 *
 * error-conditions:
 * o  if repository has already been initialised upon entry,
 *    no operation is carried out and M2C_STRING_STATUS_ALREADY_INITIALIZED
 *    is passed back in status unless status is NULL
 * o  if repository allocation failed, M2C_STRING_STATUS_ALLOCATION_FAILED
 *    is passed back in status unless status is NULL
 * ----------------------------------------------------------------------- */

void m2c_init_string_repository
  (uint_t size, m2c_string_status_t *status) {
  
  uint_t index, bucket_count, allocation_size;
  
  /* check pre-conditions */
  if (repository != NULL) {
    SET_STATUS(status, M2C_STRING_STATUS_ALREADY_INITIALIZED);
    return;
  } /* end if */
  
  /* determine bucket count */
  if (size == 0) {
    bucket_count = M2C_STRING_REPO_DEFAULT_BUCKET_COUNT;
  }
  else /* size != 0 */ {
    bucket_count = size;
  } /* end if */
  
  /* allocate repository */
  allocation_size = sizeof(m2c_string_repo_s) +
    bucket_count * sizeof(m2c_string_repo_entry_t);
  repository = malloc(allocation_size);
  
  /* bail out if allocation failed */
  if (repository == NULL) {
    SET_STATUS(status, M2C_STRING_STATUS_ALLOCATION_FAILED);
    return;
  } /* end if */
  
  /* set entry and bucket count */
  repository->entry_count = 0;
  repository->bucket_count = bucket_count;
  
  /* initialise buckets */
  index = 0;
  while (index < bucket_count) {
    repository->bucket[index] = NULL;
    index++;
  } /* end while */
  
  SET_STATUS(status, M2C_STRING_STATUS_SUCCESS);
  return;
} /* end m2c_init_string_repository */


/* --------------------------------------------------------------------------
 * function m2c_get_string(str, status)
 * --------------------------------------------------------------------------
 * Returns a unique string object for str which must be a pointer to a NUL
 * terminated character string.  
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry
 *
 * post-conditions:
 * o  if a string object for str is present in the internal repository,
 *    that string object is retrieved, retained and returned.
 * o  if no string object for str is present in the repository,
 *    a new string object with a copy of str is created, stored and returned.
 * o  M2C_STRING_STATUS_SUCCESS is passed back in status, unless NULL
 *
 * error-conditions:
 * o  if str is NULL upon entry, no operation is carried out,
 *    NULL is returned and M2C_STRING_STATUS_INVALID_REFERENCE is
 *    passed back in status unless status is NULL
 * o  if string object allocation failed, no operation is carried out,
 *    NULL is returned and M2C_STRING_STATUS_ALLOCATION_FAILED is
 *    passed back in status unless status is NULL
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_get_string (char *str, m2c_string_status_t *status) {
  m2c_string_repo_entry_t new_entry, this_entry;
  m2c_string_t new_string, this_string;
  uint_t index, length;
  m2c_hash_t key;
  char ch;
  
  /* check repository */
  if (repository == NULL) {
    SET_STATUS(status, M2C_STRING_STATUS_NOT_INITIALIZED);
    return NULL;
  } /* end if */
  
  /* check str */
  if (str == NULL) {
    SET_STATUS(status, M2C_STRING_STATUS_INVALID_REFERENCE);
    return NULL;
  } /* end if */
    
  /* determine length and key */
  index = 0;
  ch = str[index];
  key = HASH_INITIAL;
  while (ch != ASCII_NUL) {
    key = HASH_NEXT_CHAR(key, ch);
    index++;
    ch = str[index];
  } /* end while */
  
  length = index + 1;
  key = HASH_FINAL(key);
  
  /* determine bucket index */
  index = key % repository->bucket_count;
  
  /* check if str is already in repository */
  if /* bucket empty */ (repository->bucket[index] == NULL) {
    
    /* create a new string object */
    new_string = new_string_from_string(str, length);
    
    /* create a new repository entry */
    new_entry = new_repo_entry(new_string, key);
    
    /* link the bucket to the new entry */
    repository->bucket[index] = new_entry;
    
    /* update the entry counter */
    repository->entry_count++;
    
    /* set status and return string object */
    SET_STATUS(status, M2C_STRING_STATUS_SUCCESS);
    return new_string;
  }
  else /* bucket not empty */ {
    
    /* first entry in bucket is starting point */
    this_entry = repository->bucket[index];
    
    /* search bucket for matching string */
    while (true) {
      if /* match found in current entry */
        ((this_entry->key == key) &&
          matches_str_and_len(this_entry->str, str, length)) {
        
        /* get string object of matching entry and retain it */
        this_string = this_entry->str;
        m2c_string_retain(this_string);
        
        /* set status and return string object */
        SET_STATUS(status, M2C_STRING_STATUS_SUCCESS);
        return this_string;
      }
      else if /* last entry reached without match */
        (this_entry->next == NULL) {
        
        /* create a new string object */
        new_string = new_string_from_string(str, length);
        
        /* create a new repository entry */
        new_entry = new_repo_entry(new_string, key);
        
        /* link last entry to the new entry */
        this_entry->next = new_entry;
        
        /* update the entry counter */
        repository->entry_count++;
        
        /* set status and return string object */
        SET_STATUS(status, M2C_STRING_STATUS_SUCCESS);
        return new_string;
      }
      else /* not last entry yet, move to next entry */ {
        this_entry = this_entry->next;
      } /* end if */
    } /* end while */      
  } /* end if */  
} /* end m2c_get_string */


/* --------------------------------------------------------------------------
 * function m2c_get_string_for_slice(str, offset, length, status)
 * --------------------------------------------------------------------------
 * Returns a unique string object for a given slice of str.  Parameter str
 * must be a pointer to a NUL terminated character string.  The position of
 * the first character of the slice is given by parameter offset and the
 * length of the slice is given by parameter length.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry
 * o  slice must be within range of str (NOT GUARDED),
 * o  slice must not contain any characters with code points < 32 or > 126 
 * 
 *
 * post-conditions:
 * o  if a string object for the slice is present in the internal repository,
 *    that string object is retrieved, retained and returned.
 * o  if no string object for the slice is present in the repository, a new
 *    string object with a copy of the slice is created, stored and returned.
 * o  M2C_STRING_STATUS_SUCCESS is passed back in status, unless NULL
 *
 * error-conditions:
 * o  if precondition #1 is not met, no operation is carried out,
 *    NULL is returned and M2C_STRING_STATUS_INVALID_REFERENCE is
 *    passed back in status, unless status is NULL
 * o  if precondition #3 is not met, no operation is carried out,
 *    NULL is returned and M2C_STRING_STATUS_INVALID_INDICES is
 *    passed back in status, unless status is NULL
 * o  if string object allocation failed, no operation is carried out,
 *    NULL is returned and M2C_STRING_STATUS_ALLOCATION_FAILED is
 *    passed back in status unless status is NULL
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_get_string_for_slice
  (char *str, uint_t offset, uint_t length, m2c_string_status_t *status) {
  
  m2c_string_repo_entry_t new_entry, this_entry;
  m2c_string_t new_string, this_string;
  uint_t index;
  m2c_hash_t key;
  char ch;
  
  /* check repository */
  if (repository == NULL) {
    SET_STATUS(status, M2C_STRING_STATUS_NOT_INITIALIZED);
    return NULL;
  } /* end if */
  
  /* check str */
  if (str == NULL) {
    SET_STATUS(status, M2C_STRING_STATUS_INVALID_REFERENCE);
    return NULL;
  } /* end if */
  
  /* determine key for slice */
  index = offset;
  key = HASH_INITIAL;
  while (index < length) {
    ch = str[index];
    /* bail out if any control codes are found */
    if (IS_CONTROL_CHAR(ch)) {
      SET_STATUS(status, M2C_STRING_STATUS_INVALID_INDICES);
      return NULL;
    } /* end if */
    key = HASH_NEXT_CHAR(key, ch);
    index++;
  } /* end while */
  
  key = HASH_FINAL(key);
  
  /* determine bucket index */
  index = key % repository->bucket_count;
  
  /* check if str is already in repository */
  if /* bucket empty */ (repository->bucket[index] == NULL) {
    
    /* create a new string object */
    new_string = new_string_from_slice(str, offset, length);
    
    /* create a new repository entry */
    new_entry = new_repo_entry(new_string, key);
    
    /* link the bucket to the new entry */
    repository->bucket[index] = new_entry;
    
    /* update the entry counter */
    repository->entry_count++;
    
    /* set status and return string object */
    SET_STATUS(status, M2C_STRING_STATUS_SUCCESS);
    return new_string;
  }
  else /* bucket not empty */ {
    
    /* first entry in bucket is starting point */
    this_entry = repository->bucket[index];
    
    /* search bucket for matching string */
    while (true) {
      if /* match found in current entry */
        ((this_entry->key == key) &&
          matches_str_and_len(this_entry->str, str, length)) {
        
        /* get string object of matching entry and retain it */
        this_string = this_entry->str;
        m2c_string_retain(this_string);
        
        /* set status and return string object */
        SET_STATUS(status, M2C_STRING_STATUS_SUCCESS);
        return this_string;
      }
      else if /* last entry reached without match */
        (this_entry->next == NULL) {
        
        /* create a new string object */
        new_string = new_string_from_slice(str, offset, length);
        
        /* create a new repository entry */
        new_entry = new_repo_entry(new_string, key);
        
        /* link last entry to the new entry */
        this_entry->next = new_entry;
        
        /* update the entry counter */
        repository->entry_count++;
        
        /* set status and return string object */
        SET_STATUS(status, M2C_STRING_STATUS_SUCCESS);
        return new_string;
      }
      else /* not last entry yet, move to next entry */ {
        this_entry = this_entry->next;
      } /* end if */
    } /* end while */      
  } /* end if */  
} /* end m2c_get_string_for_slice */


/* --------------------------------------------------------------------------
 * function m2c_get_string_for_concatenation(str, append_str, status)
 * --------------------------------------------------------------------------
 * Allocates a new object of type m2c_string_t and initialises it with a
 * copy of str appended by a copy of append_str each of which must be a
 * pointer to a NUL terminated character string.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry
 * o  parameter append_str must not be NULL upon entry
 *
 * post-conditions:
 * o  pointer to newly allocated dynamic string is returned
 * o  copy of character string and its length is stored internally
 * o  M2C_STRING_STATUS_SUCCESS is passed back in status, unless NULL
 *
 * error-conditions:
 * o  if any of str or append_str is NULL upon entry, no operation is
 *    carried out, NULL is returned and M2C_STRING_STATUS_INVALID_REFERENCE
 *    is passed back in status, unless NULL
 * o  if no dynamic string object could be allocated, NULL is returned,
 *    and M2C_STRING_STATUS_ALLOCATION_FAILED is passed back in status,
 *    unless NULL
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_get_string_for_concatenation
  (char *str, char *append_str, m2c_string_status_t *status) {
  
  m2c_string_repo_entry_t new_entry, this_entry;
  m2c_string_t new_string, this_string;
  uint_t index, length, str_len, append_str_len;
  m2c_hash_t key;
  char ch;
  
  /* check repository */
  if (repository == NULL) {
    SET_STATUS(status, M2C_STRING_STATUS_NOT_INITIALIZED);
    return NULL;
  } /* end if */
  
  /* check str and append_str */
  if ((str == NULL) || (append_str == NULL)) {
    SET_STATUS(status, M2C_STRING_STATUS_INVALID_REFERENCE);
    return NULL;
  } /* end if */
  
  /* determine key and length of str */
  index = 0;
  ch = str[index];
  key = HASH_INITIAL;
  while (ch != ASCII_NUL) {
    key = HASH_NEXT_CHAR(key, ch);
    index++;
    ch = str[index];
  } /* end while */
  
  str_len = index + 1;
  
  /* determine key and length of append_str */
  index = 0;
  ch = append_str[index];
  key = HASH_INITIAL;
  while (ch != ASCII_NUL) {
    key = HASH_NEXT_CHAR(key, ch);
    index++;
    ch = append_str[index];
  } /* end while */
  
  append_str_len = index + 1;
  
  /* finalise length and key of concatenation */
  length = str_len + append_str_len;
  key = HASH_FINAL(key);
  
  /* determine bucket index */
  index = key % repository->bucket_count;
  
  /* check if str is already in repository */
  if /* bucket empty */ (repository->bucket[index] == NULL) {
    
    /* create a new string object */
    new_string =
      new_string_by_appending(str, append_str, str_len, append_str_len);
    
    /* create a new repository entry */
    new_entry = new_repo_entry(new_string, key);
    
    /* link the bucket to the new entry */
    repository->bucket[index] = new_entry;
    
    /* update the entry counter */
    repository->entry_count++;
    
    /* set status and return string object */
    SET_STATUS(status, M2C_STRING_STATUS_SUCCESS);
    return new_string;
  }
  else /* bucket not empty */ {
    
    /* first entry in bucket is starting point */
    this_entry = repository->bucket[index];
    
    /* search bucket for matching string */
    while (true) {
      if /* match found in current entry */
        ((this_entry->key == key) &&
          matches_str_and_len(this_entry->str, str, length)) {
        
        /* get string object of matching entry and retain it */
        this_string = this_entry->str;
        m2c_string_retain(this_string);
        
        /* set status and return string object */
        SET_STATUS(status, M2C_STRING_STATUS_SUCCESS);
        return this_string;
      }
      else if /* last entry reached without match */
        (this_entry->next == NULL) {
        
        /* create a new string object */
        new_string =
          new_string_by_appending(str, append_str, str_len, append_str_len);
        
        /* create a new repository entry */
        new_entry = new_repo_entry(new_string, key);
        
        /* link last entry to the new entry */
        this_entry->next = new_entry;
        
        /* update the entry counter */
        repository->entry_count++;
        
        /* set status and return string object */
        SET_STATUS(status, M2C_STRING_STATUS_SUCCESS);
        return new_string;
      }
      else /* not last entry yet, move to next entry */ {
        this_entry = this_entry->next;
      } /* end if */
    } /* end while */      
  } /* end if */  
} /* end m2c_new_string_by_appending_string */


/* --------------------------------------------------------------------------
 * function m2c_string_length(str)
 * --------------------------------------------------------------------------
 * Returns the length of str.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry
 *
 * post-conditions:
 * o  the length of str is returned
 *
 * error-conditions:
 * o  if str is NULL upon entry, zero is returned
 * ----------------------------------------------------------------------- */

uint_t m2c_string_length (m2c_string_t str) {
  
  if (str == NULL) {
    return 0;
  } /* end if */
  
  return str->length;
} /* end m2c_string_length */


/* --------------------------------------------------------------------------
 * function m2c_string_char_ptr(str)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to str's character string.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry
 *
 * post-conditions:
 * o  an immutable pointer to the character string of str is returned
 *
 * error-conditions:
 * o  if str is NULL upon entry, NULL is returned
 * ----------------------------------------------------------------------- */

inline const char *m2c_string_char_ptr (m2c_string_t str) {
  
  if (str == NULL) {
    return NULL;
  } /* end if */
  
  return (const char *) &(str->char_array);
} /* end m2c_string_char_ptr */


/* --------------------------------------------------------------------------
 * function m2c_unique_string_count()
 * --------------------------------------------------------------------------
 * Returns the number of unique strings stored in the string repository.
 *
 * pre-conditions:
 * o  none
 *
 * post-conditions:
 * o  none
 *
 * error-conditions:
 * o  none
 * ----------------------------------------------------------------------- */

inline uint_t m2c_unique_string_count (void) {
  
  if (repository == NULL) {
    return 0;
  } /* end if */
  
  return repository->entry_count;
} /* end m2c_unique_string_count */


/* --------------------------------------------------------------------------
 * function m2c_string_retain(str)
 * --------------------------------------------------------------------------
 * Prevents str from deallocation.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry
 *
 * post-conditions:
 * o  str's reference count is incremented.
 *
 * error-conditions:
 * o  if str is not NULL upon entry, no operation is carried out
 * ----------------------------------------------------------------------- */

void m2c_string_retain (m2c_string_t str) {
  
  if ((str != NULL) && (str->ref_count > 0)) {
    str->ref_count++;
  } /* end if */
  
} /* end m2c_string_retain */


/* --------------------------------------------------------------------------
 * function m2c_string_release(str)
 * --------------------------------------------------------------------------
 * Cancels an outstanding retain, or deallocates str if there are no
 * outstanding retains.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry
 *
 * post-conditions:
 * o  if str's reference count is zero upon entry, str is deallocated
 * o  if str's reference count is not zero upon entry, it is decremented
 *
 * error-conditions:
 * o  if str is not NULL upon entry, no operation is carried out
 * ----------------------------------------------------------------------- */

void m2c_string_release (m2c_string_t str) {
  
  m2c_hash_t key;
  
  if (str == NULL) {
    return;
  } /* end if */
  
  /* printf("  release called on '%s', refcount %u\n",
    str->char_array, str->ref_count); */
  
  if (str->ref_count > 1) {
    str->ref_count--;
    /* printf("  new refcount %u\n", str->ref_count); */
  }
  else if (str->ref_count == 1) {
    /* remove and deallocate */
    printf("* deallocating string '%s' (%p)\n",
      m2c_string_char_ptr(str), str);
    
    /* remove from repo */
    key = key_for_string(str);
    remove_repo_entry(str, key);
    
    /* reset */
    str->length = 0;
    str->ref_count = 0;
    str->char_array[0] = ASCII_NUL;
    
    /* deallocate */
    free(str);
  } /* end if */
} /* end m2c_string_release */


/* *********************************************************************** *
 * Private Functions                                                       *
 * *********************************************************************** */

/* --------------------------------------------------------------------------
 * private function new_string_from_string(str, length)
 * --------------------------------------------------------------------------
 * Allocates and returns a new string object, initialised with str and
 * length.  Returns NULL if allocation failed.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry (NOT GUARDED)
 *
 * post-conditions:
 * o  newly allocated, initalised and retained string object is returned
 *
 * error-conditions:
 * o  if allocation fails, no operation is carried out, NULL is returned
 * ----------------------------------------------------------------------- */

static m2c_string_t new_string_from_string (char *str, uint_t length) {
  
  m2c_string_t new_string;
  uint_t index;
  
  /* allocate new string object */
  new_string = malloc(sizeof(m2c_string_struct_t) + length + 1);
  
  /* bail if allocation failed */
  if (new_string == NULL) {
    return NULL;
  } /* end if */
    
  /* initialise */
  new_string->ref_count = 1;
  new_string->length = length;
  
  /* copy string */
  index = 0;
  while (index < length) {
    new_string->char_array[index] = str[index];
    index++;
  } /* end while */
  
  /* terminate character array */
  new_string->char_array[index] = ASCII_NUL;
  
  return new_string;
} /* end new_string_from_string */


/* --------------------------------------------------------------------------
 * private function matches_str_and_len(str1, str2, length)
 * --------------------------------------------------------------------------
 * Compares character strings str1 and str2 up to length characters
 * and returns true if they match, otherwise false.
 *
 * pre-conditions:
 * o  parameters str1 and str2 must not be NULL upon entry (NOT GUARDED)
 *
 * post-conditions:
 * o  boolean result is returned
 *
 * error-conditions:
 * o  none
 * ----------------------------------------------------------------------- */

static bool matches_str_and_len
  (m2c_string_t str, const char *cmpstr, uint_t length) {
  
  uint_t index;
  
  if (str->length != length) {
    return false;
  } /* end if */
  
  index = 0;
  while (index <= length) {
    if (str->char_array[index] != cmpstr[index]) {
      return false;
    }
    else if (str->char_array[index] == ASCII_NUL) {
      return true;
    }
    else {
      index++;
    } /* end if */
  } /* end while */
  
  return false;
} /* end matches_str_and_len */


/* --------------------------------------------------------------------------
 * private function new_string_from_slice(str, offset, length)
 * --------------------------------------------------------------------------
 * Allocates and returns a new string object.  The string is initialised with
 * a slice of str starting at offset and ending at offset+length.
 * Returns NULL if allocation failed.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry (NOT GUARDED)
 *
 * post-conditions:
 * o  newly allocated, initalised and retained string object is returned
 *
 * error-conditions:
 * o  if allocation fails, no operation is carried out, NULL is returned
 * ----------------------------------------------------------------------- */

static m2c_string_t new_string_from_slice
  (char *str, uint_t offset, uint_t length) {
  
  m2c_string_t new_string;
  uint_t source_index, target_index;
  
  /* allocate new string object */
  new_string = malloc(sizeof(m2c_string_struct_t) + length + 1);
  
  /* check allocation, bail out on failure */
  if /* allocation failed */ (new_string == NULL) {
    return NULL;
  } /* end if */
    
  /* initialise */
  new_string->ref_count = 1;
  new_string->length = length;
  
  /* copy slice */
  target_index = 0;
  source_index = offset;
  while (target_index < length) {
    new_string->char_array[target_index] = str[source_index];
    target_index++;
    source_index++;
  } /* end while */
  
  /* terminate character array */
  new_string->char_array[target_index] = ASCII_NUL;
  
  return new_string;
} /* end new_string_from_slice */


/* --------------------------------------------------------------------------
 * private function new_string_by_appending(str, append_str, len, len2)
 * --------------------------------------------------------------------------
 * Allocates and returns a new string object.  The string is initialised with
 * the concatenation product of str and append_str.
 * Returns NULL if allocation failed.
 *
 * pre-conditions:
 * o  parameters str and append_str must not be NULL upon entry (NOT GUARDED)
 *
 * post-conditions:
 * o  newly allocated, initalised and retained string object is returned
 *
 * error-conditions:
 * o  if allocation fails, no operation is carried out, NULL is returned
 * ----------------------------------------------------------------------- */

static m2c_string_t new_string_by_appending
  (char *str, char *append_str, uint_t str_len, uint_t append_str_len) {
  
  m2c_string_t new_string;
  uint_t source_index, target_index, length;
  
  length = str_len + append_str_len;
  
  /* allocate new dynamic string */
  new_string = malloc(sizeof(m2c_string_struct_t) + length + 1);
  
  /* bail if allocation failed */
  if (new_string == NULL) {
    return NULL;
  } /* end if */
      
  /* initialise */
  new_string->ref_count = 1;
  new_string->length = length;
  
  target_index = 0;
  
  /* copy str */
  source_index = 0;
  while (target_index < str_len) {
    new_string->char_array[target_index] = str[source_index];
    target_index++;
    source_index++;
  } /* end while */
  
  /* copy append_str */
  source_index = 0;
  while (target_index < append_str_len) {
    new_string->char_array[target_index] = append_str[source_index];
    target_index++;
    source_index++;
  } /* end while */
  
  /* terminate character array */
  new_string->char_array[target_index] = ASCII_NUL;
  
  return new_string;
} /* end new_string_by_appending */


/* --------------------------------------------------------------------------
 * private function new_repo_entry(str, key)
 * --------------------------------------------------------------------------
 * Allocates and returns a new repository entry, initialised with str and
 * key.  Returns NULL if allocation failed.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry (NOT GUARDED)
 *
 * post-conditions:
 * o  newly allocated and initalised entry is returned
 *
 * error-conditions:
 * o  if allocation fails, no operation is carried out, NULL is returned
 * --------------------------------------------------------------------------
 */
 
static m2c_string_repo_entry_t new_repo_entry (m2c_string_t str, m2c_hash_t key) {
  
  m2c_string_repo_entry_t new_entry;
  
  /* allocate new entry */
  new_entry = malloc(sizeof(m2c_string_repo_entry_s));
  
  /* initialise new entry */
  if (new_entry != NULL) {
    new_entry->str = str;
    new_entry->key = key;
    new_entry->next = NULL;
  } /* end if */
  
  return new_entry;
} /* end new_repo_entry */


/* --------------------------------------------------------------------------
 * private function remove_repo_entry(str, key)
 * --------------------------------------------------------------------------
 * Removes the repository entry for str and key.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry (NOT GUARDED)
 *
 * post-conditions:
 * o  if entry for str and key exists, entry is removed, str is returned
 *
 * error-conditions:
 * o  if entry for str and key does not exist, NULL is returned
 * ----------------------------------------------------------------------- */

static m2c_string_t remove_repo_entry (m2c_string_t str, m2c_hash_t key) {
  
  m2c_string_repo_entry_t this_entry, prev_entry;
  uint_t index;
  
  /* determine bucket index */
  index = key % repository->bucket_count;
  
  /* check first entry of bucket for matching string */
  this_entry = repository->bucket[index];
  if (str == this_entry->str) {
    repository->bucket[index] = this_entry->next;
    repository->entry_count--;
    free(this_entry);
    return str;
  } /* end if */
  
  /* move to next entry */
  prev_entry = this_entry;
  this_entry = this_entry->next;
  
  /* search remainder of bucket for matching string */
  while (this_entry != NULL) {
    if (str == this_entry->str) {
      /* match, remove entry */
      prev_entry->next = this_entry->next;
      repository->entry_count--;
      free(this_entry);
      return str;
    }
    else /* no match */ {
      /* move to next entry */
      prev_entry = this_entry;
      this_entry = this_entry->next;
    } /* end if */
  } /* end while */
  
  /* no match in bucket */  
  return NULL;
} /* end remove_repo_entry */


/* --------------------------------------------------------------------------
 * private function key_for_string(str)
 * --------------------------------------------------------------------------
 * Calculates and returns the hash key for string str.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry (NOT GUARDED)
 *
 * post-conditions:
 * o  hash key is returned
 *
 * error-conditions:
 * o  none
 * ----------------------------------------------------------------------- */

static inline m2c_hash_t key_for_string (m2c_string_t str) {
  m2c_hash_t key;
  uint_t index;
  char ch;
  
  index = 0;
  ch = str->char_array[index];
  key = HASH_INITIAL;
  while (ch != ASCII_NUL) {
    key = HASH_NEXT_CHAR(key, ch);
    index++;
    ch = str->char_array[index];
  } /* end while */

  key = HASH_FINAL(key);
  
  return key;  
} /* end key_for_string */


/* END OF FILE */