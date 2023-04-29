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
 * @file
 *
 * m2-token.h
 *
 * Public interface for M2T token type.
 *
 * @license
 *
 * M2T is free software:  You can redistribute and modify it  under the terms
 * of the  GNU Lesser General Public License (LGPL) either version 2.1  or at
 * your choice version 3, both as published by the Free Software Foundation.
 *
 * M2T is distributed  in the hope  that it will be useful,  but  WITHOUT ANY
 * WARRANTY; without even  the implied warranty of MERCHANTABILITY or FITNESS
 * FOR ANY PARTICULAR PURPOSE.  Read the license for more details.
 *
 * You should have received  a copy of the  GNU Lesser General Public License
 * along with M2T.  If not, see <https://www.gnu.org/copyleft/lesser.html>.
 */

#ifndef M2T_TOKEN_H
#define M2T_TOKEN_H


#include <stdbool.h>
#include "m2t-common.h"


/* --------------------------------------------------------------------------
 * type m2t_token_t
 * --------------------------------------------------------------------------
 * Enumerated token values representing Modula-2 terminal symbols.
 * --------------------------------------------------------------------------
 */

typedef enum {
  /* Null Token */
  
  TOKEN_UNKNOWN,
  
  /* Reserved Words */

  TOKEN_AND,              /* "AND" */
  TOKEN_ARRAY,            /* "ARRAY" */
  TOKEN_BEGIN,            /* "BEGIN" */
  TOKEN_BY,               /* "BY" */
  TOKEN_CASE,             /* "CASE" */
  TOKEN_CONST,            /* "CONST" */
  TOKEN_DEFINITION,       /* "DEFINITION" */
  TOKEN_DIV,              /* "DIV" */
  TOKEN_DO,               /* "DO" */
  TOKEN_ELSE,             /* "ELSE" */
  TOKEN_ELSIF,            /* "ELSIF" */
  TOKEN_END,              /* "END" */
  TOKEN_EXIT,             /* "EXIT" */
  TOKEN_EXPORT,           /* "EXPORT" */
  TOKEN_FOR,              /* "FOR" */
  TOKEN_FROM,             /* "FROM" */
  TOKEN_IF,               /* "IF" */
  TOKEN_IMPLEMENTATION,   /* "IMPLEMENTATION" */
  TOKEN_IMPORT,           /* "IMPORT" */
  TOKEN_IN,               /* "IN" */
  TOKEN_LOOP,             /* "LOOP" */
  TOKEN_MOD,              /* "MOD" */
  TOKEN_MODULE,           /* "MODULE" */
  TOKEN_NOT,              /* "NOT" */
  TOKEN_OF,               /* "OF" */
  TOKEN_OR,               /* "OR" */
  TOKEN_POINTER,          /* "POINTER" */
  TOKEN_PROCEDURE,        /* "PROCEDURE" */
  TOKEN_QUALIFIED,        /* "QUALIFIED" */
  TOKEN_RECORD,           /* "RECORD" */
  TOKEN_REPEAT,           /* "REPEAT" */
  TOKEN_RETURN,           /* "RETURN" */
  TOKEN_SET,              /* "SET" */
  TOKEN_THEN,             /* "THEN" */
  TOKEN_TO,               /* "TO" */
  TOKEN_TYPE,             /* "TYPE" */
  TOKEN_UNTIL,            /* "UNTIL" */
  TOKEN_VAR,              /* "VAR" */
  TOKEN_WHILE,            /* "WHILE" */
  TOKEN_WITH,             /* "WITH" */
  
  /* Identifiers */
  
  TOKEN_IDENTIFIER,   /* Letter (Letter | Digit)* */
  
  /* Literals */
  
  TOKEN_STRING,       /* '"' Character* '"' | "'" Character* "'" */
  TOKEN_INTEGER,      /* OctDigit (OctDigit* 'B' | Digit* | HexDigit 'H')* */
  TOKEN_REAL,         /* Digit* '.' (Digit* ('E' ('+'|'-')? Digit*)? )? */
  TOKEN_CHAR,         /* "'" Character "'" | OctDigit* 'C' */
  
  TOKEN_MALFORMED_STRING,    /* string without closing delimiter */
  TOKEN_MALFORMED_INTEGER,   /* base-16 number without suffix H */
  TOKEN_MALFORMED_REAL,      /* real number with E but no exponent */
  
  /* Pragmas */
  
  TOKEN_PRAGMA,   /* '(*$' Character* '*)' */
  
  /* Special Symbols */
  
  TOKEN_PLUS,                /* '+' */
  TOKEN_MINUS,               /* '-' */
  TOKEN_EQUAL,               /* '+' */
  TOKEN_NOTEQUAL,            /* '#' */
  TOKEN_LESS,                /* '<' */
  TOKEN_LESS_EQUAL,          /* '<=' */
  TOKEN_GREATER,             /* '>' */
  TOKEN_GREATER_EQUAL,       /* '>=' */
  TOKEN_ASTERISK,            /* '*' */
  TOKEN_SOLIDUS,             /* '/' */
  TOKEN_ASSIGN,              /* ':=' */
  TOKEN_COMMA,               /* ',' */
  TOKEN_PERIOD,              /* '.' */
  TOKEN_COLON,               /* ':' */
  TOKEN_SEMICOLON,           /* ';' */
  TOKEN_RANGE,               /* '..' */
  TOKEN_DEREF,               /* '^' */
  TOKEN_BAR,                 /* '|' */
  TOKEN_LEFT_PAREN,          /* '(' */
  TOKEN_RIGHT_PAREN,         /* ')' */
  TOKEN_LEFT_BRACKET,        /* '[' */
  TOKEN_RIGHT_BRACKET,       /* ']' */
  TOKEN_LEFT_BRACE,          /* '{' */
  TOKEN_RIGHT_BRACE,         /* '}' */
  TOKEN_END_OF_FILE,
  
  /* Synonyms */
  
  /*  '&' is a synonym for AND and mapped to TOKEN_AND */
  /*  '~' is a synonym for NOT and mapped to TOKEN_NOT */
  /* '<>' is a synonym for '#' and mapped to TOKEN_NOTEQUAL */
  
  /* Enumeration Terminator */
  
  TOKEN_END_MARK /* marks the end of this enumeration */
} m2t_token_t;


