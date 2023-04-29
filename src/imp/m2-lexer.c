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
 * m2-lexer.c
 *
 * Implementation of M2C lexer module.
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

#include "m2-lexer.h"
#include "m2-error.h"
#include "m2-filereader.h"
#include "m2-compiler-options.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>


/* --------------------------------------------------------------------------
 * private type m2c_symbol_struct_t
 * --------------------------------------------------------------------------
 * record type holding symbol details.
 * ----------------------------------------------------------------------- */

typedef struct {
  /* token */ m2c_token_t token;
  /* line */ uint_t line;
  /* column */ uint_t column;
  /* lexeme */ m2c_string_t lexeme;
} m2c_symbol_struct_t;


/* --------------------------------------------------------------------------
 * null symbol for initialisation.
 * ----------------------------------------------------------------------- */

static const m2c_symbol_struct_t null_symbol = {
  /* token */ TOKEN_UNKNOWN,
  /* line */ 0,
  /* column */ 0,
  /* lexeme */ NULL
}; /* null_symbol */


/* --------------------------------------------------------------------------
 * private type m2c_number_literal_lexer_f
 * --------------------------------------------------------------------------
 * function pointer type for function to lex a number literal.
 * ----------------------------------------------------------------------- */

typedef char (*m2c_number_literal_lexer_f)
  (m2c_lexer_t lexer, m2c_token_t *token);


/* --------------------------------------------------------------------------
 * hidden type m2c_lexer_struct_t
 * --------------------------------------------------------------------------
 * record type representing a Modula-2 lexer object.
 * ----------------------------------------------------------------------- */

struct m2c_lexer_struct_t {
  /* infile */ m2c_infile_t infile;
  /* current */ m2c_symbol_struct_t current;
  /* lookahead */ m2c_symbol_struct_t lookahead;
  /* status */ m2c_lexer_status_t status;
  /* error_count */ uint_t error_count;
  /* get_number_literal */ m2c_number_literal_lexer_f get_number_literal;
};

typedef struct m2c_lexer_struct_t m2c_lexer_struct_t;


/* --------------------------------------------------------------------------
 * Forward declarations
 * ----------------------------------------------------------------------- */

static void get_new_lookahead_sym (m2c_lexer_t lexer);

static char skip_code_section (m2c_lexer_t lexer);

static char skip_line_comment (m2c_lexer_t lexer);

static char skip_block_comment (m2c_lexer_t lexer);

static char get_pragma(m2c_lexer_t lexer);

static char get_ident (m2c_lexer_t lexer);

static char get_ident_or_resword (m2c_lexer_t lexer, m2c_token_t *token);

static char get_string_literal (m2c_lexer_t lexer, m2c_token_t *token);

static char get_prefixed_number_literal
  (m2c_lexer_t lexer, m2c_token_t *token);

static char get_suffixed_number_literal
  (m2c_lexer_t lexer, m2c_token_t *token);
  
static char get_number_literal_fractional_part
  (m2c_lexer_t lexer, m2c_token_t *token);


/* --------------------------------------------------------------------------
 * procedure m2c_new_lexer(lexer, filename, status)
 * --------------------------------------------------------------------------
 * Allocates a new object of type m2c_lexer_t, opens an input file and
 * associates the opened file with the newly created lexer object.
 *
 * pre-conditions:
 * o  parameter lexer must be NULL upon entry
 * o  parameter status may be NULL
 *
 * post-conditions:
 * o  pointer to newly allocated and opened lexer is passed back in lexer
 * o  M2C_LEXER_STATUS_SUCCESS is passed back in status, unless NULL
 *
 * error-conditions:
 * o  if lexer is not NULL upon entry, no operation is carried out
 *    and status M2C_LEXER_STATUS_INVALID_REFERENCE is returned
 * o  if the file represented by filename cannot be found
 *    status M2C_LEXER_STATUS_FILE_NOT_FOUND is returned
 * o  if the file represented by filename cannot be accessed
 *    status M2C_LEXER_STATUS_FILE_ACCESS_DENIED is returned
 * o  if no infile object could be allocated
 *    status M2C_LEXER_STATUS_ALLOCATION_FAILED is returned
 * ----------------------------------------------------------------------- */

void m2c_new_lexer
  (m2c_lexer_t *lexer, m2c_string_t filename, m2c_lexer_status_t *status) {
   
   m2c_infile_t infile;
   m2c_lexer_t new_lexer;
   m2c_infile_status_t infile_status;
   
   /* check pre-conditions */
   if ((lexer == NULL) || (filename == NULL)) {
     SET_STATUS(status, M2C_LEXER_STATUS_INVALID_REFERENCE);
     return;
   } /* end if */
   
   new_lexer = malloc(sizeof(m2c_lexer_struct_t));
   
   if (new_lexer == NULL) {
     SET_STATUS(status, M2C_LEXER_STATUS_ALLOCATION_FAILED);
     return;
   } /* end if */
   
   /* open source file */
   infile = m2c_open_infile(filename, &infile_status);
   
   if (infile == NULL) {
     SET_STATUS(status, M2C_LEXER_STATUS_ALLOCATION_FAILED);
     free(new_lexer);
     return;
   } /* end if */
   
   /* initialise lexer object */
   new_lexer->infile = infile;
   new_lexer->current = null_symbol;
   new_lexer->lookahead = null_symbol;
   new_lexer->error_count = 0;
   
   if (m2c_option_prefix_literals()) {
     /* install function to lex prefix number literals */
     new_lexer->get_number_literal = get_prefixed_number_literal;
   }
   else /* suffix literals */ {
     /* install function to lex suffix number literals */
     new_lexer->get_number_literal = get_suffixed_number_literal;
   } /* end if */
   
   /* read first symbol */
   get_new_lookahead_sym(new_lexer);
   
   *lexer = new_lexer;
   return;
} /* end m2c_new_lexer */


