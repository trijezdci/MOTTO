/* C String Library
 * Copyright (c) 2015 Benjamin Kowarsch
 *
 * @file
 *
 * cstring.h
 *
 * Implementation of C string library.
 *
 * @license
 *
 * This library is free software: you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) either ver 2.1
 * or at your choice ver 3 as published by the Free Software Foundation.
 *
 * It is distributed in the hope that it will be useful,  but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  Read the license for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * with the library.  If not, see <https://www.gnu.org/copyleft/lesser.html>.
 */

#include "cstring.h"


/* --------------------------------------------------------------------------
 * C standard library
 * ----------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdarg.h>


/* --------------------------------------------------------------------------
 * C string terminator
 * ----------------------------------------------------------------------- */

#define ASCII_NUL 0


/* --------------------------------------------------------------------------
 * Sentinel values
 * ----------------------------------------------------------------------- */

#define NULL_RANK (-1)
#define RANK_NONE (-2)


/* --------------------------------------------------------------------------
 * Collation ranking table for dictionary mode, lower- before uppercase
 * ----------------------------------------------------------------------- */

static const short int dict_lower_rank[] = {
  /* NUL */ NULL_RANK,    /* SOH */ RANK_NONE,     /* STX */ RANK_NONE,
  /* ETX */ RANK_NONE,    /* EOT */ RANK_NONE,     /* ENQ */ RANK_NONE,
  /* ACK */ RANK_NONE,    /* BEL */ RANK_NONE,     /* BS  */ RANK_NONE,
  /* TAB */ RANK_NONE,    /* LF  */ RANK_NONE,     /* VT  */ RANK_NONE,
  /* FF  */ RANK_NONE,    /* CR  */ RANK_NONE,     /* SO  */ RANK_NONE,
  /* SI  */ RANK_NONE,    /* DLE */ RANK_NONE,     /* DC1 */ RANK_NONE,
  /* DC2 */ RANK_NONE,    /* DC3 */ RANK_NONE,     /* DC4 */ RANK_NONE,
  /* NAK */ RANK_NONE,    /* SYN */ RANK_NONE,     /* ETB */ RANK_NONE,
  /* CAN */ RANK_NONE,    /* EM  */ RANK_NONE,     /* SUB */ RANK_NONE,
  /* ESC */ RANK_NONE,    /* FS  */ RANK_NONE,     /* GS  */ RANK_NONE,
  /* RS  */ RANK_NONE,    /* US  */ RANK_NONE,     /* ' ' */ 0,
  /* '!' */ RANK_NONE,    /* '"' */ RANK_NONE,     /* '#' */ 1,
  /* '$' */ 2,            /* '%' */ 3,             /* '&' */ RANK_NONE,
  /* "'" */ RANK_NONE,    /* '(' */ RANK_NONE,     /* ')' */ RANK_NONE,
  /* '*' */ 4,            /* '+' */ 5,             /* ',' */ RANK_NONE,
  /* '-' */ 0,            /* '.' */ RANK_NONE,     /* '/' */ RANK_NONE,
  /* '0' */ 6,            /* '1' */ 7,             /* '2' */ 8,
  /* '3' */ 9,            /* '4' */ 10,            /* '5' */ 11,
  /* '6' */ 12,           /* '7' */ 13,            /* '8' */ 14,
  /* '9' */ 15,           /* ':' */ RANK_NONE,     /* ';' */ RANK_NONE,
  /* '<' */ RANK_NONE,    /* '=' */ RANK_NONE,     /* '>' */ RANK_NONE,
  /* '?' */ RANK_NONE,    /* '@' */ RANK_NONE,     /* 'A' */ 17,
  /* 'B' */ 19,           /* 'C' */ 21,            /* 'D' */ 23,
  /* 'E' */ 25,           /* 'F' */ 27,            /* 'G' */ 29,
  /* 'H' */ 31,           /* 'I' */ 33,            /* 'J' */ 35,
  /* 'K' */ 37,           /* 'L' */ 39,            /* 'M' */ 41,
  /* 'N' */ 43,           /* 'O' */ 45,            /* 'P' */ 47,
  /* 'Q' */ 49,           /* 'R' */ 51,            /* 'S' */ 53,
  /* 'T' */ 55,           /* 'U' */ 57,            /* 'V' */ 59,
  /* 'W' */ 61,           /* 'X' */ 63,            /* 'Y' */ 65,
  /* 'Z' */ 67,           /* '[' */ RANK_NONE,     /* '\' */ RANK_NONE,
  /* ']' */ RANK_NONE,    /* '^' */ RANK_NONE,     /* '_' */ RANK_NONE,
  /* '`' */ RANK_NONE,    /* 'a' */ 16,            /* 'b' */ 18,
  /* 'c' */ 20,           /* 'd' */ 22,            /* 'e' */ 24,
  /* 'f' */ 26,           /* 'g' */ 28,            /* 'h' */ 30,
  /* 'i' */ 32,           /* 'j' */ 34,            /* 'k' */ 36,
  /* 'l' */ 38,           /* 'm' */ 40,            /* 'n' */ 42,
  /* 'o' */ 44,           /* 'p' */ 46,            /* 'q' */ 48,
  /* 'r' */ 50,           /* 's' */ 52,            /* 't' */ 54,
  /* 'u' */ 56,           /* 'v' */ 58,            /* 'w' */ 60,
  /* 'x' */ 62,           /* 'y' */ 64,            /* 'z' */ 66,
  /* '{' */ RANK_NONE,    /* '|' */ RANK_NONE,     /* '}' */ RANK_NONE,
  /* '~' */ RANK_NONE,    /* DEL */ RANK_NONE
}; /* end rank */


