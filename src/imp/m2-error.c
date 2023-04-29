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
 * m2-error.c
 *
 * M2C error type implementation.
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

#include "m2-error.h"

#include <stdio.h>
#include <stddef.h>


/* --------------------------------------------------------------------------
 * array m2c_error_text_array
 * --------------------------------------------------------------------------
 * Human readable text for error codes.
 * --------------------------------------------------------------------------
 */

static const char *m2c_error_text_array[] = {
  /* Null Error */
  
  "UNKNOWN",
  
  /* Option Errors */

  "invalid option\0",
  "invalid argument\0",
  "missing filename\0",
  "invalid filename\0",
  "file not found\0",
  "invalid filename, suffix must be .def, .DEF, .mod or .MOD\0",
    
  /* Lexical Warnings and Errors */

  "disabled code section\0",
  "invalid character\0",
  "premature end-of-file within block comment\0",
  "new line within string literal\0",
  "premature end-of-file within string literal\0",
  "invalid escape sequence\0",
  "premature end-of-file within pragma\0",
  "missing string delimiter at end of string literal\0",
  "missing suffix H at end of base-16 integer literal\0",
  "missing exponent after E at end of real number literal\0",
  
  /* Syntax Warnings and Errors */
  
  "unexpected symbol\0",
  "duplicate identifier\0",
  "semicolon at end of field list sequence\0",
  "empty field list sequence\0",
  "semicolon at end of formal parameter list\0",
  "semicolon at end of statement sequence\0",
  "empty statement sequence\0",
  "Y\0",
  
  /* Semantic Errors */
  
  "Z\0",
  
  /* out-of-range guard */
  
  "\0"

}; /* end m2c_error_text_array */


/* --------------------------------------------------------------------------
 * function m2c_is_valid_error_code(error)
 * --------------------------------------------------------------------------
 * Returns TRUE if error represents a valid error code, otherwise FALSE.
 * --------------------------------------------------------------------------
 */

inline bool m2c_is_valid_error_code (m2c_error_t error) {
  return ((error > ERROR_UNKNOWN) && (error < ERROR_END_MARK));
} /* end m2c_is_valid_error_code */


/* --------------------------------------------------------------------------
 * function m2c_is_lexical_error(error)
 * --------------------------------------------------------------------------
 * Returns TRUE if error represents a lexical error code, otherwise FALSE.
 * --------------------------------------------------------------------------
 */

inline bool m2c_is_lexical_error (m2c_error_t error) {
  return ((error >= FIRST_LEXICAL_ERROR_CODE) &&
          (error <= LAST_LEXICAL_ERROR_CODE));
} /* end m2c_is_lexical_error */


/* --------------------------------------------------------------------------
 * function m2c_is_syntax_error(error)
 * --------------------------------------------------------------------------
 * Returns TRUE if error represents a syntax error code, otherwise FALSE.
 * --------------------------------------------------------------------------
 */

inline bool m2c_is_syntax_error (m2c_error_t error) {
  return ((error >= FIRST_SYNTAX_ERROR_CODE) &&
          (error <= LAST_SYNTAX_ERROR_CODE));
} /* end m2c_is_syntax_error */


/* --------------------------------------------------------------------------
 * function m2c_is_semantic_error(error)
 * --------------------------------------------------------------------------
 * Returns TRUE if error represents a semantic error code, otherwise FALSE.
 * --------------------------------------------------------------------------
 */

inline bool m2c_is_semantic_error (m2c_error_t error) {
  return ((error >= FIRST_SEMANTIC_ERROR_CODE) &&
          (error <= LAST_SEMANTIC_ERROR_CODE));
} /* end m2c_is_semantic_error */


/* --------------------------------------------------------------------------
 * function m2c_error_text(token)
 * --------------------------------------------------------------------------
 * Returns a pointer to an immutable human readable error string for the
 * given error code or NULL if error is not a valid error code.
 * --------------------------------------------------------------------------
 */

const char *m2c_error_text (m2c_error_t error) {
  if (error < ERROR_END_MARK) {
    return m2c_error_text_array[error];
  }
  else /* invalid error code */ {
    return NULL;
  } /* end if */
} /* end m2c_error_text */


/* --------------------------------------------------------------------------
 * procedure m2c_emit_error(error)
 * --------------------------------------------------------------------------
 * Emits an error message for error code error to the console.
 * --------------------------------------------------------------------------
 */

void m2c_emit_error (m2c_error_t error) {
  if (error < ERROR_END_MARK) {
      printf("%s\n", m2c_error_text_array[error]);
  } /* end if */
} /* end m2c_emit_error */


/* --------------------------------------------------------------------------
 * procedure m2c_emit_error_w_str(error, offending_str)
 * --------------------------------------------------------------------------
 * Emits an error message for error code error to the console.
 * --------------------------------------------------------------------------
 */

void m2c_emit_error_w_str (m2c_error_t error, const char *offending_str) {
  if (error < ERROR_END_MARK) {
      printf("%s: %s\n", m2c_error_text_array[error], offending_str);
  } /* end if */
} /* end m2c_emit_error_w_str */


/* --------------------------------------------------------------------------
 * procedure m2c_emit_error_w_pos(error, line, column)
 * --------------------------------------------------------------------------
 * Emits an error message for error code error to the console.
 * --------------------------------------------------------------------------
 */

void m2c_emit_error_w_pos
  (m2c_error_t error, uint_t line, uint_t column) {
  if (error < ERROR_END_MARK) {
    printf("line %u, column %u, error: %s\n",
      line, column, m2c_error_text_array[error]);
  } /* end if */
} /* end m2c_emit_error_w_pos */