/* --------------------------------------------------------------------------
 * function m2c_read_sym(lexer)
 * --------------------------------------------------------------------------
 * Reads the lookahead symbol from the source file associated with lexer and
 * consumes it, thus advancing the current reading position, then returns
 * the symbol's token.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  lookahead symbol's token is returned
 * o  current reading position and line and column counters are updated
 * o  file status is set to M2C_LEXER_STATUS_SUCCESS
 *
 * error-conditions:
 * o  if lexer is NULL upon entry, no operation is carried out
 *    and status M2C_LEXER_STATUS_INVALID_REFERENCE is returned
 * ----------------------------------------------------------------------- */

m2c_token_t m2c_read_sym (m2c_lexer_t lexer) {
  
  /* release the lexeme of the current symbol */
  m2c_string_release(lexer->current.lexeme);
  
  /* lookahead symbol becomes current symbol */
  lexer->current = lexer->lookahead;
  
  /* read new lookahead symbol */
  get_new_lookahead_sym(lexer);
  
  /* return current token */
  return lexer->current.token;
  
} /* end m2c_read_sym */


/* --------------------------------------------------------------------------
 * function m2c_next_sym(lexer)
 * --------------------------------------------------------------------------
 * Returns the lookahead symbol without consuming it.
 * ----------------------------------------------------------------------- */

inline m2c_token_t m2c_next_sym (m2c_lexer_t lexer) {
  
  return lexer->lookahead.token;

} /* end m2c_next_sym */


/* --------------------------------------------------------------------------
 * function m2c_consume_sym(lexer)
 * --------------------------------------------------------------------------
 * Consumes the lookahead symbol and returns the new lookahead symbol.
 * ----------------------------------------------------------------------- */

m2c_token_t m2c_consume_sym (m2c_lexer_t lexer) {
  
  /* release the lexeme of the current symbol */
  m2c_string_release(lexer->current.lexeme);
  
  /* lookahead symbol becomes current symbol */
  lexer->current = lexer->lookahead;
  
  /* read new lookahead symbol and return it */
  get_new_lookahead_sym(lexer);
  return lexer->lookahead.token;
  
} /* end m2c_consume_sym */


/* --------------------------------------------------------------------------
 * function m2c_lexer_filename(lexer)
 * --------------------------------------------------------------------------
 * Returns the filename associated with lexer.
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_lexer_filename (m2c_lexer_t lexer) {
  
  return m2c_infile_filename(lexer->infile);
  
} /* end m2c_lexer_filename */


/* --------------------------------------------------------------------------
 * function m2c_lexer_status(lexer)
 * --------------------------------------------------------------------------
 * Returns the status of the last operation on lexer.
 * ----------------------------------------------------------------------- */

m2c_lexer_status_t m2c_lexer_status (m2c_lexer_t lexer) {
  
  return lexer->status;
  
} /* end m2c_lexer_status */


/* --------------------------------------------------------------------------
 * function m2c_lexer_lookahead_lexeme(lexer)
 * --------------------------------------------------------------------------
 * Returns the lexeme of the lookahead symbol.
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_lexer_lookahead_lexeme (m2c_lexer_t lexer) {
  
  m2c_string_retain(lexer->lookahead.lexeme);
  
  return lexer->lookahead.lexeme;
  
} /* end m2c_lexer_lookahead_lexeme */


/* --------------------------------------------------------------------------
 * function m2c_lexer_current_lexeme(lexer)
 * --------------------------------------------------------------------------
 * Returns the lexeme of the most recently consumed symbol.
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_lexer_current_lexeme (m2c_lexer_t lexer) {
  
  m2c_string_retain(lexer->current.lexeme);
  
  return lexer->current.lexeme;
  
} /* end m2c_lexer_current_lexeme */


/* --------------------------------------------------------------------------
 * function m2c_lexer_lookahead_line(lexer)
 * --------------------------------------------------------------------------
 * Returns the line counter of the lookahead symbol.
 * ----------------------------------------------------------------------- */

uint_t m2c_lexer_lookahead_line (m2c_lexer_t lexer) {
  
  return lexer->lookahead.line;
  
} /* end m2c_lexer_lookahead_line */


/* --------------------------------------------------------------------------
 * function m2c_lexer_current_line(lexer)
 * --------------------------------------------------------------------------
 * Returns the line counter of the most recently consumed symbol.
 * ----------------------------------------------------------------------- */

uint_t m2c_lexer_current_line (m2c_lexer_t lexer) {
  
  return lexer->current.line;
  
} /* end m2c_lexer_current_line */


/* --------------------------------------------------------------------------
 * function m2c_lexer_lookahead_column(lexer)
 * --------------------------------------------------------------------------
 * Returns the column counter of the lookahead symbol.
 * ----------------------------------------------------------------------- */

uint_t m2c_lexer_lookahead_column (m2c_lexer_t lexer) {
  
  return lexer->lookahead.column;
  
} /* end m2c_lexer_lookahead_column */


