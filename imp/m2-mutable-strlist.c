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


#include "m2-mutable-strlist.h"

#include <stdlib.h>
#include <stddef.h>


#include <stdio.h> /* debugging only */


/* --------------------------------------------------------------------------
 * hidden type m2c_strlist_table_t
 * --------------------------------------------------------------------------
 * Implementation of mutable string list segment table.
 * ----------------------------------------------------------------------- */

#define M2C_MUTABLE_STRLIST_SEGMENT_SIZE 16

typedef m2c_string_t m2c_strlist_table_t[M2C_MUTABLE_STRLIST_SEGMENT_SIZE];


/* --------------------------------------------------------------------------
 * hidden type m2c_strlist_segment_t
 * --------------------------------------------------------------------------
 * Implementation of mutable string list segment.
 * ----------------------------------------------------------------------- */

typedef struct m2c_strlist_segment_s *m2c_strlist_segment_t;

struct m2c_strlist_segment_s {
  /* next */ m2c_strlist_segment_t next;
  /* table */ m2c_strlist_table_t table;
};

typedef struct m2c_strlist_segment_s m2c_strlist_segment_s;


/* --------------------------------------------------------------------------
 * hidden type m2c_mutable_strlist_s
 * --------------------------------------------------------------------------
 * Implementation of mutable string list object type.
 * ----------------------------------------------------------------------- */

struct m2c_mutable_strlist_struct_t {
  /* entry_count */ uint_t entry_count;
  /* table */ m2c_strlist_table_t table;
  /* next */ m2c_strlist_segment_t next;
  /* last */ m2c_strlist_segment_t last;
};

typedef struct m2c_mutable_strlist_struct_t m2c_mutable_strlist_struct_t;


/* --------------------------------------------------------------------------
 * function m2c_new_mutable_strlist(first_entry, status)
 * --------------------------------------------------------------------------
 * Returns a new mutable string list object with string first_entry.
 * ----------------------------------------------------------------------- */

m2c_mutable_strlist_t m2c_new_mutable_strlist
  (m2c_string_t first_entry, m2c_strlist_status_t *status) {
  
  m2c_mutable_strlist_t new_list;
  uint_t index;
  
  if (first_entry == NULL) {
    SET_STATUS(status, M2C_STRLIST_STATUS_INVALID_REFERENCE);
    return NULL;
  } /* end if */
    
  new_list = malloc(sizeof(m2c_mutable_strlist_struct_t));
  
  if (new_list == NULL) {
    SET_STATUS(status, M2C_STRLIST_STATUS_ALLOCATION_FAILED);
    return NULL;
  } /* end if */
  
  new_list->entry_count = 0;
  
  for (index = 0; index < M2C_MUTABLE_STRLIST_SEGMENT_SIZE; index++) {
    new_list->table[index] = NULL;
  } /* end for */
  
  if (first_entry != NULL) {
    new_list->table[0] = first_entry;
    m2c_string_retain(first_entry);
    new_list->entry_count++;
  } /* end if */
  
  new_list->next = NULL;
  new_list->last = NULL;
  
  SET_STATUS(status, M2C_STRLIST_STATUS_SUCCESS);
  return new_list;
} /* end m2c_new_mutable_strlist */


void m2c_print_strlist (m2c_mutable_strlist_t list) {
  uint_t index;
  
  for (index = 0; index < M2C_MUTABLE_STRLIST_SEGMENT_SIZE; index++) {
    printf(" list->table[%u] : '%s' (%p)\n",
      index, m2c_string_char_ptr(list->table[index]),list->table[index]);
    if (list->table[index] == NULL) {
      break;
    } /* end for */
  } /* end for */
  
  return;
} /* end m2c_print_strlist */


/* --------------------------------------------------------------------------
 * function m2c_mutable_strlist_append(list, new_entry)
 * --------------------------------------------------------------------------
 * Appends a new string entry to a given list and returns a status.
 * The new entry is not appended if it already exists within list.
 * ----------------------------------------------------------------------- */

