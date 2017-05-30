/* M2C Modula-2 Compiler & Translator
 * Copyright (c) 2015-2016 Benjamin Kowarsch
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
 * m2-fifo.c
 *
 * Implementation of M2C generic queue.
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

#include "m2-fifo.h"

#include <stdlib.h>


/* --------------------------------------------------------------------------
 * private type m2c_fifo_segment_t
 * --------------------------------------------------------------------------
 * pointer type representing a FIFO queue segment.
 * ----------------------------------------------------------------------- */

typedef struct m2c_fifo_segment_s *m2c_fifo_segment_t;

struct m2c_fifo_segment_s {
  /* next */ m2c_fifo_segment_t next;
  /* table */ m2c_fifo_value_t table[M2C_FIFO_SEGMENT_SIZE];
};

typedef struct m2c_fifo_segment_s m2c_fifo_segment_s;


/* --------------------------------------------------------------------------
 * hidden type m2c_fifo_struct_t
 * --------------------------------------------------------------------------
 * record type representing a FIFO queue object.
 * ----------------------------------------------------------------------- */

struct m2c_fifo_struct_t {
  /* entry_count */ uint_t entry_count;
  /* head_index */ uint_t head_index;
  /* tail_index */ uint_t tail_index;
  /* next */ m2c_fifo_segment_t next;
  /* table */ m2c_fifo_value_t table[M2C_FIFO_SEGMENT_SIZE];
};

typedef struct m2c_fifo_struct_t m2c_fifo_struct_t;


/* --------------------------------------------------------------------------
 * forward declarations
 * ----------------------------------------------------------------------- */

static m2c_fifo_segment_t alloc_init_segment (void);


/* --------------------------------------------------------------------------
 * function m2c_fifo_new_queue(first_value)
 * --------------------------------------------------------------------------
 * Allocates a new queue object with an initial size of init_size, stores a
 * first value first_value and returns the queue.  Returns NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_fifo_t m2c_fifo_new_queue (m2c_fifo_value_t first_value) {
  
  m2c_fifo_t new_queue;
  uint_t index;
  
  /* allocate new queue */
  new_queue = malloc
    (sizeof(m2c_fifo_struct_t) +
     M2C_FIFO_SEGMENT_SIZE * sizeof(m2c_fifo_value_t));
  
  if (new_queue == NULL) {
    return NULL;
  } /* end if */
  
  if (first_value == NULL) {
    new_queue->entry_count = 0;
  }
  else {
    new_queue->entry_count = 1;
  } /* end if */ 
  
  new_queue->head_index = 0;
  new_queue->tail_index = 0;
  new_queue->next = NULL;
  
  new_queue->table[0] = first_value;
  
  for (index = 1; index < M2C_FIFO_SEGMENT_SIZE; index++) {
    new_queue->table[index] = NULL;
  } /* end for */
  
  return new_queue;
} /* end m2c_fifo_new_queue */


/* --------------------------------------------------------------------------
 * function m2c_fifo_enqueue(queue, new_value)
 * --------------------------------------------------------------------------
 * Adds a value to the head of queue and returns queue, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_fifo_t m2c_fifo_enqueue (m2c_fifo_t queue, m2c_fifo_value_t new_value) {
  
  uint_t index, new_tail_index, this_segment_index, target_segment_index;
  m2c_fifo_segment_t this_segment, new_segment;
  
  if ((queue == NULL) || (new_value == NULL)) {
    return NULL;
  } /* end if */ 
  
  if (queue->entry_count > 0) {
    new_tail_index = queue->tail_index + 1;
  } /* end if */
  
  /* check if tail is within base segment */
  if (new_tail_index < M2C_FIFO_SEGMENT_SIZE) {
    /* store value in base segment */
    queue->table[new_tail_index] = new_value;
    queue->entry_count++;
    queue->tail_index++;
    return queue;
  } /* end if */
  
  if (queue->next == NULL) {
    new_segment = alloc_init_segment();
    
    if (new_segment == NULL) {
      return NULL;
    } /* end if */
    
    queue->next = new_segment;
  } /* end if */
  
  this_segment_index = 1;
  this_segment = queue->next;
  target_segment_index = new_tail_index / M2C_FIFO_SEGMENT_SIZE;
  
  /* find target segment */
  while (this_segment_index != target_segment_index) {
    
    if (this_segment->next == NULL) {
      /* allocate new segment */
      new_segment = alloc_init_segment();
      
      if (new_segment == NULL) {
        return NULL;
      } /* end if */
      
      this_segment->next = new_segment;
    } /* end if */
    
    this_segment = this_segment->next;
    this_segment_index++;
  } /* end while */
  
  /* store new value in target segment */
  index = new_tail_index % M2C_FIFO_SEGMENT_SIZE;
  this_segment->table[index] = new_value;
  queue->entry_count++;
  queue->tail_index++;
  
  return queue;
} /* end m2c_fifo_enqueue */


