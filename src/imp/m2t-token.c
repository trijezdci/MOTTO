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
 * m2t-token.c
 *
 * M2T token type implementation.
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

#include "m2t-token.h"

#include <stddef.h>


/* --------------------------------------------------------------------------
 * array m2t_token_name_table
 * --------------------------------------------------------------------------
 * Human readable names for tokens.
 * ----------------------------------------------------------------------- */

static const char *m2t_token_name_table[] = {
  /* Null Token */
  
  "UNKNOWN",
  
  /* Reserved Words */

  "RW-AND\0",
  "RW-ARRAY\0",
  "RW-BEGIN\0",
  "RW-BY\0",
  "RW-CASE\0",
  "RW-CONST\0",
  "RW-DEFINITION\0",
  "RW-DIV\0",
  "RW-DO\0",
  "RW-ELSE\0",
  "RW-ELSIF\0",
  "RW-END\0",
  "RW-EXIT\0",
  "RW-EXPORT\0",
  "RW-FOR\0",
  "RW-FROM\0",
  "RW-IF\0",
  "RW-IMPLEMENTATION\0",
  "RW-IMPORT\0",
  "RW-IN\0",
  "RW-LOOP\0",
  "RW-MOD\0",
  "RW-MODULE\0",
  "RW-NOT\0",
  "RW-OF\0",
  "RW-OR\0",
  "RW-POINTER\0",
  "RW-PROCEDURE\0",
  "RW-QUALIFIED\0",
  "RW-RECORD\0",
  "RW-REPEAT\0",
  "RW-RETURN\0",
  "RW-SET\0",
  "RW-THEN\0",
  "RW-TO\0",
  "RW-TYPE\0",
  "RW-UNTIL\0",
  "RW-VAR\0",
  "RW-WHILE\0",
  "RW-WITH\0",
  
  /* Identifiers */
  
  "IDENTIFIER\0",
  
  /* Literals */
  
  "STRING-LITERAL\0",
  "INTEGER-LITERAL\0",
  "REAL-LITERAL\0",
  "CHAR-LITERAL\0",
  
  "MALFORMED-STRING\0",
  "MALFORMED-INTEGER\0",
  "MALFORMED-REAL\0",
  
  /* Pragmas */
  
  "PRAGMA\0",
  
  /* Special Symbols */
  
  "PLUS\0",
  "MINUS\0",
  "EQUAL\0",
  "NOTEQUAL\0",
  "LESS-THAN\0",
  "LESS-OR-EQUAL\0",
  "GREATER-THAN\0",
  "GREATER-OR-EQUAL\0",
  "ASTERISK\0",
  "SOLIDUS\0",
  "ASSIGNMENT\0",
  "COMMA\0",
  "PERIOD\0",
  "COLON\0",
  "SEMICOLON\0",
  "RANGE\0",
  "DEREF\0",
  "VERTICAL-BAR\0",
  "LEFT-PAREN\0",
  "RIGHT-PAREN\0",
  "LEFT-BRACKET\0",
  "RIGHT-BRACKET\0",
  "LEFT-BRACE\0",
  "RIGHT-BRACE\0",
  "END-OF-FILE\0",
  
  /* out-of-range guard */
  
  "\0"

}; /* end m2c_token_name_table */


/* --------------------------------------------------------------------------
 * array m2t_resword_lexeme_table
 * --------------------------------------------------------------------------
 * Lexeme strings for reserved word tokens.
 * ----------------------------------------------------------------------- */

static const char *m2t_resword_lexeme_table[] = {
  
  /* dummy */
  "\0",
  
  /* lexemes */
  "AND\0",
  "ARRAY\0",
  "BEGIN\0",
  "BY\0",
  "CASE\0",
  "CONST\0",
  "DEFINITION\0",
  "DIV\0",
  "DO\0",
  "ELSE\0",
  "ELSIF\0",
  "END\0",
  "EXIT\0",
  "EXPORT\0",
  "FOR\0",
  "FROM\0",
  "IF\0",
  "IMPLEMENTATION\0",
  "IMPORT\0",
  "IN\0",
  "LOOP\0",
  "MOD\0",
  "MODULE\0",
  "NOT\0",
  "OF\0",
  "OR\0",
  "POINTER\0",
  "PROCEDURE\0",
  "QUALIFIED\0",
  "RECORD\0",
  "REPEAT\0",
  "RETURN\0",
  "SET\0",
  "THEN\0",
  "TO\0",
  "TYPE\0",
  "UNTIL\0",
  "VAR\0",
  "WHILE\0",
  "WITH\0",

}; /* end m2t_resword_lexeme_table */