/* --------------------------------------------------------------------------
 * function m2c_lexer_current_column(lexer)
 * --------------------------------------------------------------------------
 * Returns the column counter of the most recently consumed symbol.
 * ----------------------------------------------------------------------- */

uint_t m2c_lexer_current_column (m2c_lexer_t lexer) {
  
  return lexer->current.column;
  
} /* end m2c_lexer_current_column */


/* --------------------------------------------------------------------------
 * procedure m2c_print_line_and_mark_column(lexer, line, column)
 * --------------------------------------------------------------------------
 * Prints the given source line of the current symbol to the console and
 * marks the given coloumn with a caret '^'.
 * ----------------------------------------------------------------------- */

void m2c_print_line_and_mark_column
  (m2c_lexer_t lexer, uint_t line, uint_t column) {
  
  m2c_string_t source;
  uint_t n;
  
  source = m2c_infile_source_for_line (lexer->infile, line);
  printf("\n%s\n", m2c_string_char_ptr(source));
  
  n = 1;
  while (n < column) {
    printf(" ");
    n++;
  }
  printf("^\n\n");
  
  m2c_string_release(source);
  
  return;
} /* end m2c_print_line_and_mark_column */


/* --------------------------------------------------------------------------
 * procedure m2c_release_lexer(lexer, status)
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
 * o  M2C_LEXER_STATUS_SUCCESS is passed back in status, unless NULL
 *
 * error-conditions:
 * o  if lexer is NULL upon entry, no operation is carried out
 *    and status M2C_LEXER_STATUS_INVALID_REFERENCE is returned
 * ----------------------------------------------------------------------- */

void m2c_release_lexer (m2c_lexer_t *lexptr, m2c_lexer_status_t *status) {
  
  m2c_lexer_t lexer;
  
  /* check pre-conditions */
  if ((lexptr == NULL) || (*lexptr == NULL)) {
    SET_STATUS(status, M2C_LEXER_STATUS_INVALID_REFERENCE);
    return;
  } /* end if */
  
  lexer = *lexptr;
  
  m2c_close_infile(&lexer->infile, NULL);
  m2c_string_release(lexer->current.lexeme);
  m2c_string_release(lexer->lookahead.lexeme);
  
  free(lexer);
  *lexptr = NULL;
  
  return;
} /* end m2c_release_lexer */


/* --------------------------------------------------------------------------
 * procedure report_error_w_offending_pos(error, lexer, line, column)
 * ----------------------------------------------------------------------- */

static void report_error_w_offending_pos
  (m2c_error_t error, m2c_lexer_t lexer, uint_t line, uint_t column) {
    
  m2c_emit_error_w_pos(error, line, column);
  
  if (m2c_option_verbose()) {
    m2c_print_line_and_mark_column(lexer, line, column);
  } /* end if */
  
  lexer->error_count++;
  
  return;
} /* end report_error_w_offending_pos */
  

/* --------------------------------------------------------------------------
 * procedure report_error_w_offending_char(error, lexer, line, column, char)
 * ----------------------------------------------------------------------- */

static void report_error_w_offending_char
  (m2c_error_t error,
   m2c_lexer_t lexer, uint_t line, uint_t column, char offending_char) {
    
  m2c_emit_error_w_chr(error, line, column, offending_char);
  
  if (m2c_option_verbose() && (!IS_CONTROL_CHAR(offending_char))) {
    m2c_print_line_and_mark_column(lexer, line, column);
  } /* end if */
  
  lexer->error_count++;
  
  return;
} /* end report_error_w_offending_char */
  

/* --------------------------------------------------------------------------
 * private procedure get_new_lookahead_sym(lexer)
 * ----------------------------------------------------------------------- */