/* --------------------------------------------------------------------------
 * function m2c_fifo_enqueue_unique(queue, new_value)
 * --------------------------------------------------------------------------
 * Adds a value to the head of queue if and only if the value is not already
 * present in queue.  Returns queue on success, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_fifo_t m2c_fifo_enqueue_unique
  (m2c_fifo_t queue, m2c_fifo_value_t new_value) {
  
  if (m2c_fifo_entry_exists(queue, new_value)) {
    return NULL;
  } /* end if */
  
  return m2c_fifo_enqueue(queue, new_value);
  
} /* end m2c_fifo_enqueue_unique */


/* --------------------------------------------------------------------------
 * function m2c_fifo_dequeue(queue)
 * --------------------------------------------------------------------------
 * Removes the value at the tail of queue and returns it, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_fifo_value_t m2c_fifo_dequeue (m2c_fifo_t queue) {
  
  m2c_fifo_value_t head_value;
  m2c_fifo_segment_t this_segment;
  uint_t head_index, table_index, this_segment_index, target_segment_index;
  
  if ((queue == NULL) || (queue->entry_count == 0)) {
    return NULL;
  } /* end if */ 
  
  head_index = queue->head_index;
  
  /* check if head entry is within base segment */
  if (queue->head_index < M2C_FIFO_SEGMENT_SIZE) {
    
    /* remove head entry */
    head_value = queue->table[head_index];
    queue->table[head_index] = NULL;
    queue->entry_count--;
    
    if (queue->entry_count == 0) {
      queue->head_index = 0;
      queue->tail_index = 0;
    }
    else {
      queue->head_index++;
    } /* end if */
    
    /* and return it */
    return head_value;
  } /* end if */
  
  this_segment_index = 1;
  this_segment = queue->next;
  target_segment_index = head_index / M2C_FIFO_SEGMENT_SIZE;
  
  /* find target segment */
  while (this_segment_index != target_segment_index) {
    
    this_segment = this_segment->next;
    this_segment_index++;
    
    if (this_segment == NULL) {
      return NULL;
    } /* end if */
  } /* end while */
  
  /* remove head entry */
  table_index = head_index % M2C_FIFO_SEGMENT_SIZE;
  head_value = this_segment->table[table_index];
  this_segment->table[table_index] = NULL;
  queue->entry_count--;
  
  if (queue->entry_count == 0) {
    queue->head_index = 0;
    queue->tail_index = 0;
  }
  else {
    queue->head_index++;
  } /* end if */
  
  /* and return it */
  return head_value;
} /* end m2c_fifo_dequeue */


/* --------------------------------------------------------------------------
 * function m2c_fifo_entry_count(queue)
 * --------------------------------------------------------------------------
 * Returns the number of values stored in queue. 
 * ----------------------------------------------------------------------- */

uint_t m2c_fifo_entry_count (m2c_fifo_t queue) {
  
  if (queue == NULL) {
   return 0;
  } /* end if */  
  
  return queue->entry_count;
} /* end m2c_fifo_entry_count */


/* --------------------------------------------------------------------------
 * function m2c_fifo_entry_exists(queue, value)
 * --------------------------------------------------------------------------
 * Returns true if value is present in queue, otherwise false. 
 * ----------------------------------------------------------------------- */

