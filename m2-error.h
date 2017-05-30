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
 * m2-error.h
 *
 * Public interface for M2C error type.
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

#ifndef M2C_ERROR_H
#define M2C_ERROR_H

#include "m2-common.h"
#include "m2-tokenset.h"

#include <stdbool.h>


/* --------------------------------------------------------------------------
 * type m2c_error_t
 * --------------------------------------------------------------------------
 * Enumerated error values representing compilation errors.
 * ----------------------------------------------------------------------- */

typedef enum {
  /* Null Error */
  
  ERROR_UNKNOWN,
  
  /* Option Errors */
  
  M2C_ERROR_INVALID_OPTION,
  M2C_ERROR_INVALID_ARGUMENT,
  M2C_ERROR_MISSING_FILENAME,
  M2C_ERROR_INVALID_FILENAME,
  M2C_ERROR_INPUT_FILE_NOT_FOUND,
  M2C_ERROR_INVALID_FILENAME_SUFFIX,
  
  /* Lexical Warnings and Errors */
  
  M2C_WARN_DISABLED_CODE_SECTION,
  M2C_ERROR_INVALID_INPUT_CHAR,
  M2C_ERROR_EOF_IN_BLOCK_COMMENT,
  M2C_ERROR_NEW_LINE_IN_STRING_LITERAL,
  M2C_ERROR_EOF_IN_STRING_LITERAL,
  M2C_ERROR_INVALID_ESCAPE_SEQUENCE,
  M2C_ERROR_EOF_IN_PRAGMA,
  M2C_ERROR_MISSING_STRING_DELIMITER,
  M2C_ERROR_MISSING_SUFFIX,
  M2C_ERROR_MISSING_EXPONENT,
  
  /* Syntax Warnings and Errors */
  
  M2C_ERROR_UNEXPECTED_TOKEN,
  M2C_ERROR_DUPLICATE_IDENT_IN_IDENT_LIST,
  M2C_SEMICOLON_AFTER_FIELD_LIST_SEQ,
  M2C_EMPTY_FIELD_LIST_SEQ,
  M2C_SEMICOLON_AFTER_FORMAL_PARAM_LIST,
  M2C_SEMICOLON_AFTER_STMT_SEQ,
  M2C_EMPTY_STMT_SEQ,
  ERROR_Y,              /* Y */
  
  /* Semantic Errors */
  
  ERROR_Z,              /* Z */
  
  /* Enumeration Terminator */
  
  ERROR_END_MARK /* marks the end of this enumeration */
} m2c_error_t;


/* --------------------------------------------------------------------------
 * first and last option error codes
 * ----------------------------------------------------------------------- */

#define FIRST_OPTION_ERROR_CODE M2C_ERROR_INVALID_OPTION
#define LAST_OPTION_ERROR_CODE M2C_ERROR_INVALID_FILENAME_SUFFIX


/* --------------------------------------------------------------------------
 * first and last lexical error codes
 * ----------------------------------------------------------------------- */

#define FIRST_LEXICAL_ERROR_CODE M2C_ERROR_INVALID_INPUT_CHAR
#define LAST_LEXICAL_ERROR_CODE M2C_ERROR_MISSING_EXPONENT


/* --------------------------------------------------------------------------
 * first and last syntax error codes
 * ----------------------------------------------------------------------- */

#define FIRST_SYNTAX_ERROR_CODE M2C_ERROR_UNEXPECTED_TOKEN
#define LAST_SYNTAX_ERROR_CODE M2C_EMPTY_STMT_SEQ


/* --------------------------------------------------------------------------
 * first and last semantic error codes
 * ----------------------------------------------------------------------- */

#define FIRST_SEMANTIC_ERROR_CODE ERROR_Z
#define LAST_SEMANTIC_ERROR_CODE ERROR_Z


/* --------------------------------------------------------------------------
 * function m2c_is_valid_error_code(error)
 * --------------------------------------------------------------------------
 * Returns TRUE if error represents a valid error code, otherwise FALSE.
 * ----------------------------------------------------------------------- */

bool m2c_is_valid_error_code (m2c_error_t error);


/* --------------------------------------------------------------------------
 * function m2c_is_option_error(error)
 * --------------------------------------------------------------------------
 * Returns TRUE if error represents an option error code, otherwise FALSE.
 * ----------------------------------------------------------------------- */

bool m2c_is_option_error (m2c_error_t error);


/* --------------------------------------------------------------------------
 * function m2c_is_lexical_error(error)
 * --------------------------------------------------------------------------
 * Returns TRUE if error represents a lexical error code, otherwise FALSE.
 * ----------------------------------------------------------------------- */