/* --------------------------------------------------------------------------
 * procedure m2c_emit_error_w_chr(error, line, column, offending_chr)
 * --------------------------------------------------------------------------
 * Emits an error message for error code error to the console.
 * --------------------------------------------------------------------------
 */

#define IS_PRINTABLE(_ch) \
  ((_ch >= 32) && (_ch <= 126))

void m2c_emit_error_w_chr
  (m2c_error_t error, uint_t line, uint_t column, char offending_chr) {
  if (error < ERROR_END_MARK) {
    printf("line: %u, column: %u, %s",
      line, column, m2c_error_text_array[error]);
    if (IS_PRINTABLE(offending_chr)) {
      printf(", offending character: '%c'\n", offending_chr);
    }
    else /* non-printable */ {
      printf(", offending character code: 0u%X\n", offending_chr);
    } /* end if */
  } /* end if */
} /* end m2c_emit_error_w_chr */


/* --------------------------------------------------------------------------
 * procedure m2c_emit_error_w_lex(error, line, column, offending_lex)
 * --------------------------------------------------------------------------
 * Emits an error message for error code error to the console.
 * --------------------------------------------------------------------------
 */

void m2c_emit_error_w_lex
  (m2c_error_t error,
   uint_t line, uint_t column, const char *offending_lex) {
  if (error < ERROR_END_MARK) {
    printf("line: %u, column: %u, %s",
      line, column, m2c_error_text_array[error]);
    if (offending_lex != NULL) {
      printf(", offending lexeme: %s\n", offending_lex);
    }
    else {
      printf(", offending lexeme: (null)\n");
    } /* end if */
  } /* end if */
} /* end m2c_emit_error_w_lex */


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
   m2c_token_t expected_token) {
  
  /* print line and column */
  printf("line %u, column %u, error: unexpected ", line, column);
  
  /* print offending symbol and its lexeme */
  if (offending_sym == TOKEN_IDENTIFIER) {
    printf("identifier '%s'", offending_lex);
  }
  else if (m2c_is_literal_token(offending_sym)) {
    printf("literal <<%s>>", offending_lex);
  }
  else if (m2c_is_resword_token(offending_sym)) {
    printf("reserved word %s", m2c_lexeme_for_resword(offending_sym));
  }
  else if (m2c_is_special_symbol_token(offending_sym)) {
    printf("symbol '%s'", m2c_lexeme_for_special_symbol(offending_sym));
  }
  else if (offending_sym == TOKEN_END_OF_FILE) {
    printf("end of file");
  }
  else {
    printf("unknown token");
  } /* end if */

  printf(" found\n");
  
  /* print name of expected token */
  printf("  expected ");
  
  if (expected_token == TOKEN_IDENTIFIER) {
    printf("identifier");
  }
  else if (m2c_is_literal_token(expected_token)) {
    printf("integer, real number, character code or string literal");
  }
  else if (m2c_is_resword_token(expected_token)) {
    printf("reserved word %s", m2c_lexeme_for_resword(expected_token));
  }
  else if (m2c_is_special_symbol_token(expected_token)) {
    printf("symbol '%s'", m2c_lexeme_for_special_symbol(expected_token));
  }
  else if (expected_token == TOKEN_END_OF_FILE) {
    printf("end of file");
  } /* end if */
  
  printf("\n");
  
} /* end m2c_emit_syntax_error_w_token */


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
   m2c_tokenset_t expected_set) {
  
  /* print line and column */
  printf("line %u, column %u, error: unexpected ", line, column);
  
  /* print offending symbol and its lexeme */
  if (offending_sym == TOKEN_IDENTIFIER) {
    printf("identifier '%s'", offending_lex);
  }
  else if (m2c_is_literal_token(offending_sym)) {
    printf("literal <<%s>>", offending_lex);
  }
  else if (m2c_is_resword_token(offending_sym)) {
    printf("reserved word %s", m2c_lexeme_for_resword(offending_sym));
  }
  else if (m2c_is_special_symbol_token(offending_sym)) {
    printf("symbol '%s'", m2c_lexeme_for_special_symbol(offending_sym));
  }
  else if (offending_sym == TOKEN_END_OF_FILE) {
    printf("end of file");
  }
  else {
    printf("unknown token");
  } /* end if */

  printf(" found\n");
  
  /* print list of expected symbols */
  printf("  expected ");
  m2c_tokenset_print_list(expected_set);
  
} /* end m2c_emit_syntax_error_w_set */


/* --------------------------------------------------------------------------
 * procedure m2c_emit_warning_w_pos(error, line, column)
 * --------------------------------------------------------------------------
 * Emits a warning message for error code error to the console.
 * ----------------------------------------------------------------------- */

void m2c_emit_warning_w_pos
  (m2c_error_t error, uint_t line, uint_t column) {
  if (error < ERROR_END_MARK) {
    printf("line %u, column %u, warning: %s\n",
      line, column, m2c_error_text_array[error]);
  } /* end if */
} /* end m2c_emit_error_w_pos */


/* --------------------------------------------------------------------------
 * procedure m2c_emit_warning_w_range(error, first_line, last_line)
 * --------------------------------------------------------------------------
 * Emits a warning message for range from first_line to last_line.
 * ----------------------------------------------------------------------- */

void m2c_emit_warning_w_range
  (m2c_error_t error, uint_t first_line, uint_t last_line) {
  if (error < ERROR_END_MARK) {
    printf("line %u to line %u, warning: %s\n",
      first_line, last_line, m2c_error_text_array[error]);
  } /* end if */
} /* end m2c_emit_warning_w_range */

/* END OF FILE */