/* --------------------------------------------------------------------------
 * array m2t_special_symbol_lexeme_table
 * --------------------------------------------------------------------------
 * Lexeme strings for special symbol tokens.
 * ----------------------------------------------------------------------- */

static const char *m2t_special_symbol_lexeme_table[] = {
  
  "+\0",  /* PLUS */
  "-\0",  /* MINUS */
  "=\0",  /* EQUAL */
  "#\0",  /* NOTEQUAL */
  "<\0",  /* LESS-THAN */
  "<=\0", /* LESS-OR-EQUAL */
  ">\0",  /* GREATER-THAN */
  ">=\0", /* GREATER-OR-EQUAL */ 
  "*\0",  /* ASTERISK */         
  "/\0",  /* SOLIDUS */
  ":=\0", /* ASSIGNMENT */       
  ",\0",  /* COMMA */ 
  ".\0",  /* PERIOD */ 
  ":\0",  /* COLON */ 
  ";\0",  /* SEMICOLON */ 
  "..\0", /* RANGE */ 
  "^\0",  /* DEREF */ 
  "|\0",  /* VERTICAL-BAR */ 
  "(\0",  /* LEFT-PAREN */ 
  ")\0",  /* RIGHT-PAREN */ 
  "[\0",  /* LEFT-BRACKET */ 
  "]\0",  /* RIGHT-BRACKET */ 
  "{\0",  /* LEFT-BRACE */ 
  "}\0"   /* RIGHT-BRACE */ 
  
}; /* end m2t_special_symbol_lexeme_table */


/* --------------------------------------------------------------------------
 * private function str_match(str1, str2)
 * --------------------------------------------------------------------------
 * Returns true character strings str1 and str2 match, otherwise false.
 * ----------------------------------------------------------------------- */

#define MAX_RESWORD_LENGTH 14

static bool str_match (const char *str1, const char *str2) {
  uint_t index;
  
  index = 0;
  while (index <= MAX_RESWORD_LENGTH + 1) {
    if (str1[index] != str2[index]) {
      return false;
    }
    else if (str1[index] == '\0') {
      return true;
    }
    else {
      index++;
    } /* end if */
  } /* end while */
  
  return false;
} /* end str_match */


/* --------------------------------------------------------------------------
 * function m2t_is_valid_token(token)
 * --------------------------------------------------------------------------
 * Returns true if token represents a terminal symbol, otherwise false.
 * ----------------------------------------------------------------------- */

inline bool m2t_is_valid_token (m2t_token_t token) {
  return ((token > TOKEN_UNKNOWN) && (token < TOKEN_END_MARK));
} /* end  */


/* --------------------------------------------------------------------------
 * function m2t_is_resword_token(token)
 * --------------------------------------------------------------------------
 * Returns true if token represents a reserved word, otherwise false.
 * ----------------------------------------------------------------------- */

inline bool m2t_is_resword_token (m2t_token_t token) {
  return ((token >= TOKEN_AND) && (token <= TOKEN_WITH));
} /* end m2t_is_resword_token */


/* --------------------------------------------------------------------------
 * function m2t_is_literal_token(token)
 * --------------------------------------------------------------------------
 * Returns true if token represents a literal, otherwise false.
 * ----------------------------------------------------------------------- */

inline bool m2t_is_literal_token (m2t_token_t token) {
  return ((token >= TOKEN_STRING) && (token <= TOKEN_CHAR));
} /* end m2t_is_literal_token */


/* --------------------------------------------------------------------------
 * function m2t_is_malformed_literal_token(token)
 * --------------------------------------------------------------------------
 * Returns TRUE if token represents a malformed literal, otherwise FALSE.
 * ----------------------------------------------------------------------- */

inline bool m2t_is_malformed_literal_token (m2t_token_t token) {
  return ((token >= TOKEN_MALFORMED_STRING) &&
          (token <= TOKEN_MALFORMED_REAL));
} /* end m2t_is_malformed_literal_token */


/* --------------------------------------------------------------------------
 * function m2t_is_special_symbol_token(token)
 * --------------------------------------------------------------------------
 * Returns TRUE if token represents a special symbol, otherwise FALSE.
 * ----------------------------------------------------------------------- */

inline bool m2t_is_special_symbol_token (m2t_token_t token) {
  return ((token >= TOKEN_ADDITION) && (token <= TOKEN_RIGHT_BRACE));
} /* end m2t_is_special_symbol_token */


/* --------------------------------------------------------------------------
 * function m2t_token_for_resword(lexeme, length)
 * --------------------------------------------------------------------------
 * Tests if the given lexeme represents a reserved word and returns the
 * corresponding token or TOKEN_UNKNOWN if it does not match a reserved word.
 * ----------------------------------------------------------------------- */