static void get_new_lookahead_sym (m2c_lexer_t lexer) {
  
  uint_t line, column;
  m2c_token_t token;
  char next_char;
  
  /* no token yet */
  token = TOKEN_UNKNOWN;
  
  /* get the lookahead character */
  next_char = m2c_next_char(lexer->infile);
  
  while (token == TOKEN_UNKNOWN) {
  
    /* skip all whitespace and line feeds */
    while ((next_char == ASCII_SPACE) ||
           (next_char == ASCII_TAB) ||
           (next_char == ASCII_LF)) {
      
      /* consume the character and get new lookahead */
      next_char = m2c_consume_char(lexer->infile);
    } /* end while */
    
    /* get line and column of lookahead */
    line = m2c_infile_current_line(lexer->infile);
    column = m2c_infile_current_column(lexer->infile);
    
    switch (next_char) {
      
      case ASCII_EOT :
        /* End-of-File marker */        
        if (m2c_infile_status(lexer->infile) ==
            M2C_INFILE_STATUS_ATTEMPT_TO_READ_PAST_EOF) {
          token = TOKEN_END_OF_FILE;
        }
        else /* invalid char */ {
          report_error_w_offending_char
            (M2C_ERROR_INVALID_INPUT_CHAR, lexer, line, column, next_char);
          next_char = m2c_consume_char(lexer->infile);
          token = TOKEN_UNKNOWN;
        } /* end if */
        break;
        
      case '!' :
        /* line comment */        
        if (m2c_option_line_comments()) {
          next_char = skip_line_comment(lexer);
        }
        else /* invalid char */ {
          report_error_w_offending_char
            (M2C_ERROR_INVALID_INPUT_CHAR, lexer, line, column, next_char);
          next_char = m2c_consume_char(lexer->infile);
        } /* end if */
        token = TOKEN_UNKNOWN;
        break;
        
      case '\"' :
        /* string literal */
        next_char = get_string_literal(lexer, &token);
        if (token == TOKEN_MALFORMED_STRING) {
          m2c_emit_error_w_pos
            (M2C_ERROR_MISSING_STRING_DELIMITER, line, column);
          lexer->error_count++;
        } /* end if */
        break;
        
      case '#' :
        /* not-equal operator */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_NOTEQUAL;
        break;
        
      case '&' :
        /* ampersand synonym */
        if (m2c_option_synonyms()) {
          next_char = m2c_consume_char(lexer->infile);
          token = TOKEN_AND;
        }
        else /* invalid char */ {
          report_error_w_offending_char
            (M2C_ERROR_INVALID_INPUT_CHAR, lexer, line, column, next_char);
          next_char = m2c_consume_char(lexer->infile);
          token = TOKEN_UNKNOWN;
        } /* end if */
        break;
        
      case '\'' :
        /* string literal */
        next_char = get_string_literal(lexer, &token);
        if (token == TOKEN_MALFORMED_STRING) {
          m2c_emit_error_w_pos
            (M2C_ERROR_MISSING_STRING_DELIMITER, line, column);
          lexer->error_count++;
        } /* end if */
        break;
        
      case '(' :
        /* left parenthesis */
        if (m2c_la2_char(lexer->infile) != '*') {
          next_char = m2c_consume_char(lexer->infile);
          token = TOKEN_LEFT_PARENTHESIS;
        }
        else /* block comment */ {
          next_char = skip_block_comment(lexer);
          token = TOKEN_UNKNOWN;
        } /* end if */
        break;
        
      case ')' :
        /* right parenthesis */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_RIGHT_PARENTHESIS;
        break;
        
      case '*' :
        /* asterisk operator */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_MULTIPLICATION;
        break;
        
      case '+' :
        /* plus operator */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_ADDITION;
        break;
        
      case ',' :
        /* comma */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_COMMA;
        break;
        
      case '-' :
        /* minus operator */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_SUBTRACTION;
        break;
        
      case '.' :
        /* range or period */
        next_char = m2c_consume_char(lexer->infile);
        if /* range */ (next_char == '.') {
          next_char = m2c_consume_char(lexer->infile);
          token = TOKEN_RANGE;
        }
        else /* period */ {
          token = TOKEN_PERIOD;
        } /* end if */
        break;
        
      case '/' :
        /* solidus operator */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_DIVISION;
        break;
        
      case '0' :
      case '1' :
      case '2' :
      case '3' :
      case '4' :
      case '5' :
      case '6' :
      case '7' :
      case '8' :
      case '9' :
        /* number literal */
        next_char = lexer->get_number_literal(lexer, &token);
        if (token == TOKEN_MALFORMED_INTEGER) {
          m2c_emit_error_w_pos(M2C_ERROR_MISSING_SUFFIX, line, column);
          lexer->error_count++;
        }
        else if (token == TOKEN_MALFORMED_REAL) {
          m2c_emit_error_w_pos(M2C_ERROR_MISSING_EXPONENT, line, column);
          lexer->error_count++;
        } /* end if */
        break;
        
      case ':' :
        /* assignment or colon*/
        next_char = m2c_consume_char(lexer->infile);
        if /* assignment */ (next_char == '=') {
          next_char = m2c_consume_char(lexer->infile);
          token = TOKEN_ASSIGNMENT;
        }
        else /* colon */ {
          token = TOKEN_COLON;
        } /* end if */
        break;
        
      case ';' :
        /* semicolon */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_SEMICOLON;
        break;
        
      case '<' :
        /* pragma */
        if (m2c_la2_char(lexer->infile) == '*') {
          next_char = get_pragma(lexer);
          token = TOKEN_PRAGMA;
          break;
        }
        /* not-equal synonym, or less-or-equal or equal operator */
        next_char = m2c_consume_char(lexer->infile);
        
        if /* diamond */ (next_char == '>') {
          if (m2c_option_synonyms()) {
            next_char = m2c_consume_char(lexer->infile);
            token = TOKEN_NOTEQUAL;
          }
          else /* invalid char */ {
            report_error_w_offending_char
              (M2C_ERROR_INVALID_INPUT_CHAR, lexer, line, column, next_char);
            next_char = m2c_consume_char(lexer->infile);
            token = TOKEN_UNKNOWN;
          } /* end if */
        }
        else if /* less-or-equal */ (next_char == '=') {
          next_char = m2c_consume_char(lexer->infile);
          token = TOKEN_LESS_THAN_OR_EQUAL;
        }
        else /* less */ {
          token = TOKEN_LESS_THAN;
        } /* end if */
        break;
        
      case '=' :
        /* equal operator */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_EQUAL;
        break;
        
      case '>' :
        /* greater-or-equal or equal operator */
        next_char = m2c_consume_char(lexer->infile);
        if /* greater-or-equal */ (next_char == '=') {
          next_char = m2c_consume_char(lexer->infile);
          token = TOKEN_GREATER_THAN_OR_EQUAL;
        }
        else /* greater */ {
          token = TOKEN_GREATER_THAN;
        } /* end if */
        break;
        
      case '?' :
        /* disabled code section */
        if ((column == 1) && (m2c_la2_char(lexer->infile) == '<')) {
          next_char = skip_code_section(lexer);
        }
        else /* invalid character */ {
          report_error_w_offending_char
            (M2C_ERROR_INVALID_INPUT_CHAR, lexer, line, column, next_char);
          next_char = m2c_consume_char(lexer->infile);
        } /* end if */
        token = TOKEN_UNKNOWN;
        break;
        
      case 'A' :
      case 'B' :
      case 'C' :
      case 'D' :
      case 'E' :
      case 'F' :
      case 'G' :
      case 'H' :
      case 'I' :
      case 'J' :
      case 'K' :
      case 'L' :
      case 'M' :
      case 'N' :
      case 'O' :
      case 'P' :
      case 'Q' :
      case 'R' :
      case 'S' :
      case 'T' :
      case 'U' :
      case 'V' :
      case 'W' :
      case 'X' :
      case 'Y' :
      case 'Z' :
        /* identifier or reserved word */
        next_char = get_ident_or_resword(lexer, &token);
        break;
        
      case '[' :
        /* left bracket */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_LEFT_BRACKET;
        break;
        
      case ']' :
        /* right bracket */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_RIGHT_BRACKET;
        break;
        
      case '^' :
        /* caret */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_DEREF;
        break;
        
      case 'a' :
      case 'b' :
      case 'c' :
      case 'd' :
      case 'e' :
      case 'f' :
      case 'g' :
      case 'h' :
      case 'i' :
      case 'j' :
      case 'k' :
      case 'l' :
      case 'm' :
      case 'n' :
      case 'o' :
      case 'p' :
      case 'q' :
      case 'r' :
      case 's' :
      case 't' :
      case 'u' :
      case 'v' :
      case 'w' :
      case 'x' :
      case 'y' :
      case 'z' :
        /* identifier */
        next_char = get_ident(lexer);
        token = TOKEN_IDENTIFIER;
        break;
        
      case '{' :
        /* left brace */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_LEFT_BRACE;
        break;
        
      case '|' :
        /* vertical bar */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_BAR;
        break;
        
      case '}' :
        /* right brace */
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_RIGHT_BRACE;
        break;
        
      case '~' :
        /* tilde synonym */
        if (m2c_option_synonyms()) {
          next_char = m2c_consume_char(lexer->infile);
          token = TOKEN_NOT;
        }
        else /* invalid char */ {
          report_error_w_offending_char
            (M2C_ERROR_INVALID_INPUT_CHAR, lexer, line, column, next_char);
          next_char = m2c_consume_char(lexer->infile);
          token = TOKEN_UNKNOWN;
        } /* end if */
        break;
                
      default :
        /* invalid character */
        report_error_w_offending_char
          (M2C_ERROR_INVALID_INPUT_CHAR, lexer, line, column, next_char);
        next_char = m2c_consume_char(lexer->infile);
        token = TOKEN_UNKNOWN;
    } /* end switch */
  } /* end while */
  
  /* update lexer's lookahead symbol */
  lexer->lookahead.token = token;
  lexer->lookahead.line = line;
  lexer->lookahead.column = column;
  
  return;
} /* end get_new_lookahead_sym */


