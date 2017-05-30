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
 * m2-filereader.h
 *
 * Public interface for M2C source file reader module.
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

#ifndef M2C_FILEREADER_H
#define M2C_FILEREADER_H

#include "m2-unique-string.h"

#include <stdbool.h>


/* --------------------------------------------------------------------------
 * File size, line and column counter limits
 * --------------------------------------------------------------------------
 */

#define M2C_INFILE_MAX_SIZE 260000

#define M2C_INFILE_MAX_LINES 64000

#define M2C_INFILE_MAX_COLUMNS 200


/* --------------------------------------------------------------------------
 * opaque type m2c_infile_t
 * --------------------------------------------------------------------------
 * Opaque pointer type representing a Modula-2 source file.
 * --------------------------------------------------------------------------
 */

typedef struct m2c_infile_struct_t *m2c_infile_t;


/* --------------------------------------------------------------------------
 * type m2c_infile_status_t
 * --------------------------------------------------------------------------
 * Status codes for operations on type m2c_infile_t.
 * --------------------------------------------------------------------------
 */

typedef enum {
  M2C_INFILE_STATUS_SUCCESS,
  M2C_INFILE_STATUS_INVALID_REFERENCE,
  M2C_INFILE_STATUS_FILE_NOT_FOUND,
  M2C_INFILE_STATUS_FILE_ACCESS_DENIED,
  M2C_INFILE_STATUS_ALLOCATION_FAILED,
  M2C_INFILE_STATUS_FILE_EMPTY,
  M2C_INFILE_STATUS_ATTEMPT_TO_READ_PAST_EOF,
  M2C_INFILE_STATUS_IO_SUBSYSTEM_ERROR
} m2c_infile_status_t;


/* --------------------------------------------------------------------------
 * procedure m2c_open_infile(infile, filename, status)
 * --------------------------------------------------------------------------
 * Allocates a new object of type m2c_infile_t, opens an input file and
 * associates the opened file with the newly created infile object.
 *
 * pre-conditions:
 * o  parameter file must be NULL upon entry
 * o  parameter status may be NULL
 *
 * post-conditions:
 * o  pointer to newly allocated and opened file is passed back in infile
 * o  line and column counters of the newly allocated infile are set to 1
 * o  M2C_INFILE_STATUS_SUCCESS is passed back in status, unless NULL
 *
 * error-conditions:
 * o  if infile is not NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * o  if the file represented by filename cannot be found
 *    status M2C_INFILE_STATUS_FILE_NOT_FOUND is returned
 * o  if the file represented by filename cannot be accessed
 *    status M2C_INFILE_STATUS_FILE_ACCESS_DENIED is returned
 * o  if no infile object could be allocated
 *    status M2C_INFILE_STATUS_ALLOCATION_FAILED is returned
 * --------------------------------------------------------------------------
 */

m2c_infile_t m2c_open_infile
  (m2c_string_t filename, m2c_infile_status_t *status);


/* --------------------------------------------------------------------------
 * function m2c_read_char(infile)
 * --------------------------------------------------------------------------
 * Reads the lookahead character from infile, advancing the current reading
 * position, updating line and column counter and returns its character code.
 * Returns EOF if the lookahead character lies beyond the end of infile.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  character code of lookahead character or EOF is returned
 * o  current reading position and line and column counters are updated
 * o  file status is set to M2C_INFILE_STATUC_SUCCESS
 *
 * error-conditions:
 * o  if infile is NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * --------------------------------------------------------------------------
 */

int m2c_read_char (m2c_infile_t infile);


/* --------------------------------------------------------------------------
 * function m2c_mark_lexeme(infile)
 * --------------------------------------------------------------------------
 * Marks the current lookahead character as the start of a lexeme.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  position of lookahead character is stored internally
 *
 * error-conditions:
 * o  if infile is NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * --------------------------------------------------------------------------
 */

void m2c_mark_lexeme (m2c_infile_t infile);


/* --------------------------------------------------------------------------
 * function m2c_read_marked_lexeme(infile)
 * --------------------------------------------------------------------------
 * Returns a string object with the character sequence starting with the
 * character that has been marked using procedure m2c_mark_lexeme() and
 * ending with the last consumed character.  Returns NULL if no marker
 * has been set or if the marked character has not been consumed.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  marked position is cleared
 * o  dynamic string with lexeme is returned
 *
 * error-conditions:
 * o  if infile is NULL upon entry,
 *    no operation is carried out and NULL is returned
 * --------------------------------------------------------------------------
 */

m2c_string_t m2c_read_marked_lexeme (m2c_infile_t infile);