/* --------------------------------------------------------------------------
 * Collation ranking table for dictionary mode, upper- before lowercase
 * ----------------------------------------------------------------------- */

static const short int dict_upper_rank[] = {
  /* NUL */ NULL_RANK,    /* SOH */ RANK_NONE,     /* STX */ RANK_NONE,
  /* ETX */ RANK_NONE,    /* EOT */ RANK_NONE,     /* ENQ */ RANK_NONE,
  /* ACK */ RANK_NONE,    /* BEL */ RANK_NONE,     /* BS  */ RANK_NONE,
  /* TAB */ RANK_NONE,    /* LF  */ RANK_NONE,     /* VT  */ RANK_NONE,
  /* FF  */ RANK_NONE,    /* CR  */ RANK_NONE,     /* SO  */ RANK_NONE,
  /* SI  */ RANK_NONE,    /* DLE */ RANK_NONE,     /* DC1 */ RANK_NONE,
  /* DC2 */ RANK_NONE,    /* DC3 */ RANK_NONE,     /* DC4 */ RANK_NONE,
  /* NAK */ RANK_NONE,    /* SYN */ RANK_NONE,     /* ETB */ RANK_NONE,
  /* CAN */ RANK_NONE,    /* EM  */ RANK_NONE,     /* SUB */ RANK_NONE,
  /* ESC */ RANK_NONE,    /* FS  */ RANK_NONE,     /* GS  */ RANK_NONE,
  /* RS  */ RANK_NONE,    /* US  */ RANK_NONE,     /* ' ' */ 0,
  /* '!' */ RANK_NONE,    /* '"' */ RANK_NONE,     /* '#' */ 1,
  /* '$' */ 2,            /* '%' */ 3,             /* '&' */ RANK_NONE,
  /* "'" */ RANK_NONE,    /* '(' */ RANK_NONE,     /* ')' */ RANK_NONE,
  /* '*' */ 4,            /* '+' */ 5,             /* ',' */ RANK_NONE,
  /* '-' */ 0,            /* '.' */ RANK_NONE,     /* '/' */ RANK_NONE,
  /* '0' */ 6,            /* '1' */ 7,             /* '2' */ 8,
  /* '3' */ 9,            /* '4' */ 10,            /* '5' */ 11,
  /* '6' */ 12,           /* '7' */ 13,            /* '8' */ 14,
  /* '9' */ 15,           /* ':' */ RANK_NONE,     /* ';' */ RANK_NONE,
  /* '<' */ RANK_NONE,    /* '=' */ RANK_NONE,     /* '>' */ RANK_NONE,
  /* '?' */ RANK_NONE,    /* '@' */ RANK_NONE,     /* 'A' */ 16,
  /* 'B' */ 18,           /* 'C' */ 20,            /* 'D' */ 22,
  /* 'E' */ 24,           /* 'F' */ 26,            /* 'G' */ 28,
  /* 'H' */ 30,           /* 'I' */ 32,            /* 'J' */ 34,
  /* 'K' */ 36,           /* 'L' */ 38,            /* 'M' */ 40,
  /* 'N' */ 42,           /* 'O' */ 44,            /* 'P' */ 46,
  /* 'Q' */ 48,           /* 'R' */ 50,            /* 'S' */ 52,
  /* 'T' */ 54,           /* 'U' */ 56,            /* 'V' */ 58,
  /* 'W' */ 60,           /* 'X' */ 62,            /* 'Y' */ 64,
  /* 'Z' */ 66,           /* '[' */ RANK_NONE,     /* '\' */ RANK_NONE,
  /* ']' */ RANK_NONE,    /* '^' */ RANK_NONE,     /* '_' */ RANK_NONE,
  /* '`' */ RANK_NONE,    /* 'a' */ 17,            /* 'b' */ 19,
  /* 'c' */ 21,           /* 'd' */ 23,            /* 'e' */ 25,
  /* 'f' */ 27,           /* 'g' */ 29,            /* 'h' */ 31,
  /* 'i' */ 33,           /* 'j' */ 35,            /* 'k' */ 37,
  /* 'l' */ 39,           /* 'm' */ 41,            /* 'n' */ 43,
  /* 'o' */ 45,           /* 'p' */ 47,            /* 'q' */ 49,
  /* 'r' */ 51,           /* 's' */ 53,            /* 't' */ 55,
  /* 'u' */ 57,           /* 'v' */ 59,            /* 'w' */ 61,
  /* 'x' */ 63,           /* 'y' */ 65,            /* 'z' */ 67,
  /* '{' */ RANK_NONE,    /* '|' */ RANK_NONE,     /* '}' */ RANK_NONE,
  /* '~' */ RANK_NONE,    /* DEL */ RANK_NONE
}; /* end rank */