/* --------------------------------------------------------------------------
 * private function skip_code_section(lexer)
 * ----------------------------------------------------------------------- */

#define IS_EOF(_lexer,_ch) \
  (((_ch) == ASCII_EOT) && \
   (m2c_infile_status(_lexer->infile) == \
    M2C_INFILE_STATUS_ATTEMPT_TO_READ_PAST_EOF))

static char skip_code_section (m2c_lexer_t lexer) {
  
  bool delimiter_found = false;
  uint_t first_line;
  char next_char;
  
  /* remember line number for warning */
  first_line = m2c_infile_current_line(lexer->infile);
  
  /* consume opening '?' and '<' */
  next_char = m2c_consume_char(lexer->infile);
  next_char = m2c_consume_char(lexer->infile);
  
  while ((!delimiter_found) && (!IS_EOF(lexer, next_char))) {
    
    /* check for closing delimiter */
    if ((next_char == '>') && (m2c_la2_char(lexer->infile) == '?') &&
       /* first column */ (m2c_infile_current_column(lexer->infile) == 1)) {
      
      /* closing delimiter */
      delimiter_found = true;
        
      /* consume closing '>' and '?' */
      next_char = m2c_consume_char(lexer->infile);
      next_char = m2c_consume_char(lexer->infile);
      break;
    } /* end if */
    
    /* check for illegal control characters */
    if ((IS_CONTROL_CHAR(next_char)) &&
        (next_char != ASCII_TAB) &&
        (next_char != ASCII_LF)) {
      /* invalid input character */
      report_error_w_offending_char
        (M2C_ERROR_INVALID_INPUT_CHAR, lexer,
         m2c_infile_current_line(lexer->infile),
         m2c_infile_current_column(lexer->infile), next_char);
    } /* end if */
    
    next_char = m2c_consume_char(lexer->infile);
  } /* end while */
  
  /* disabled code section warning */
  m2c_emit_warning_w_range
    (M2C_WARN_DISABLED_CODE_SECTION,
     first_line, m2c_infile_current_line(lexer->infile));
  
  return next_char;
} /* end skip_code_section */


/* --------------------------------------------------------------------------
 * private function skip_line_comment(lexer)
 * ----------------------------------------------------------------------- */