bool m2c_fifo_entry_exists (m2c_fifo_t queue, m2c_fifo_value_t value) {
  
  m2c_fifo_segment_t this_segment;
  uint_t index, table_index, this_segment_index, target_segment_index;
  
  if ((queue == NULL) || (value == NULL)) {
    return false;
  } /* end if */ 
  
  /*** case 1 : both head and tail in base segment ***/
  
  if (queue->head_index < M2C_FIFO_SEGMENT_SIZE) {
    if (queue->tail_index < M2C_FIFO_SEGMENT_SIZE) {
      /* search in base segment */
      for (index = queue->head_index;
           index <= queue->tail_index; index++) {
        if /* not unique */ (queue->table[index] == value) {
          return true;
        } /* end if */
      } /* end for */
      return false;
    }
    
  /*** case 2 : head in base segment, tail further down ***/
  
    else {
      /* search in base segment */
      for (index = queue->head_index;
           index < M2C_FIFO_SEGMENT_SIZE; index++) {
        if /* not unique */ (queue->table[index] == value) {
          return true;
        } /* end if */
      } /* end for */
      
      /* proceed to next segment */
      index = 0;
      this_segment = queue->next;
    } /* end if */
  }
  
  /*** case 3 : both head and tail further down ***/
  
  else {
    /* set table index from where to commence */
    index = queue->head_index;
    
    /* set segment from where to commence */
    this_segment_index = 1;
    this_segment = queue->next;
    target_segment_index = queue->head_index / M2C_FIFO_SEGMENT_SIZE;
    
    /* move to target segment */
    while (this_segment_index < target_segment_index) {
      this_segment = this_segment->next;
      this_segment_index++;
    } /* end while */
  } /* end if */
  
  /*** continue search for cases 2 and 3 ***/
  
  while (index < queue->tail_index) {
    table_index = index % M2C_FIFO_SEGMENT_SIZE;
    
    if (this_segment->table[table_index] == value) {
      return true;
    } /* end if */
    
    if ((table_index + 1) == M2C_FIFO_SEGMENT_SIZE) {
      this_segment = this_segment->next;
    } /* end if */
    
    index++;
  } /* end while */ 
  
  return false;
} /* end m2c_fifo_entry_exists */


/* --------------------------------------------------------------------------
 * function m2c_fifo_reset_queue(queue)
 * --------------------------------------------------------------------------
 * Removes all entries from queue but does not deallocate it.
 * Returns the queue on success or NULL if queue is NULL.
 * ----------------------------------------------------------------------- */

m2c_fifo_t m2c_fifo_reset_queue (m2c_fifo_t queue) {
  
  m2c_fifo_segment_t this_segment;
  uint_t index;
  
  if (queue == NULL) {
   return NULL;
  } /* end if */  
  
  /* reset meta data */
  queue->entry_count = 0;
  queue->head_index = 0;
  queue->tail_index = 0;
  
  /* reset all entries in base segment */
  for (index = 0; index < M2C_FIFO_SEGMENT_SIZE; index++) {
    queue->table[index] = NULL;
  } /* end for */
  
  this_segment = queue->next;
  
  while (this_segment != NULL) {
    /* reset all entries in this segment */
    for (index = 0; index < M2C_FIFO_SEGMENT_SIZE; index++) {
      this_segment->table[index] = NULL;
    } /* end for */
    
    /* next segment */
    this_segment = this_segment->next;
  } /* end while */
  
  return queue;
} /* end m2c_fifo_reset_queue */


/* --------------------------------------------------------------------------
 * function m2c_fifo_release_queue(queue)
 * --------------------------------------------------------------------------
 * Deallocates queue. 
 * ----------------------------------------------------------------------- */

void m2c_fifo_release_queue (m2c_fifo_t queue) {
  
  if (queue == NULL) {
   return;
  } /* end if */  
  
  queue->entry_count = 0;
  queue->head_index = 0;
  queue->tail_index = 0;
  free(queue);
  
  return;
} /* end m2c_fifo_release_queue */


/* --------------------------------------------------------------------------
 * private function alloc_init_segment()
 * ----------------------------------------------------------------------- */

static m2c_fifo_segment_t alloc_init_segment (void) {
  
  m2c_fifo_segment_t new_segment;
  uint_t index;
  
  /* allocate a new segment */
  new_segment = malloc
    (sizeof(m2c_fifo_segment_s) +
     M2C_FIFO_SEGMENT_SIZE * sizeof(m2c_fifo_value_t));
  
  if (new_segment == NULL) {
    return NULL;
  } /* end if */
  
  /* initialise new segment */
  new_segment->next = NULL;
  for (index = 0; index < M2C_FIFO_SEGMENT_SIZE; index++) {
    new_segment->table[index] = NULL;
  } /* end for */
  
  return new_segment;
} /* end alloc_init_segment */


/* END OF FILE */