/* --------------------------------------------------------------------------
 * Forward declarations
 * ----------------------------------------------------------------------- */

static inline short int get_rank_of_first_ranking_char
  (const char *cstr, unsigned *index, cstr_collation_mode_t mode);

static inline short int collation_rank (char ch, cstr_collation_mode_t mode);


/* --------------------------------------------------------------------------
 * function cstr_length(cstr)
 * --------------------------------------------------------------------------
 * Returns the length of C string cstr.  Returns 0 if cstr is NULL.
 * ----------------------------------------------------------------------- */

unsigned cstr_length (const char *cstr) {
  unsigned length = 0;
  
  if (cstr == NULL) {
    return 0;
  } /* end if */
  
  while (cstr[length] != ASCII_NUL) {
    length++;
  } /* end while */
  
  return length;
} /* end cstr_length */


/* --------------------------------------------------------------------------
 * function cstr_last_char(cstr)
 * --------------------------------------------------------------------------
 * Returns last character of cstr, or ASCII NUL if cstr is NULL or empty.
 * ----------------------------------------------------------------------- */

char cstr_last_char (const char *cstr) {
  
  unsigned index = 0;
  
  if (cstr == NULL) {
    return ASCII_NUL;
  } /* end if */
  
  while (cstr[index] != ASCII_NUL) {
    index++;
  } /* end while */
  
  if (index > 0) {
    return cstr[index-1];
  }
  else {
    return ASCII_NUL;
  } /* end if */
} /* end cstr_last_char */


/* --------------------------------------------------------------------------
 * function cstr_contains_char(cstr, ch)
 * --------------------------------------------------------------------------
 * Returns true if cstr contains ch, otherwise false.
 * ----------------------------------------------------------------------- */