static char skip_line_comment(m2c_lexer_t lexer) {
  
  char next_char;

  /* consume opening '!' */
  next_char = m2c_consume_char(lexer->infile);
  
  while ((next_char != ASCII_LF) && (!IS_EOF(lexer, next_char))) {
    
    /* check for illegal characters */
    if (IS_CONTROL_CHAR(next_char) && (next_char != ASCII_TAB)) {
      /* invalid input character */
      report_error_w_offending_char
        (M2C_ERROR_INVALID_INPUT_CHAR, lexer,
         m2c_infile_current_line(lexer->infile),
         m2c_infile_current_column(lexer->infile), next_char);
    } /* end if */
  
    next_char = m2c_consume_char(lexer->infile);
  } /* end while */
  
  return next_char;
} /* end skip_line_comment */


/* --------------------------------------------------------------------------
 * private function skip_block_comment(lexer)
 * ----------------------------------------------------------------------- */

static char skip_block_comment(m2c_lexer_t lexer) {
  
  uint_t line, column, comment_nesting_level = 1;
  char next_char;
  
  /* consume opening '(' and '*' */
  next_char = m2c_consume_char(lexer->infile);
  next_char = m2c_consume_char(lexer->infile);
  
  while (comment_nesting_level > 0) {
    
    /* check for opening block comment */
    if (next_char == '(') {
      next_char = m2c_consume_char(lexer->infile);
      if (next_char == '*') {
        next_char = m2c_consume_char(lexer->infile);
        comment_nesting_level++;
      } /* end if */
    }
    
    /* check for closing block comment */
    else if (next_char == '*') {
      next_char = m2c_consume_char(lexer->infile);
      if (next_char == ')') {
        next_char = m2c_consume_char(lexer->infile);
        comment_nesting_level--;
      } /* end if */
    }
    
    /* other characters permitted within block comments */
    else if ((!IS_CONTROL_CHAR(next_char)) ||
             (next_char == ASCII_TAB) ||
             (next_char == ASCII_LF)) {
      next_char = m2c_consume_char(lexer->infile);
    }
    
    else /* error */ {
      line = m2c_infile_current_line(lexer->infile);
      column = m2c_infile_current_column(lexer->infile);
      
      /* end-of-file reached */
      if (IS_EOF(lexer, next_char)) {
        report_error_w_offending_pos
          (M2C_ERROR_EOF_IN_BLOCK_COMMENT, lexer,  line, column);
      }
      else /* illegal character */ {
        report_error_w_offending_char
          (M2C_ERROR_INVALID_INPUT_CHAR, lexer,  line, column, next_char);
        next_char = m2c_consume_char(lexer->infile);
      } /* end if */
    } /* end if */
  } /* end while */
  
  return next_char;
} /* end skip_block_comment */


/* --------------------------------------------------------------------------
 * private function get_pragma(lexer)
 * ----------------------------------------------------------------------- */

static char get_pragma(m2c_lexer_t lexer) {
  
  bool delimiter_found = false;
  uint_t line, column;
  char next_char;
  
  m2c_mark_lexeme(lexer->infile);
  
  /* consume opening '<' and '*' */
  next_char = m2c_consume_char(lexer->infile);
  next_char = m2c_consume_char(lexer->infile);
    
  while (!delimiter_found) {
    
    if /* closing delimiter */
      ((next_char == '*') && (m2c_la2_char(lexer->infile) == '>')) {
      
      delimiter_found = true;
            
      /* consume closing '*' and '>' */
      next_char = m2c_consume_char(lexer->infile);
      next_char = m2c_consume_char(lexer->infile);
      
      /* get lexeme */
      lexer->lookahead.lexeme = m2c_read_marked_lexeme(lexer->infile);
    }
    
    /* other non-control characters */
    else if (!IS_CONTROL_CHAR(next_char)) {
      next_char = m2c_consume_char(lexer->infile);
    }
    
    else /* error */ {
      line = m2c_infile_current_line(lexer->infile);
      column = m2c_infile_current_column(lexer->infile);
      
      /* end-of-file reached */
      if (IS_EOF(lexer, next_char)) {
        report_error_w_offending_pos
          (M2C_ERROR_EOF_IN_PRAGMA, lexer,  line, column);
      }
      else /* illegal character */ {
        report_error_w_offending_char
          (M2C_ERROR_INVALID_INPUT_CHAR, lexer,  line, column, next_char);
        next_char = m2c_consume_char(lexer->infile);
      } /* end if */
      lexer->error_count++;
    } /* end if */
  } /* end while */
  
  return next_char;
} /* end get_pragma */


/* --------------------------------------------------------------------------
 * private function get_ident(lexer)
 * ----------------------------------------------------------------------- */

static char get_ident(m2c_lexer_t lexer) {
  
  char next_char;
  char next_next_char;
  
  m2c_mark_lexeme(lexer->infile);
  next_char = m2c_consume_char(lexer->infile);
  next_next_char = m2c_la2_char(lexer->infile);
  
  /* lowline enabled */
  if (m2c_option_lowline_identifiers()) {
    while (IS_ALPHANUMERIC(next_char) ||
           (next_char == '_' && IS_ALPHANUMERIC(next_next_char))) {
      next_char = m2c_consume_char(lexer->infile);
      next_next_char = m2c_la2_char(lexer->infile);
    } /* end while */
  }
  
  /* lowline disabled */
  else {
    while (IS_ALPHANUMERIC(next_char)) {
      next_char = m2c_consume_char(lexer->infile);
    } /* end while */
  } /* end if */
  
  /* get lexeme */
  lexer->lookahead.lexeme = m2c_read_marked_lexeme(lexer->infile);
    
  return next_char;
} /* end get_ident */