m2c_strlist_status_t m2c_mutable_strlist_append
  (m2c_mutable_strlist_t list, m2c_string_t new_entry) {
  
  m2c_strlist_segment_t segment, new_segment;
  uint_t index;
  
  if ((list == NULL) || (new_entry == NULL)) {
    return M2C_STRLIST_STATUS_INVALID_REFERENCE;
  } /* end if */
  
  printf("appending '%s' (%p) to list\n",
    m2c_string_char_ptr(new_entry), new_entry);
  
  /* check for duplicate in base table */
  for (index = 0; index < M2C_MUTABLE_STRLIST_SEGMENT_SIZE; index++) {
    if (list->table[index] == new_entry) {
      return M2C_STRLIST_STATUS_DUPLICATE_ENTRY;
    }
    else if (list->table[index] == NULL) {
      list->table[index] = new_entry;
      m2c_string_retain(new_entry);
      list->entry_count++;
      return M2C_STRLIST_STATUS_SUCCESS;
    } /* end if */
  } /* end for */
  
  /* check for duplicate in segment tables */
  segment = list->next;
  while (segment != NULL) {
    for (index = 0; index < M2C_MUTABLE_STRLIST_SEGMENT_SIZE; index++) {
      if (segment->table[index] == new_entry) {
        return M2C_STRLIST_STATUS_DUPLICATE_ENTRY;
      }
      else if (segment->table[index] == NULL) {
        segment->table[index] = new_entry;
        m2c_string_retain(new_entry);
        list->entry_count++;
        return M2C_STRLIST_STATUS_SUCCESS;
      } /* end if */
    } /* end for */
    segment = segment->next;
  } /* end while */
  
  /* allocate new segment */
  new_segment = malloc(sizeof(m2c_strlist_segment_s));
  
  if (new_segment == NULL) {
    return M2C_STRLIST_STATUS_ALLOCATION_FAILED;
  } /* end if */
  
  /* initialise new segment */
  new_segment->next = NULL;
  
  new_segment->table[0] = new_entry;
  m2c_string_retain(new_entry);
  list->entry_count++;
  
  for (index = 1; index < M2C_MUTABLE_STRLIST_SEGMENT_SIZE; index++) {
    new_segment->table[index] = NULL;
  } /* end if */
  
  /* link to end of list */
  list->last->next = new_segment;
  list->last = list->last->next;
  
  return M2C_STRLIST_STATUS_SUCCESS;
} /* end m2c_mutable_strlist_append */


/* --------------------------------------------------------------------------
 * function m2c_mutable_strlist_entry_at_index(list, index)
 * --------------------------------------------------------------------------
 * Returns the string entry stored at the given index in the given list.
 * Returns NULL if list is NULL or if no entry exists for the given index.
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_mutable_strlist_entry_at_index
  (m2c_mutable_strlist_t list, uint_t index) {
  
  m2c_strlist_segment_t segment;
  uint_t running_seg_index, target_seg_index;
  
  if ((list == NULL) || (index >= list->entry_count)) {
    return NULL;
  } /* end if */
  
  if (index < M2C_MUTABLE_STRLIST_SEGMENT_SIZE) {
    return list->table[index];
  }
  else {
    segment = list->next;
    running_seg_index = 1;
    target_seg_index = index / M2C_MUTABLE_STRLIST_SEGMENT_SIZE;
    
    while ((segment != NULL) && (running_seg_index != target_seg_index)) {
      segment = segment->next;
      running_seg_index++;
    } /* end while */
    
    return segment->table[index % M2C_MUTABLE_STRLIST_SEGMENT_SIZE];
  } /* end if */
  
} /* end m2c_mutable_strlist_entry_at_index */


/* --------------------------------------------------------------------------
 * function m2c_mutable_strlist_entry_exists(list, string)
 * --------------------------------------------------------------------------
 * Returns true if the given string exists in the given string list.
 * ----------------------------------------------------------------------- */

bool m2c_mutable_strlist_entry_exists
  (m2c_mutable_strlist_t list, m2c_string_t string) {
  
  m2c_strlist_segment_t segment;
  uint_t index;
  
  if ((list == NULL) || (string == NULL)) {
    return false;
  } /* end if */
  
  for (index = 0; index < M2C_MUTABLE_STRLIST_SEGMENT_SIZE; index++) {
    if (list->table[index] == string) {
      return true;
    }
    else if (list->table[index] == NULL) {
      return false;
    } /* end if */
  } /* end for */
  
  segment = list->next;
  while (segment != NULL) {
    for (index = 0; index < M2C_MUTABLE_STRLIST_SEGMENT_SIZE; index++) {
      if (segment->table[index] == string) {
        return true;
      }
      else if (segment->table[index] == NULL) {
        return false;
      } /* end if */
    } /* end for */
    segment = segment->next;
  } /* end while */
  
  return false;
} /* end m2c_mutable_strlist_entry_exists */


/* --------------------------------------------------------------------------
 * function m2c_mutable_strlist_entry_count(list)
 * --------------------------------------------------------------------------
 * Returns the entry count of the given string list.
 * ----------------------------------------------------------------------- */

uint_t m2c_mutable_strlist_entry_count (m2c_mutable_strlist_t list) {
  
  if (list == NULL) {
    return 0;
  } /* end if */
  
  return list->entry_count;
} /* end m2c_mutable_strlist_entry_count */


/* --------------------------------------------------------------------------
 * procedure m2c_mutable_strlist_release(list)
 * --------------------------------------------------------------------------
 * Releases the given string list. 
 * ----------------------------------------------------------------------- */

void m2c_mutable_strlist_release (m2c_mutable_strlist_t list) {
  
  m2c_strlist_segment_t segment, prev_segment;
  
  if (list == NULL) {
    return;
  } /* end if */
  
  segment = list->next;
  while (segment != NULL) {
    prev_segment = segment;
    segment = segment->next;
    free(prev_segment);
  } /* end while */
  
  free(list);
  return;
} /* end m2c_mutable_strlist_release */

/* END OF FILE */