bool m2c_is_lexical_error (m2c_error_t error);


/* --------------------------------------------------------------------------
 * function m2c_is_syntax_error(error)
 * --------------------------------------------------------------------------
 * Returns TRUE if error represents a syntax error code, otherwise FALSE.
 * ----------------------------------------------------------------------- */

bool m2c_is_syntax_error (m2c_error_t error);


/* --------------------------------------------------------------------------
 * function m2c_is_semantic_error(error)
 * --------------------------------------------------------------------------
 * Returns TRUE if error represents a semantic error code, otherwise FALSE.
 * ----------------------------------------------------------------------- */

bool m2c_is_semantic_error (m2c_error_t error);


/* --------------------------------------------------------------------------
 * function m2c_error_text(error)
 * --------------------------------------------------------------------------
 * Returns a pointer to an immutable human readable error string for the
 * given error code or NULL if error is not a valid error code.
 * ----------------------------------------------------------------------- */

const char *m2c_error_text (m2c_error_t error);


/* --------------------------------------------------------------------------
 * procedure m2c_emit_error(error)
 * --------------------------------------------------------------------------
 * Emits an error message for error code error to the console.
 * ----------------------------------------------------------------------- */

void m2c_emit_error (m2c_error_t error);


/* --------------------------------------------------------------------------
 * procedure m2c_emit_error_w_str(error, offending_str)
 * --------------------------------------------------------------------------
 * Emits an error message for error code error to the console.
 * ----------------------------------------------------------------------- */

void m2c_emit_error_w_str (m2c_error_t error, const char *offending_str);


/* --------------------------------------------------------------------------
 * procedure m2c_emit_error_w_pos(error, line, column)
 * --------------------------------------------------------------------------
 * Emits an error message for error code error to the console.
 * ----------------------------------------------------------------------- */

void m2c_emit_error_w_pos
  (m2c_error_t error, uint_t line, uint_t column);


/* --------------------------------------------------------------------------
 * procedure m2c_emit_error_w_chr(error, line, column, offending_chr)
 * --------------------------------------------------------------------------
 * Emits an error message for error code error to the console.
 * ----------------------------------------------------------------------- */

void m2c_emit_error_w_chr
  (m2c_error_t error, uint_t line, uint_t column, char offending_chr);


/* --------------------------------------------------------------------------
 * procedure m2c_emit_error_w_lex(error, line, column, offending_lex)
 * --------------------------------------------------------------------------
 * Emits an error message for error code error to the console.
 * ----------------------------------------------------------------------- */

void m2c_emit_error_w_lex
  (m2c_error_t error, uint_t line, uint_t column, const char *offending_lex);


/* --------------------------------------------------------------------------
 * procedure m2c_emit_syntax_error_w_token(line, col, off_sym, off_lex, tokn)
 * --------------------------------------------------------------------------
 * Emits a syntax error message of the following format to the console:
 * line: n, column: m, unexpected offending-symbol offending-lexeme found
 *   expected token
 * ----------------------------------------------------------------------- */

void m2c_emit_syntax_error_w_token
  (uint_t line, uint_t column,
   m2c_token_t offending_sym,
   const char *offending_lex,
   m2c_token_t expected_token);


/* --------------------------------------------------------------------------
 * procedure m2c_emit_syntax_error_w_set(line, col, off_sym, off_lex, set)
 * --------------------------------------------------------------------------
 * Emits a syntax error message of the following format to the console:
 * line: n, column: m, unexpected offending-symbol offending-lexeme found
 *   expected set-symbol-1, set-symbol-2, set-symbol-3, ... or set-symbol-N
 * ----------------------------------------------------------------------- */

void m2c_emit_syntax_error_w_set
  (uint_t line, uint_t column,
   m2c_token_t offending_sym,
   const char *offending_lex,
   m2c_tokenset_t expected_set);


/* --------------------------------------------------------------------------
 * procedure m2c_emit_warning_w_pos(error, line, column)
 * --------------------------------------------------------------------------
 * Emits a warning message for error code error to the console.
 * ----------------------------------------------------------------------- */

void m2c_emit_warning_w_pos
  (m2c_error_t error, uint_t line, uint_t column);


/* --------------------------------------------------------------------------
 * procedure m2c_emit_warning_w_range(error, first_line, last_line)
 * --------------------------------------------------------------------------
 * Emits a warning message for range from first_line to last_line.
 * ----------------------------------------------------------------------- */

void m2c_emit_warning_w_range
  (m2c_error_t error, uint_t first_line, uint_t last_line);

#endif /* M2C_ERROR_H */

/* END OF FILE */