m2t_token_t m2t_token_for_resword (const char *lexeme, uint_t length) {
  
  /* verify pre-conditions */
  if ((lexeme == NULL) || (length < 2) || (length > 14)) {
    return TOKEN_UNKNOWN;
  } /* end if */
  
  switch (length) {
    case /* length = 2 */ 2 :
      switch (lexeme[0]) {
      
	    case 'B' :
	      /* BY */
	      if (lexeme[1] == 'Y') {
	        return TOKEN_BY;
	      }
	    
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	
	    case 'D' :
	      /* DO */
	      if (lexeme[1] == 'O') {
	        return TOKEN_DO;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	
	    case 'I' :
	      /* IF */
	      if (lexeme[1] == 'F') {
	        return TOKEN_IF;
	      }
	  	    
	      /* IN */
	      else if (lexeme[1] == 'N') {
	        return TOKEN_IN;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	    
	    case 'O' :
	      /* OF */
	      if (lexeme[1] == 'F') {
	        return TOKEN_OF;
	      }
	      
	      /* OR */
	      else if (lexeme[1] == 'R') {
	        return TOKEN_OR;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	    
	    case 'T' :
	      /* TO */
	      if (lexeme[1] == 'O') {
	        return TOKEN_TO;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	    
	    default :
	      return TOKEN_UNKNOWN;
	  } /* end switch */
	
    case /* length = 3 */ 3 :
      switch (lexeme[0]) {
      
	    case 'A' :
	      /* AND */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_AND])) {
	        return TOKEN_AND;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
        	
	    case 'D' :
	      /* DIV */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_DIV])) {
	        return TOKEN_DIV;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	
	    case 'E' :
	      /* END */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_END])) {
	        return TOKEN_END;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'F' :
	      /* FOR */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_FOR])) {
	        return TOKEN_FOR;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'M' :
	      /* MOD */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_MOD])) {
	        return TOKEN_MOD;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'N' :
	      /* NOT */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_NOT])) {
	        return TOKEN_NOT;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'S' :
	      /* SET */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_SET])) {
	        return TOKEN_SET;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'V' :
	      /* VAR */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_VAR])) {
	        return TOKEN_VAR;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    default :
	      return TOKEN_UNKNOWN;
	  } /* end switch */
	
    case /* length = 4 */ 4 :
      switch (lexeme[1]) {
      
	    case 'A' :
	      /* CASE */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_CASE])) {
	        return TOKEN_CASE;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'H' :
	      /* THEN */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_THEN])) {
	        return TOKEN_THEN;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'I' :
	      /* WITH */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_WITH])) {
	        return TOKEN_WITH;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'L' :
	      /* ELSE */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_ELSE])) {
	        return TOKEN_ELSE;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'O' :
	      /* LOOP */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_LOOP])) {
	        return TOKEN_LOOP;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'R' :
	      /* FROM */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_FROM])) {
	        return TOKEN_FROM;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'X' :
	      /* EXIT */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_EXIT])) {
	        return TOKEN_EXIT;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
        case 'Y' :
          /* TYPE */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_TYPE])) {
	        return TOKEN_TYPE;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    default :
	      return TOKEN_UNKNOWN;
	  } /* end switch */
	
    case /* length = 5 */ 5 :
      switch (lexeme[0]) {
      
	    case 'A' :
	      /* ARRAY */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_ARRAY])) {
	        return TOKEN_ARRAY;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'B' :
	      /* BEGIN */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_BEGIN])) {
	        return TOKEN_BEGIN;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'C' :
	      /* CONST */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_CONST])) {
	        return TOKEN_CONST;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'E' :
	      /* ELSIF */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_ELSIF])) {
	        return TOKEN_ELSIF;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'U' :
	      /* UNTIL */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_UNTIL])) {
	        return TOKEN_UNTIL;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'W' :
	      /* WHILE */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_WHILE])) {
	        return TOKEN_WHILE;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    default :
	      return TOKEN_UNKNOWN;
	  } /* end switch */
	
    case /* length 6 */ 6 :
      switch (lexeme[5]) {
      
	    case 'E' :
	      /* MODULE */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_MODULE])) {
	        return TOKEN_MODULE;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'D' :
	      /* RECORD */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_RECORD])) {
	        return TOKEN_RECORD;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'N' :
	      /* RETURN */
	      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_RETURN])) {
	        return TOKEN_RETURN;
	      }
	      
	      else {
	        return TOKEN_UNKNOWN;
	      } /* end if */
	      
	    case 'T' :
	      switch (lexeme[0]) {
	      
	        case 'E' :
	          /* EXPORT */
	          if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_EXPORT])) {
	            return TOKEN_EXPORT;
	          }
	          
	          else {
	            return TOKEN_UNKNOWN;
	          } /* end if */
	        
	        case 'I' :
	          /* IMPORT */
	          if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_IMPORT])) {
	            return TOKEN_IMPORT;
	          }
	          
	          else {
	            return TOKEN_UNKNOWN;
	          } /* end if */
	        
	        case 'R' :
	          /* REPEAT */
	          if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_REPEAT])) {
	            return TOKEN_REPEAT;
	          }
	          
	          else {
	            return TOKEN_UNKNOWN;
	          } /* end if */
	          
	        default :
	          return TOKEN_UNKNOWN;
	      } /* end switch */
	      
	    default :
	      return TOKEN_UNKNOWN;
	  } /* end switch */
	
    case /* length = 7 */ 7 :
      /* POINTER */
      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_POINTER])) {
	    return TOKEN_POINTER;
      }
      
      else {
	    return TOKEN_UNKNOWN;
      } /* end if */
      
    case /* length = 9 */ 9 :
      switch (lexeme[0]) {
      
        case 'P' :
          /* PROCEDURE */
          if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_PROCEDURE])) {
	        return TOKEN_PROCEDURE;
          }
          
          else {
	        return TOKEN_UNKNOWN;
          } /* end if */
        
        case 'Q' :
          /* QUALIFIED */
          if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_QUALIFIED])) {
	        return TOKEN_QUALIFIED;
          }
          
          else {
	        return TOKEN_UNKNOWN;
          } /* end if */
        
        default :
	      return TOKEN_UNKNOWN;
      } /* end switch */
	
    case /* length = 10 */ 10 :
      /* DEFINITION */
      if (str_match(lexeme, m2t_resword_lexeme_table[TOKEN_DEFINITION])) {
	    return TOKEN_DEFINITION;
      }
      
      else {
	    return TOKEN_UNKNOWN;
      } /* end if */
    
    case /* length = 14 */ 14 :
      /* IMPLEMENTATION */
      if (str_match(lexeme,
          m2t_resword_lexeme_table[TOKEN_IMPLEMENTATION])) {
	    return TOKEN_IMPLEMENTATION;
      }
      
      else {
	    return TOKEN_UNKNOWN;
      } /* end if */
    
    default :
      return TOKEN_UNKNOWN;
  } /* end switch (length) */
} /* end m2t_token_for_resword */