/* --------------------------------------------------------------------------
 * function m2c_infile_source_for_line(infile, line)
 * --------------------------------------------------------------------------
 * Returns a string object with the source of the given line number.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 * o  parameter line must not be zero upon entry
 *
 * post-conditions:
 * o  dynamic string with source of line is returned
 *
 * error-conditions:
 * o  if infile is NULL, or line is zero upon entry,
 *    no operation is carried out and NULL is returned
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_infile_source_for_line (m2c_infile_t infile, uint_t line);


/* --------------------------------------------------------------------------
 * function m2c_consume_char(infile)
 * --------------------------------------------------------------------------
 * Consumes the current lookahead character, advancing the current reading
 * position, updating line and column counter and returns the character code
 * of the new lookahead character that follows the consumed character.
 * Returns EOF if the lookahead character lies beyond the end of infile.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  character code of lookahead character or EOF is returned
 * o  current reading position and line and column counters are updated
 * o  file status is set to M2C_INFILE_STATUC_SUCCESS
 *
 * error-conditions:
 * o  if infile is NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * --------------------------------------------------------------------------
 */

int m2c_consume_char (m2c_infile_t infile);


/* --------------------------------------------------------------------------
 * function m2c_next_char(infile)
 * --------------------------------------------------------------------------
 * Reads the lookahead character from infile without advancing the current
 * reading position and returns its character code.  Returns EOF if the
 * lookahead character lies beyond the end of infile.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  character code of lookahead character or EOF is returned
 * o  current reading position and line and column counters are NOT updated
 * o  file status is set to M2C_INFILE_STATUC_SUCCESS
 *
 * error-conditions:
 * o  if infile is NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * --------------------------------------------------------------------------
 */

int m2c_next_char (m2c_infile_t infile);


/* --------------------------------------------------------------------------
 * function m2c_la2_char(infile)
 * --------------------------------------------------------------------------
 * Reads the second lookahead character from infile without advancing the
 * current reading position and returns its character code.  Returns EOF
 * if the second lookahead character lies beyond the end of infile.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  character code of second lookahead character or EOF is returned
 * o  current reading position and line and column counters are NOT updated
 * o  file status is set to M2C_INFILE_STATUC_SUCCESS
 *
 * error-conditions:
 * o  if infile is NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * --------------------------------------------------------------------------
 */

int m2c_la2_char (m2c_infile_t infile);


/* --------------------------------------------------------------------------
 * function m2c_infile_filename(infile)
 * --------------------------------------------------------------------------
 * Returns the filename associated with infile.
 * --------------------------------------------------------------------------
 */

m2c_string_t m2c_infile_filename (m2c_infile_t infile);


/* --------------------------------------------------------------------------
 * function m2c_infile_status(infile)
 * --------------------------------------------------------------------------
 * Returns the status of the last operation on file.
 * --------------------------------------------------------------------------
 */

m2c_infile_status_t m2c_infile_status (m2c_infile_t infile);


/* --------------------------------------------------------------------------
 * function m2c_infile_eof(infile)
 * --------------------------------------------------------------------------
 * Returns true if the current reading position of infile lies beyond the end
 * of the associated file, returns false otherwise.
 * --------------------------------------------------------------------------
 */

bool m2c_infile_eof (m2c_infile_t infile);


/* --------------------------------------------------------------------------
 * function m2c_infile_current_line(infile)
 * --------------------------------------------------------------------------
 * Returns the current line counter of infile.
 * --------------------------------------------------------------------------
 */

unsigned int m2c_infile_current_line (m2c_infile_t infile);


/* --------------------------------------------------------------------------
 * function m2c_infile_current_column(infile)
 * --------------------------------------------------------------------------
 * Returns the current column counter of infile.
 * --------------------------------------------------------------------------
 */

unsigned int m2c_infile_current_column (m2c_infile_t infile);


/* --------------------------------------------------------------------------
 * procedure m2c_close_infile(file, status)
 * --------------------------------------------------------------------------
 * Closes the file associated with handle file, deallocates its file object
 * and returns NULL in handle file.
 *
 * pre-conditions:
 * o  parameter file must not be NULL upon entry
 * o  parameter status may be NULL
 *
 * post-conditions:
 * o  file object is deallocated
 * o  NULL is passed back in file
 * o  M2C_INFILE_STATUS_SUCCESS is passed back in status, unless NULL
 *
 * error-conditions:
 * o  if file is NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * -------------------------------------------------------------------------- 
 */

void m2c_close_infile (m2c_infile_t *infptr, m2c_infile_status_t *status);


#endif /* M2C_FILEREADER_H */

/* END OF FILE */