bool cstr_contains_char(const char *cstr, char ch) {
  unsigned index = 0;
  
  if (cstr == NULL) {
    return false;
  } /* end if */
  
  while (cstr[index] != ASCII_NUL) {
    if (cstr[index] == ch) {
      return true; 
    } /* end if */
    index++;
  } /* end for */
  
  return false;
} /* end cstr_contains_char */


/* --------------------------------------------------------------------------
 * function cstr_match(cstr1, cstr2)
 * --------------------------------------------------------------------------
 * Returns true if cstr1 and cstr2 match, otherwise false.
 * If any argument is NULL, false is returned.
 * ----------------------------------------------------------------------- */

bool cstr_match (const char *cstr1, const char *cstr2) {
  unsigned index = 0;
  
  if ((cstr1 == NULL) || (cstr2 == NULL)) {
    return false;
  } /* end if */
  
  while (cstr1[index] == cstr2[index]) {
    if (cstr1[index] == ASCII_NUL) {
      return true;
    } /* end if */
    index++;
  } /* end while */
  
  return false;
} /* end cstr_match */


/* --------------------------------------------------------------------------
 * function cstr_collation(cstr1, cstr2, mode)
 * --------------------------------------------------------------------------
 * Compares cstr1 and cstr2 using collation mode and returns a result code:
 *
 * o  COLLATION_PREDECESSOR if cstr1 precedes cstr2,
 * o  COLLATION_SUCCESSOR if cstr1 succeeds cstr2,
 * o  COLLATION_EQUIVALENT if cstr1 is equivalent to cstr2,
 * o  COLLATION_FAILURE if cstr1 or cstr2 is NULL.
 * ----------------------------------------------------------------------- */

cstr_collation_status_t cstr_collation
  (const char *cstr1, const char *cstr2, cstr_collation_mode_t mode) {
  
  unsigned index1 = 0, index2 = 0;
  short int rank1 = 0, rank2 = 0;
  
  if ((cstr1 == NULL) || (cstr2 == NULL)) {
    return COLLATION_FAILURE;
  } /* end if */
  
  while (rank1 == rank2) {
    rank1 = get_rank_of_first_ranking_char(cstr1, &index1, mode);
    rank2 = get_rank_of_first_ranking_char(cstr2, &index2, mode);
        
    /* exit if end of cstr1 or cstr2 reached */
    if ((cstr1[index1] == ASCII_NUL) || (cstr2[index2] == ASCII_NUL)) {
      break;
    } /* end if */
    
    /* next character */
    index1++;
    index2++;
  } /* end while */
  
  if (rank1 < rank2) {
    return COLLATION_PREDECESSOR;
  }
  else if (rank1 > rank2) {
    return COLLATION_SUCCESSOR;
  }
  else /* (rank1 == rank2) */ {
    return COLLATION_EQUIVALENT;
  } /* end if */
} /* end cstr_collation */


/* --------------------------------------------------------------------------
 * function new_cstr_from_slice(source, start_index, length)
 * --------------------------------------------------------------------------
 * Returns a newly allocated and NUL terminated C string containing length
 * characters from C string source starting at start_index.  Returns NULL
 * if source is NULL, if length is 0, if the length of source is less than
 * the required length (start_index + length) or if allocation fails.
 * ----------------------------------------------------------------------- */

const char *new_cstr_from_slice
  (const char *source, unsigned start_index, unsigned length) {
  
  char *target;
  unsigned reqlen, index;
  
  /* source must not be NULL, length must not be zero */
  if ((source == NULL) || (length == 0)) {
    return NULL;
  } /* end if */
  
  /* length of source must not be less than start_index + length */
  index = 0;
  reqlen = start_index + length;
  for (index = 0; index < reqlen; index++) {
    if (source[0] == ASCII_NUL) {
      return NULL;
    } /* end if */
  } /* end for */
  
  /* allocate target */
  target = malloc(length + 1);
  
  /* bail out if allocation fails */
  if (target == NULL) {
    return NULL;
  } /* end if */
  
  /* copy slice from source to target */
  for (index = 0; index < length; index++) {
    target[index] = source[start_index + index];
  } /* end for */
  
  /* terminate target */
  target[length] = ASCII_NUL;
  
  return (const char*) target;
} /* end new_cstr_from_slice */