/* --------------------------------------------------------------------------
 * token count
 * ----------------------------------------------------------------------- */

#define TOKEN_COUNT TOKEN_END_MARK


/* --------------------------------------------------------------------------
 * first and last reserved word tokens
 * ----------------------------------------------------------------------- */

#define FIRST_RESERVED_WORD_TOKEN TOKEN_AND
#define LAST_RESERVED_WORD_TOKEN TOKEN_WITH


/* --------------------------------------------------------------------------
 * first and last special symbol tokens
 * ----------------------------------------------------------------------- */

#define FIRST_SPECIAL_SYMBOL_TOKEN TOKEN_PLUS
#define LAST_SPECIAL_SYMBOL_TOKEN TOKEN_RIGHT_BRACE


/* --------------------------------------------------------------------------
 * function m2t_is_valid_token(token)
 * --------------------------------------------------------------------------
 * Returns TRUE if token represents a terminal symbol, otherwise FALSE.
 * ----------------------------------------------------------------------- */

bool m2t_is_valid_token (m2t_token_t token);


/* --------------------------------------------------------------------------
 * function m2t_is_resword_token(token)
 * --------------------------------------------------------------------------
 * Returns TRUE if token represents a reserved word, otherwise FALSE.
 * ----------------------------------------------------------------------- */

bool m2t_is_resword_token (m2t_token_t token);


/* --------------------------------------------------------------------------
 * function m2t_is_literal_token(token)
 * --------------------------------------------------------------------------
 * Returns TRUE if token represents a literal, otherwise FALSE.
 * ----------------------------------------------------------------------- */

bool m2t_is_literal_token (m2t_token_t token);


/* --------------------------------------------------------------------------
 * function m2t_is_malformed_literal_token(token)
 * --------------------------------------------------------------------------
 * Returns TRUE if token represents a malformed literal, otherwise FALSE.
 * ----------------------------------------------------------------------- */

bool m2t_is_malformed_literal_token (m2t_token_t token);


/* --------------------------------------------------------------------------
 * function m2t_is_special_symbol_token(token)
 * --------------------------------------------------------------------------
 * Returns TRUE if token represents a special symbol, otherwise FALSE.
 * ----------------------------------------------------------------------- */

bool m2t_is_special_symbol_token (m2t_token_t token);


/* --------------------------------------------------------------------------
 * function m2t_token_for_resword(lexeme, length)
 * --------------------------------------------------------------------------
 * Tests if the given lexeme represents a reserved word and returns the
 * corresponding token or TOKEN_UNKNOWN if it does not match a reserved word.
 * ----------------------------------------------------------------------- */

m2t_token_t m2t_token_for_resword (const char *lexeme, uint_t length);


/* --------------------------------------------------------------------------
 * function m2t_lexeme_for_resword(token)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to a NUL terminated character string with
 * the lexeme for the reserved word represented by token.  Returns NULL
 * if the token does not represent a reserved word.
 * ----------------------------------------------------------------------- */

const char *m2t_lexeme_for_resword (m2t_token_t token);


/* --------------------------------------------------------------------------
 * function m2t_lexeme_for_special_symbol(token)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to a NUL terminated character string with
 * the lexeme for the special symbol represented by token.  Returns NULL
 * if the token does not represent a special symbol.
 * ----------------------------------------------------------------------- */

const char *m2t_lexeme_for_special_symbol (m2t_token_t token);


/* --------------------------------------------------------------------------
 * function m2t_name_for_token(token)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to a NUL terminated character string with
 * a human readable name for token.  Returns NULL token is not a valid token.
 * ----------------------------------------------------------------------- */

const char *m2t_name_for_token (m2t_token_t token);


#endif /* M2T_TOKEN_H */

/* END OF FILE */