/* --------------------------------------------------------------------------
 * function m2t_lexeme_for_resword(token)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to a NUL terminated character string with
 * the lexeme for the reserved word represented by token.  Returns NULL
 * if the token does not represent a reserved word.
 * ----------------------------------------------------------------------- */

const char *m2t_lexeme_for_resword (m2t_token_t token) {
  uint_t index;
  
  /* check pre-conditions */
  if ((token < FIRST_RESERVED_WORD_TOKEN) ||
      (token > LAST_RESERVED_WORD_TOKEN)) {
    return NULL;
  } /* end if */
  
  index = (uint_t) token;
    
  /* return lexeme */
  return m2t_resword_lexeme_table[index];
  
} /* end m2t_lexeme_for_resword */


/* --------------------------------------------------------------------------
 * function m2t_lexeme_for_special_symbol(token)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to a NUL terminated character string with
 * the lexeme for the special symbol represented by token.  Returns NULL
 * if the token does not represent a special symbol.
 * ----------------------------------------------------------------------- */

const char *m2t_lexeme_for_special_symbol (m2t_token_t token) {
  uint_t index;
  
  /* check pre-conditions */
  if ((token < FIRST_SPECIAL_SYMBOL_TOKEN) ||
      (token > LAST_SPECIAL_SYMBOL_TOKEN)) {
    return NULL;
  } /* end if */
  
  index = token - FIRST_SPECIAL_SYMBOL_TOKEN;
  
  /* return lexeme */
  return m2t_special_symbol_lexeme_table[index];
  
} /* end m2t_lexeme_for_special_symbol */


/* --------------------------------------------------------------------------
 * function m2t_name_for_token(token)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to a NUL terminated character string with
 * a human readable name for token.  Returns NULL token is not a valid token.
 * ----------------------------------------------------------------------- */

const char *m2t_name_for_token (m2t_token_t token) {
  if (token < TOKEN_END_MARK) {
    return m2t_token_name_table[token];
  }
  else /* invalid token */ {
    return NULL;
  } /* end if */
} /* end m2t_name_for_token */

/* END OF FILE */