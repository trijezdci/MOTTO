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
 * m2-fifo.h
 *
 * Public interface for M2C generic queue.
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

#ifndef M2C_FIFO_H
#define M2C_FIFO_H

#include "m2-common.h"

#include <stdbool.h>


/* --------------------------------------------------------------------------
 * Number of entries per FIFO queue segment
 * ----------------------------------------------------------------------- */

#define M2C_FIFO_SEGMENT_SIZE 16


/* --------------------------------------------------------------------------
 * type m2c_fifo_value_t
 * --------------------------------------------------------------------------
 * Generic pointer type representing a value for storage in a FIFO queue.
 * ----------------------------------------------------------------------- */

typedef void *m2c_fifo_value_t;


/* --------------------------------------------------------------------------
 * opaque type m2c_fifo_t
 * --------------------------------------------------------------------------
 * Opaque pointer type representing a FIFO queue object.
 * ----------------------------------------------------------------------- */

typedef struct m2c_fifo_struct_t *m2c_fifo_t;


void print_fifo_info(m2c_fifo_t q);


/* --------------------------------------------------------------------------
 * function m2c_fifo_new_queue(first_value)
 * --------------------------------------------------------------------------
 * Allocates a new queue object with an initial size of init_size, stores a
 * first value first_value and returns the queue.  Returns NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_fifo_t m2c_fifo_new_queue (m2c_fifo_value_t first_value);


/* --------------------------------------------------------------------------
 * function m2c_fifo_enqueue(queue, new_value)
 * --------------------------------------------------------------------------
 * Adds a value to the head of queue and returns queue, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_fifo_t m2c_fifo_enqueue (m2c_fifo_t queue, m2c_fifo_value_t new_value);


/* --------------------------------------------------------------------------
 * function m2c_fifo_enqueue_unique(queue, new_value)
 * --------------------------------------------------------------------------
 * Adds a value to the head of queue if and only if the value is not already
 * present in queue.  Returns queue on success, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_fifo_t m2c_fifo_enqueue_unique
  (m2c_fifo_t queue, m2c_fifo_value_t new_value);


/* --------------------------------------------------------------------------
 * function m2c_fifo_dequeue(queue)
 * --------------------------------------------------------------------------
 * Removes the value at the tail of queue and returns it, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_fifo_value_t m2c_fifo_dequeue (m2c_fifo_t queue);


/* --------------------------------------------------------------------------
 * function m2c_fifo_entry_count(queue)
 * --------------------------------------------------------------------------
 * Returns the number of values present in queue. 
 * ----------------------------------------------------------------------- */

uint_t m2c_fifo_entry_count (m2c_fifo_t queue);


/* --------------------------------------------------------------------------
 * function m2c_fifo_entry_exists(queue, value)
 * --------------------------------------------------------------------------
 * Returns true if value is present in queue, otherwise false. 
 * ----------------------------------------------------------------------- */

bool m2c_fifo_entry_exists (m2c_fifo_t queue, m2c_fifo_value_t value);


/* --------------------------------------------------------------------------
 * function m2c_fifo_reset_queue(queue)
 * --------------------------------------------------------------------------
 * Removes all entries from queue but does not deallocate it.
 * Returns the queue on success or NULL if queue is NULL.
 * ----------------------------------------------------------------------- */

m2c_fifo_t m2c_fifo_reset_queue (m2c_fifo_t queue);


/* --------------------------------------------------------------------------
 * function m2c_fifo_release_queue(queue)
 * --------------------------------------------------------------------------
 * Deallocates queue. 
 * ----------------------------------------------------------------------- */

void m2c_fifo_release_queue (m2c_fifo_t queue);

#endif /* M2C_FIFO_H */

/* END OF FILE */