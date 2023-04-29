/* M2T -- Sorce to Source Modula-2 Translator
 *
 * Copyright (c) 2016-2023 Benjamin Kowarsch
 *
 * Author & Maintainer: Benjamin Kowarsch <org.m2sf>
 *
 * @synopsis
 *
 * M2T is a multi-dialect Modula-2 source-to-source translator. It translates
 * source files  written in the  classic dialects  to semantically equivalent
 * source files in  Modula-2 Revision 2010 (M2R10).  It supports  the classic
 * Modula-2 dialects  described in  the 2nd, 3rd and 4th editions  of Niklaus
 * Wirth's book "Programming in Modula-2" (PIM) published by Springer Verlag.
 *
 * For more details please visit: https://github.com/trijezdci/m2t/wiki
 *
 * @repository
 *
 * https://github.com/trijezdci/m2t
 *
 * @file *
 *
 * m2t-lexer.h
 *
 * Public interface for M2T lexer module.
 *
 * @license
 *
 * M2T is free software: you can redistribute and/or modify it under the
 * terms of the GNU Lesser General Public License (LGPL) either version 2.1
 * or at your choice version 3 as published by the Free Software Foundation.
 *
 * M2T is distributed in the hope that it will be useful,  but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  Read the license for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with m2t.  If not, see <https://www.gnu.org/copyleft/lesser.html>.
 */

#ifndef M2T_LEXER_H
#define M2T_LEXER_H

#include "m2t-token.h"
#include "m2t-common.h"
#include "m2t-unique-string.h"


/* --------------------------------------------------------------------------
 * opaque type m2t_lexer_t
 * --------------------------------------------------------------------------
 * Opaque pointer type representing a Modula-2 lexer object.
 * ----------------------------------------------------------------------- */

typedef struct m2t_lexer_struct_t *m2t_lexer_t;


/* --------------------------------------------------------------------------
 * type m2t_lexer_status_t
 * --------------------------------------------------------------------------
 * Status codes for operations on type m2t_lexer_t.
 * ----------------------------------------------------------------------- */

typedef enum {
  M2T_LEXER_STATUS_SUCCESS,
  M2T_LEXER_STATUS_INVALID_REFERENCE,
  M2T_LEXER_STATUS_ALLOCATION_FAILED,
  /* TO DO : add status codes for all error scenarios */
} m2t_lexer_status_t;


/* --------------------------------------------------------------------------
 * procedure m2t_new_lexer(lexer, filename, status)
 * --------------------------------------------------------------------------
 * Allocates a new object of type m2t_lexer_t, opens an input file and
 * associates the opened file with the newly created lexer object.
 *
 * pre-conditions:
 * o  parameter lexer must be NULL upon entry
 * o  parameter status may be NULL
 *
 * post-conditions:
 * o  pointer to newly allocated and opened lexer is passed back in lexer
 * o  M2T_INFILE_STATUS_SUCCESS is passed back in status, unless NULL
 *
 * error-conditions:
 * o  if lexer is not NULL upon entry, no operation is carried out
 *    and status M2T_LEXER_STATUS_INVALID_REFERENCE is returned
 * o  if the file represented by filename cannot be found
 *    status M2T_LEXER_STATUS_FILE_NOT_FOUND is returned
 * o  if the file represented by filename cannot be accessed
 *    status M2T_LEXER_STATUS_FILE_ACCESS_DENIED is returned
 * o  if no infile object could be allocated
 *    status M2T_LEXER_STATUS_ALLOCATION_FAILED is returned
 * ----------------------------------------------------------------------- */

void m2t_new_lexer
  (m2t_lexer_t *lexer, m2t_string_t filename, m2t_lexer_status_t *status);


/* --------------------------------------------------------------------------
 * function m2t_read_sym(lexer)
 * --------------------------------------------------------------------------
 * Reads the lookahead symbol from the source file associated with lexer and
 * consumes it, thus advancing the current reading position, then returns
 * the symbol's token.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  character code of lookahead character or EOF is returned
 * o  current reading position and line and column counters are updated
 * o  file status is set to M2T_INFILE_STATUS_SUCCESS
 *
 * error-conditions:
 * o  if infile is NULL upon entry, no operation is carried out
 *    and status M2T_INFILE_STATUS_INVALID_REFERENCE is returned
 * ----------------------------------------------------------------------- */

m2t_token_t m2t_read_sym (m2t_lexer_t lexer);


