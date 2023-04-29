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
 * m2-mutable-strlist.h
 *
 * Public interface for M2C mutable string list module.
 *
 * @license
 *
 * M2C is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation;  either version 2 of the License (GPL2),
 * or (at your option) version 3 (GPL3).
 *
 * M2C is distributed in the hope that it will be useful,  but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with m2c.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef M2C_MUTABLE_STRLIST_H
#define M2C_MUTABLE_STRLIST_H

#include "m2-common.h"
#include "m2-unique-string.h"

#include <stdbool.h>


/* --------------------------------------------------------------------------
 * opaque type m2c_mutable_strlist_t
 * --------------------------------------------------------------------------
 * Opaque pointer type representing a mutable string list object.
 * ----------------------------------------------------------------------- */

typedef struct m2c_mutable_strlist_struct_t *m2c_mutable_strlist_t;


/* --------------------------------------------------------------------------
 * type m2c_strlist_status_t
 * --------------------------------------------------------------------------
 * Status codes for operations on type m2c_mutable_strlist_t.
 * ----------------------------------------------------------------------- */

typedef enum {
  M2C_STRLIST_STATUS_SUCCESS,
  M2C_STRLIST_STATUS_INVALID_REFERENCE,
  M2C_STRLIST_STATUS_ALLOCATION_FAILED,
  M2C_STRLIST_STATUS_DUPLICATE_ENTRY
} m2c_strlist_status_t;


/* --------------------------------------------------------------------------
 * function m2c_new_mutable_strlist(first_entry, status)
 * --------------------------------------------------------------------------
 * Returns a new mutable string list object with string first_entry.
 * ----------------------------------------------------------------------- */

m2c_mutable_strlist_t m2c_new_mutable_strlist
  (m2c_string_t first_entry, m2c_strlist_status_t *status);


/* --------------------------------------------------------------------------
 * function m2c_mutable_strlist_append(list, new_entry)
 * --------------------------------------------------------------------------
 * Appends a new string entry to a given list and returns a status.
 * The new entry is not appended if it already exists within list.
 * ----------------------------------------------------------------------- */

m2c_strlist_status_t m2c_mutable_strlist_append
  (m2c_mutable_strlist_t list, m2c_string_t new_entry);


/* --------------------------------------------------------------------------
 * function m2c_mutable_strlist_entry_at_index(list, index)
 * --------------------------------------------------------------------------
 * Returns the string entry stored at the given index in the given list.
 * Returns NULL if list is NULL or if no entry exists for the given index.
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_mutable_strlist_entry_at_index
  (m2c_mutable_strlist_t list, uint_t index);


/* --------------------------------------------------------------------------
 * function m2c_mutable_strlist_entry_exists(list, string)
 * --------------------------------------------------------------------------
 * Returns true if the given string exists in the given string list.
 * ----------------------------------------------------------------------- */

bool m2c_mutable_strlist_entry_exists
  (m2c_mutable_strlist_t list, m2c_string_t string);


/* --------------------------------------------------------------------------
 * function m2c_mutable_strlist_entry_count(list)
 * --------------------------------------------------------------------------
 * Returns the entry count of the given string list.
 * ----------------------------------------------------------------------- */

uint_t m2c_mutable_strlist_entry_count (m2c_mutable_strlist_t list);


/* --------------------------------------------------------------------------
 * procedure m2c_mutable_strlist_release(list)
 * --------------------------------------------------------------------------
 * Releases the given string list. 
 * ----------------------------------------------------------------------- */

void m2c_mutable_strlist_release (m2c_mutable_strlist_t list);

void m2c_print_strlist (m2c_mutable_strlist_t list);

#endif /* M2C_MUTABLE_STRLIST_H */

/* END OF FILE */