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
 * m2-unique-strlist.h
 *
 * Public interface for M2C unique string list module.
 *
 * The module provides a list ADT for unique strings.
 *
 * @license
 *
 * M2C is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation;  either version 2 of the License (GPL2),
 * or (at your option) any later version.
 *
 * M2C is distributed in the hope that it will be useful,  but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with m2c.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef M2C_UNIQUE_STRLIST_H
#define M2C_UNIQUE_STRLIST_H

#include "m2-common.h"


/* --------------------------------------------------------------------------
 * opaque type m2c_strlist_t
 * --------------------------------------------------------------------------
 * Opaque pointer type representing a list of unique strings.
 * ----------------------------------------------------------------------- */

typedef struct m2c_strlist_struct_t *m2c_strlist_t;


/* --------------------------------------------------------------------------
 * type m2c_strlist_status_t
 * --------------------------------------------------------------------------
 * Status codes for operations on type m2c_strlist_t.
 * ----------------------------------------------------------------------- */

typedef enum {
  M2C_STRLIST_STATUS_SUCCESS,
  M2C_STRLIST_STATUS_INVALID_REFERENCE,
  M2C_STRLIST_STATUS_DUPLICATE_STRING,
  M2C_STRLIST_STATUS_INVALID_INDEX,
  M2C_STRLIST_STATUS_ALLOCATION_FAILED,
} m2c_strlist_status_t;


m2c_strlist_t m2c_new_strlist
  (m2c_string_t first_entry, m2c_strlist_status_t *status);

bool m2c_strlist_add_unique_entry (m2c_strlist_t list, m2c_string_t entry);

m2c_string_t m2c_strlist_entry_at_index (m2c_strlist_t list, uint_t index);

uint_t m2c_strlist_entry_count (m2c_strlist_t list);

m2c_strlist_status_t m2c_strlist_last_status (m2c_strlist_t list);

void m2c_strlist_release (m2c_strlist_t list);


typedef struct m2c_strlist_node_struct_t *m2c_strlist_node_t;

typedef struct m2c_strlist_node_struct_t {
  m2c_string_t str;
  m2c_strlist_node_t next;
}; /* end m2c_strlist_node_struct_t */

typedef struct m2c_strlist_node_struct_t m2c_strlist_node_struct_t;


typedef struct m2c_strlist_struct_t {
  m2c_strlist_node_t first;
  m2c_strlist_node_t next;
  m2c_strlist_node_t last;
  uint_t next_index;
  uint_t entry_count;
}; /* end m2c_strlist_struct_t */

typdef struct m2c_strlist_struct_t m2c_strlist_struct_t;


m2c_mutable_strlist_t m2c_new_mutable_strlist
  (m2c_string_t first_entry, m2c_strlist_status_t *status);

uint_t m2c_mutable_strlist_entry_count (m2c_mutable_strlist_t list);

m2c_string_t m2c_mutable_strlist_entry_at_index
  (m2c_mutable_strlist_t list, uint_t index);

void m2c_mutable_strlist_release (m2c_strlist_t list);


new_str_list(list)


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
 * --------------------------------------------------------------------------
 */

void m2c_init_string_repository
  (uint_t size, m2c_string_status_t *status);


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
 * --------------------------------------------------------------------------
 */

m2c_string_t m2c_get_string (char *str, m2c_string_status_t *status);


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
 * --------------------------------------------------------------------------
 */

m2c_string_t m2c_get_string_for_slice
  (char *str, uint_t offset, uint_t length, m2c_string_status_t *status);


/* --------------------------------------------------------------------------
 * function m2c_get_string_for_concatenation(str, append_str, status)
 * --------------------------------------------------------------------------
 * Returns a unique string object for the character string resulting from
 * concatenation of str and append_str.  Parameters str and append_str must
 * be pointers to NUL terminated character strings.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry
 * o  parameter append_str must not be NULL upon entry
 *
 * post-conditions:
 * o  if a string object for the resulting concatenation string is present in
 *    the internal repository, that string object is retrieved, retained and
 *    returned.
 * o  if no string object for the resulting concatenation is present in the
 *    repository, a new string object with a copy of the concatenation is
 *    created, stored and returned.
 * o  M2C_STRING_STATUS_SUCCESS is passed back in status, unless NULL
 *
 * error-conditions:
 * o  if any of str or append_str is NULL upon entry, no operation is
 *    carried out, NULL is returned and M2C_STRING_STATUS_INVALID_REFERENCE
 *    is passed back in status, unless NULL
 * o  if no dynamic string object could be allocated, NULL is returned,
 *    and M2C_STRING_STATUS_ALLOCATION_FAILED is passed back in status,
 *    unless NULL
 * --------------------------------------------------------------------------
 */

m2c_string_t m2c_get_string_for_concatenation
  (char *str, char *append_str, m2c_string_status_t *status);


/* --------------------------------------------------------------------------
 * function m2c_string_length(str)
 * --------------------------------------------------------------------------
 * Returns the length of the character string of str.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry
 *
 * post-conditions:
 * o  the length of str's character string is returned
 *
 * error-conditions:
 * o  if str is NULL upon entry, zero is returned
 * --------------------------------------------------------------------------
 */

inline uint_t m2c_string_length (m2c_string_t str);


/* --------------------------------------------------------------------------
 * function m2c_string_char_ptr(str)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to the character string of str.
 *
 * pre-conditions:
 * o  parameter str must not be NULL upon entry
 *
 * post-conditions:
 * o  an immutable pointer to str's character string is returned
 *
 * error-conditions:
 * o  if str is NULL upon entry, NULL is returned
 * --------------------------------------------------------------------------
 */

inline const char *m2c_string_char_ptr (m2c_string_t str);


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
 * --------------------------------------------------------------------------
 */

inline uint_t m2c_unique_string_count (void);


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
 * --------------------------------------------------------------------------
 */

inline void m2c_string_retain (m2c_string_t str);


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
 * --------------------------------------------------------------------------
 */

void m2c_string_release (m2c_string_t str);


#endif /* M2C_UNIQUE_STRLIST_H */

/* END OF FILE */