/* --------------------------------------------------------------------------
 * private function get_ident_or_resword(lexer)
 * ----------------------------------------------------------------------- */

#define IS_NOT_UPPER(c) \
  (((c) < 'A') || ((c) > 'Z'))

static char get_ident_or_resword(m2c_lexer_t lexer, m2c_token_t *token) {
  
  m2c_token_t intermediate_token;
  bool possibly_resword = true;
  char next_char;
  char next_next_char;
  
  m2c_mark_lexeme(lexer->infile);
  next_char = m2c_next_char(lexer->infile);
  next_next_char = m2c_la2_char(lexer->infile);
  
  /* lowline enabled */
  if (m2c_option_lowline_identifiers()) {
    while (IS_ALPHANUMERIC(next_char) ||
           (next_char == '_' && IS_ALPHANUMERIC(next_next_char))) {
      
      if (IS_NOT_UPPER(next_char)) {
        possibly_resword = false;
      } /* end if */
      
      next_char = m2c_consume_char(lexer->infile);
      next_next_char = m2c_la2_char(lexer->infile);
    } /* end while */
  }
  
  /* lowline disabled */
  else {
    while (IS_ALPHANUMERIC(next_char)) {
      
      if (IS_NOT_UPPER(next_char)) {
        possibly_resword = false;
      } /* end if */
      
      next_char = m2c_consume_char(lexer->infile);
    } /* end while */
  } /* end if */
  
  /* get lexeme */
  lexer->lookahead.lexeme = m2c_read_marked_lexeme(lexer->infile);
  
  /* check if lexeme is reserved word */
  if (possibly_resword) {
    intermediate_token =
      m2c_token_for_resword
        (m2c_string_char_ptr(lexer->lookahead.lexeme),
         m2c_string_length(lexer->lookahead.lexeme));
    if (intermediate_token != TOKEN_UNKNOWN) {
      *token = intermediate_token;
    }
    else {
      *token = TOKEN_IDENTIFIER;
    } /* end if */
  }
  else {
    *token = TOKEN_IDENTIFIER;
  } /* end if */
  
  return next_char;
} /* end get_ident_or_resword */

/* --------------------------------------------------------------------------
 * private function get_string_literal(lexer)
 * ----------------------------------------------------------------------- */

static char get_string_literal(m2c_lexer_t lexer, m2c_token_t *token) {
  
  uint_t line, column;
  m2c_token_t intermediate_token;
  char next_char, string_delimiter;
  
  intermediate_token = TOKEN_STRING;
  
  /* consume opening delimiter */
  string_delimiter = m2c_read_char(lexer->infile);
  
  m2c_mark_lexeme(lexer->infile);
  next_char = m2c_next_char(lexer->infile);
  
  while (next_char != string_delimiter) {
    
    /* check for control character */
    if (IS_CONTROL_CHAR(next_char)) {
      line = m2c_infile_current_line(lexer->infile);
      column = m2c_infile_current_column(lexer->infile);
      
      intermediate_token = TOKEN_MALFORMED_STRING;
      
      /* newline */
      if (next_char == '\n') {
        report_error_w_offending_pos
          (M2C_ERROR_NEW_LINE_IN_STRING_LITERAL, lexer,  line, column);
        break;
      }
      /* end-of-file marker */
      else if (IS_EOF(lexer, next_char)) {
        report_error_w_offending_pos
          (M2C_ERROR_EOF_IN_STRING_LITERAL, lexer,  line, column);
        break;
      }
      else /* any other control character */ {
        /* invalid input character */
        report_error_w_offending_char
          (M2C_ERROR_INVALID_INPUT_CHAR, lexer,  line, column, next_char);
      } /* end if */
    } /* end if */
    
    if (m2c_option_escape_tab_and_newline() && (next_char == '\\')) {
      line = m2c_infile_current_line(lexer->infile);
      column = m2c_infile_current_column(lexer->infile);
      next_char = m2c_consume_char(lexer->infile);
      
      if ((next_char != 'n') && (next_char != 't') && (next_char != '\\')) {
        /* invalid escape sequence */
        report_error_w_offending_char
          (M2C_ERROR_INVALID_ESCAPE_SEQUENCE,
           lexer,  line, column, next_char);
      } /* end if */
    } /* end if */
    
    next_char = m2c_consume_char(lexer->infile);
  } /* end while */
  
  /* get lexeme */
  lexer->lookahead.lexeme = m2c_read_marked_lexeme(lexer->infile);
  
  /* consume closing delimiter */
  if (next_char == string_delimiter) {
    next_char = m2c_consume_char(lexer->infile);
  } /* end if */
  
  /* pass back token */
  *token = intermediate_token;
  
  return next_char;
} /* end get_string_literal */


/* --------------------------------------------------------------------------
 * private function get_prefixed_number_literal(lexer, token)
 * ----------------------------------------------------------------------- */