/* --------------------------------------------------------------------------
 * function m2t_next_sym(lexer)
 * --------------------------------------------------------------------------
 * Reads the lookahead symbol from the source file associated with lexer but
 * does not consume it, thus not advancing the current reading position,
 * then returns the symbol's token.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  character code of lookahead character or EOF is returned
 * o  current reading position and line and column counters are NOT updated
 * o  file status is set to M2T_LEXER_STATUS_SUCCESS
 *
 * error-conditions:
 * o  if infile is NULL upon entry, no operation is carried out
 *    and status M2T_LEXER_STATUS_INVALID_REFERENCE is returned
 * ----------------------------------------------------------------------- */

m2t_token_t m2t_next_sym (m2t_lexer_t lexer);


/* --------------------------------------------------------------------------
 * function m2t_consume_sym(lexer)
 * --------------------------------------------------------------------------
 * Consumes the lookahead symbol and returns the new lookahead symbol.
 * ----------------------------------------------------------------------- */

m2t_token_t m2t_consume_sym (m2t_lexer_t lexer);


/* --------------------------------------------------------------------------
 * function m2t_lexer_filename(lexer)
 * --------------------------------------------------------------------------
 * Returns the filename associated with lexer.
 * ----------------------------------------------------------------------- */

m2t_string_t m2t_lexer_filename (m2t_lexer_t lexer);


/* --------------------------------------------------------------------------
 * function m2t_lexer_status(lexer)
 * --------------------------------------------------------------------------
 * Returns the status of the last operation on lexer.
 * ----------------------------------------------------------------------- */

m2t_lexer_status_t m2t_lexer_status (m2t_lexer_t lexer);


/* --------------------------------------------------------------------------
 * function m2t_lexer_lookahead_lexeme(lexer)
 * --------------------------------------------------------------------------
 * Returns the lexeme of the lookahead symbol.
 * ----------------------------------------------------------------------- */

m2t_string_t m2t_lexer_lookahead_lexeme (m2t_lexer_t lexer);


/* --------------------------------------------------------------------------
 * function m2t_lexer_current_lexeme(lexer)
 * --------------------------------------------------------------------------
 * Returns the lexeme of the most recently consumed symbol.
 * ----------------------------------------------------------------------- */

m2t_string_t m2t_lexer_current_lexeme (m2t_lexer_t lexer);


/* --------------------------------------------------------------------------
 * function m2t_lexer_lookahead_line(lexer)
 * --------------------------------------------------------------------------
 * Returns the line counter of the lookahead symbol.
 * ----------------------------------------------------------------------- */

uint_t m2t_lexer_lookahead_line (m2t_lexer_t lexer);


/* --------------------------------------------------------------------------
 * function m2t_lexer_current_line(lexer)
 * --------------------------------------------------------------------------
 * Returns the line counter of the most recently consumed symbol.
 * ----------------------------------------------------------------------- */

uint_t m2t_lexer_current_line (m2t_lexer_t lexer);


/* --------------------------------------------------------------------------
 * function m2t_lexer_lookahead_column(lexer)
 * --------------------------------------------------------------------------
 * Returns the column counter of the lookahead symbol.
 * ----------------------------------------------------------------------- */

uint_t m2t_lexer_lookahead_column (m2t_lexer_t lexer);


/* --------------------------------------------------------------------------
 * function m2t_lexer_current_column(lexer)
 * --------------------------------------------------------------------------
 * Returns the column counter of the most recently consumed symbol.
 * ----------------------------------------------------------------------- */

uint_t m2t_lexer_current_column (m2t_lexer_t lexer);


/* --------------------------------------------------------------------------
 * procedure m2t_print_line_and_mark_column(lexer, line, column)
 * --------------------------------------------------------------------------
 * Prints the given source line of the current symbol to the console and
 * marks the given coloumn with a caret '^'.
 * ----------------------------------------------------------------------- */

void m2t_print_line_and_mark_column
  (m2t_lexer_t lexer, uint_t line, uint_t column);


/* --------------------------------------------------------------------------
 * procedure m2t_release_lexer(lexer, status)
 * --------------------------------------------------------------------------
 * Closes the file associated with lexer, deallocates its file object,
 * deallocates the lexer object and returns NULL in lexer.
 *
 * pre-conditions:
 * o  parameter lexer must not be NULL upon entry
 * o  parameter status may be NULL
 *
 * post-conditions:
 * o  file object is deallocated
 * o  NULL is passed back in lexer
 * o  M2T_LEXER_STATUS_SUCCESS is passed back in status, unless NULL
 *
 * error-conditions:
 * o  if lexer is NULL upon entry, no operation is carried out
 *    and status M2T_LEXER_STATUS_INVALID_REFERENCE is returned
 * ----------------------------------------------------------------------- */

void m2t_release_lexer (m2t_lexer_t *lexer, m2t_lexer_status_t *status);


#endif /* M2T_LEXER_H */

/* END OF FILE */