/* --------------------------------------------------------------------------
 * function new_cstr_by_concat(cstr1, cstr2, ... cstrN)
 * --------------------------------------------------------------------------
 * Returns a newly allocated and NUL terminated C string containing the
 * concatenation of all its arguments in left-to-right order. The list of
 * arguments must be terminated by NULL.  Returns NULL if first is NULL.
 * ----------------------------------------------------------------------- */

typedef const char *const_char_ptr;

const char *new_cstr_by_concat (const char *first, ...) {
  unsigned reqlen, index, source_index;
  char *target;
  const char *cstr;
  va_list cstr_list;
  
  /* first must not be NULL */
  if (first == NULL) {
    return NULL;
  } /* end if */
  
  /* calculate required length for target */
  va_start(cstr_list, first);
  reqlen = cstr_length(first);
  /* get first string in list */
  cstr = va_arg(cstr_list, const_char_ptr);
  while (cstr != NULL) {
    /* add length to tally */
    reqlen = reqlen + cstr_length(cstr);
    
    /* get next string in list */
    cstr = va_arg(cstr_list, const_char_ptr);
  } /* end while */
  
  /* allocate target string */
  target = malloc(reqlen + 1);
  
  /* bail out it allocation fails */
  if (target == NULL) {
    return NULL;
  } /* end if */
  
  /* copy first to target */
  index = 0;
  while (first[index] != ASCII_NUL) {
    target[index] = first[index];
    index++;
  } /* end while */
   
  /* append strings from list to target */
  va_start(cstr_list, first);
  /* get first cstr in list */
  cstr = va_arg(cstr_list, const_char_ptr);
  while (cstr != NULL) {
    /* append cstr */
    source_index = 0;
    while (cstr[source_index] != ASCII_NUL) {
      target[index] = cstr[source_index];
      index++; source_index++;
    } /* end while */
    
    /* get next cstr in list */
    cstr = va_arg(cstr_list, const_char_ptr);
  } /* end while */
  va_end(cstr_list);
  
  /* terminate target */
  target[index] = ASCII_NUL;
  
  return (const char *) target;
} /* end new_cstr_by_concat */


/* *********************************************************************** *
 * Private Functions
 * *********************************************************************** */

/* --------------------------------------------------------------------------
 * private function get_rank_of_first_ranking_char(cstr, index, mode)
 * --------------------------------------------------------------------------
 * Returns the collation rank of the first ranking character in cstr,
 * starting at index, skipping all non-ranking characters and passing
 * the index of the first ranking character back in index.
 * ----------------------------------------------------------------------- */

static inline short int get_rank_of_first_ranking_char
  (const char *cstr, unsigned *index, cstr_collation_mode_t mode) {
  
  short int rank;
  char ch, lookahead;
  
  while (1) {
    ch = cstr[*index];
    lookahead = cstr[*index+1];
    rank = collation_rank(ch, mode);
    if ((rank == RANK_NONE) || ((ch == ' ') && (lookahead == ' '))) {
      (*index)++; /* skip */
    }
    else /* ranking char found */ {
      break;
    } /* end if */
  } /* end while */
    
  return rank;
} /* end get_rank_of_first_ranking_char */


/* --------------------------------------------------------------------------
 * private function collation_rank(ch, mode)
 * --------------------------------------------------------------------------
 * Returns the collation rank of ch for the given collation mode.
 * ----------------------------------------------------------------------- */

static inline short int collation_rank (char ch, cstr_collation_mode_t mode) {
  
#if ((CHAR_MIN < 0) || (CHAR_MAX > 127))
  if ((ch < 0) || (ch > 127)) {
    return RANK_NONE;
  } /* end if */
#endif
  
  switch (mode) {
    case COLLATION_MODE_DICT_LOWER :
      return dict_lower_rank[ch];
  
    case COLLATION_MODE_DICT_UPPER :
      return dict_upper_rank[ch];
  } /* end switch */
  
  /* default is ASCII */
  return (short int) ch;
} /* end collation_rank */


/* END OF FILE */