static char get_prefixed_number_literal
  (m2c_lexer_t lexer, m2c_token_t *token) {
  
  m2c_token_t intermediate_token;
  char next_char, la2_char;
  
  m2c_mark_lexeme(lexer->infile);
  next_char = m2c_next_char(lexer->infile);
  la2_char = m2c_la2_char(lexer->infile);
  
  if /* prefix for base-16 integer or character code found */
    ((next_char == 0) && ((la2_char == 'x') || (la2_char == 'u'))) {

    /* consume '0' */
    next_char = m2c_consume_char(lexer->infile);
    
    if /* base-16 integer prefix */ (next_char == 'x') {
      intermediate_token = TOKEN_INTEGER;
    }
    else /* character code prefix */ {
      intermediate_token = TOKEN_CHAR;
    } /* end if */
   
    /* consume prefix */
    next_char = m2c_consume_char(lexer->infile);
    
    /* consume all digits */
    while (IS_DIGIT(next_char) || IS_A_TO_F(next_char)) {
      next_char = m2c_consume_char(lexer->infile);
    } /* end while */
  }
  else /* decimal integer or real number */ {
    
    /* consume all digits */
    while (IS_DIGIT(next_char)) {
      next_char = m2c_consume_char(lexer->infile);
    } /* end while */
    
    if /* real number literal found */ 
      ((next_char == '.') && (m2c_la2_char(lexer->infile) != '.')) {
      
      next_char =
        get_number_literal_fractional_part(lexer, &intermediate_token);
    }
    else {
      intermediate_token = TOKEN_INTEGER;
    } /* end if */
  } /* end if */
  
  /* get lexeme */
  lexer->lookahead.lexeme = m2c_read_marked_lexeme(lexer->infile);
  
  /* pass back token */
  *token = intermediate_token;
  
  return next_char;
} /* end get_prefixed_number_literal */


/* --------------------------------------------------------------------------
 * private function get_suffixed_number_literal(lexer, token)
 * ----------------------------------------------------------------------- */

static char get_suffixed_number_literal
  (m2c_lexer_t lexer, m2c_token_t *token) {
  
  m2c_token_t intermediate_token;
  uint_t char_count_0_to_7 = 0;
  uint_t char_count_8_to_9 = 0;
  uint_t char_count_A_to_F = 0;
  char next_char, last_char;
  
  m2c_mark_lexeme(lexer->infile);
  next_char = m2c_next_char(lexer->infile);
  
  /* consume any characters '0' to '9' and 'A' to 'F' */
  while (IS_DIGIT(next_char) || IS_A_TO_F(next_char)) {
    
    if ((next_char >= '0') && (next_char <= '7')) {
      char_count_0_to_7++;
    }
    else if ((next_char == '8') || (next_char == '9')) {
      char_count_8_to_9++;
    }
    else {
      char_count_A_to_F++;
    } /* end if */
    
    last_char = next_char;
    next_char = m2c_consume_char(lexer->infile);
  } /* end while */
  
  if /* base-16 integer found */ (next_char == 'H') {
    
    next_char = m2c_consume_char(lexer->infile);
    intermediate_token = TOKEN_INTEGER;
  }
  else if /* base-10 integer or real number found */
    (char_count_A_to_F == 0) {
    
    if /* real number literal found */ 
      ((next_char == '.') && (m2c_la2_char(lexer->infile) != '.')) {
      
      next_char =
        get_number_literal_fractional_part(lexer, &intermediate_token);
    }
    else /* decimal integer found */ {
      intermediate_token = TOKEN_INTEGER;
    } /* end if */
  }
  else if /* base-8 integer found */
    (m2c_option_octal_literals() &&
     (char_count_8_to_9 == 0) && (char_count_A_to_F == 1) && 
     ((last_char == 'B') || (last_char == 'C'))) {
    
    if (last_char == 'B') {
      intermediate_token = TOKEN_INTEGER;
    }
    else /* last_char == 'C' */ {
      intermediate_token = TOKEN_CHAR;
    } /* end if */    
  }
  else /* malformed base-16 integer */ {
    intermediate_token = TOKEN_MALFORMED_INTEGER;
  } /* end if */
  
  /* get lexeme */
  lexer->lookahead.lexeme = m2c_read_marked_lexeme(lexer->infile);
  
  /* pass back token */
  *token = intermediate_token;
  
  return next_char;
} /* end get_suffixed_number_literal */


/* --------------------------------------------------------------------------
 * private function get_number_literal_fractional_part(lexer, token)
 * ----------------------------------------------------------------------- */

static char get_number_literal_fractional_part
  (m2c_lexer_t lexer, m2c_token_t *token) {
  
  m2c_token_t intermediate_token = TOKEN_REAL;
  char next_char;
    
  /* consume the decimal point */
  next_char = m2c_consume_char(lexer->infile);
  
  /* consume any fractional digits */
  while (IS_DIGIT(next_char)) {
    next_char = m2c_consume_char(lexer->infile);
  } /* end if */
  
  if /* exponent prefix found */ (next_char == 'E') {
  
    /* consume exponent prefix */
    next_char = m2c_consume_char(lexer->infile);
    
    if /* exponent sign found*/
      ((next_char == '+') || (next_char == '-')) {
      
      /* consume exponent sign */
      next_char = m2c_consume_char(lexer->infile);
    } /* end if */
    
    if /* exponent digits found */ (IS_DIGIT(next_char)) {
    
      /* consume exponent digits */
      while (IS_DIGIT(next_char)) {
        next_char = m2c_consume_char(lexer->infile);
      } /* end while */
    }
    else /* exponent digits missing */ {
      intermediate_token = TOKEN_MALFORMED_REAL;
    } /* end if */
  } /* end if */
  
  /* pass back token */
  *token = intermediate_token;
  
  return next_char;
} /* end get_number_literal_fractional_part */


/* END OF FILE */
