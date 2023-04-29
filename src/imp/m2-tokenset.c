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
 * m2-tokenset.c
 *
 * Implementation of M2C tokenset type.
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

#include "m2-tokenset.h"

#include "m2-first-set-inits.h"
#include "m2-follow-set-inits.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


/* --------------------------------------------------------------------------
 * constant M2C_TOKENSET_SEGMENT_COUNT
 * --------------------------------------------------------------------------
 * Number of 32 bit segments in a tokenset.
 * ----------------------------------------------------------------------- */

#define M2C_TOKENSET_SEGMENT_COUNT ((TOKEN_END_MARK / 32) + 1)


/* --------------------------------------------------------------------------
 * hidden type m2c_tokenset_opaque_t
 * --------------------------------------------------------------------------
 * Opaque record type representing a Modula-2 tokenset object,
 * implemented as an array of 32-bit segments.
 * ----------------------------------------------------------------------- */

struct m2c_tokenset_opaque_t {
   /* segment */ uint32_t segment[M2C_TOKENSET_SEGMENT_COUNT];
   /* elem_count */ uint_t elem_count;
};

typedef struct m2c_tokenset_opaque_t m2c_tokenset_opaque_t;


/* --------------------------------------------------------------------------
 * function m2c_new_tokenset_from_list(token_list)
 * --------------------------------------------------------------------------
 * Returns a newly allocated tokenset object that includes the tokens passed
 * as arguments of a non-empty variadic argument list.  The argument list
 * must be explicitly terminated with 0.
 * ----------------------------------------------------------------------- */

uint_t count_bits_in_set (m2c_tokenset_t set);

m2c_tokenset_t m2c_new_tokenset_from_list (m2c_token_t first_token, ...) {
  m2c_tokenset_t new_set;
  uint_t bit, seg_index;
  m2c_token_t token;
  
  va_list token_list;
  va_start(token_list, first_token);
  
  /* allocate new set */
  new_set = malloc(sizeof(m2c_tokenset_opaque_t));
  
  /* bail out if allocation failed */
  if (new_set == NULL) {
    return NULL;
  } /* end if */
  
  /* initialise */
  seg_index = 0;
  while (seg_index < M2C_TOKENSET_SEGMENT_COUNT) {
    new_set->segment[seg_index] = 0;
    seg_index++;
  } /* end while */
  
  /* store tokens from list */
  token = first_token;
  while (token != 0) {
  
    /* store token in set if in range */
    if (token < TOKEN_END_MARK) {
      seg_index = token / 32;
      bit = token % 32;
      new_set->segment[seg_index] = new_set->segment[seg_index] | (1 << bit);
    } /* end if */
    
    /* get next token in list */
    token = va_arg(token_list, m2c_token_t);
  } /* end while */
  
  /* update element counter */
  new_set->elem_count = count_bits_in_set(new_set);
  
  return new_set;
} /* end m2c_new_tokenset_from_list */


/* --------------------------------------------------------------------------
 * function m2c_new_tokenset_from_union(set_list)
 * --------------------------------------------------------------------------
 * Returns a newly allocated tokenset object that represents the set union of
 * the tokensets passed as arguments of a non-empty variadic argument list.
 * The argument list must be explicitly terminated with NULL.
 * ----------------------------------------------------------------------- */

m2c_tokenset_t m2c_new_tokenset_from_union (m2c_tokenset_t first_set, ...) {
  m2c_tokenset_t new_set;
  uint_t seg_index;
  m2c_tokenset_t set;
  
  va_list set_list;
  va_start(set_list, first_set);
  
  /* allocate new set */
  new_set = malloc(sizeof(m2c_tokenset_opaque_t));
  
  /* bail out if allocation failed */
  if (new_set == NULL) {
    return NULL;
  } /* end if */
  
  /* initialise */
  seg_index = 0;
  while (seg_index < M2C_TOKENSET_SEGMENT_COUNT) {
    new_set->segment[seg_index] = 0;
    seg_index++;
  } /* end while */
  
  set = first_set;
  /* calculate union with each set in list */
  while (set != NULL) {
    /* for each segment ... */
    while (seg_index < M2C_TOKENSET_SEGMENT_COUNT) {
      /* ... store union of corresponding segments */
      new_set->segment[seg_index] =
        new_set->segment[seg_index] | set->segment[seg_index];
      
      /* next segment */
      seg_index++;
    } /* end while */
    
    /* get next set in list */
    set = va_arg(set_list, m2c_tokenset_t);
  } /* end while */
  
  /* update element counter */
  new_set->elem_count = count_bits_in_set(new_set);
  
  return new_set;
} /* m2c_new_tokenset_from_union */


/* --------------------------------------------------------------------------
 * function m2c_tokenset_element(set, token)
 * --------------------------------------------------------------------------
 * Returns true if token is an element of set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_tokenset_element (m2c_tokenset_t set, m2c_token_t token) {
  uint_t bit, seg_index;
  
  if (token >= TOKEN_END_MARK) {
    return false;
  } /* end if */
  
  seg_index = token / 32;
  bit = token % 32;
  
  return ((set->segment[seg_index] & (1 << bit)) != 0);
} /* end m2c_tokenset_element */


/* --------------------------------------------------------------------------
 * procedure m2c_tokenset_element_count(set)
 * --------------------------------------------------------------------------
 * Returns the number of elements in set.
 * ----------------------------------------------------------------------- */

uint_t m2c_tokenset_element_count (m2c_tokenset_t set) {  
  if (set == NULL) {
    return 0;
  } /* end if */
  
  return set->elem_count;
} /* end m2c_tokenset_element_count */


/* --------------------------------------------------------------------------
 * function m2c_tokenset_subset (set, subset)
 * --------------------------------------------------------------------------
 * Returns true if each element in subset is also in set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_tokenset_subset (m2c_tokenset_t set, m2c_tokenset_t subset) {
  uint_t seg_index;
  
  seg_index = 0;
  while (seg_index < M2C_TOKENSET_SEGMENT_COUNT) {
    if (((set->segment[seg_index] & subset->segment[seg_index]) ^
        subset->segment[seg_index]) == 0) {
      seg_index++;
    }
    else {
      return false;
    } /* end if */
  } /* end while */
  
  return true;
} /* end m2c_tokenset_subset */


/* --------------------------------------------------------------------------
 * function m2c_tokenset_disjunct(set1, set2)
 * --------------------------------------------------------------------------
 * Returns true if set1 and set2 have no common elements, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_tokenset_disjunct (m2c_tokenset_t set1, m2c_tokenset_t set2) {
  uint_t seg_index;
  
  seg_index = 0;
  while (seg_index < M2C_TOKENSET_SEGMENT_COUNT) {
    if ((set1->segment[seg_index] & set2->segment[seg_index]) == 0) {
      seg_index++;
    }
    else {
      return false;
    } /* end if */
  } /* end while */
  
  return true;
} /* end m2c_tokenset_disjunct */


/* --------------------------------------------------------------------------
 * procedure m2c_tokenset_print_set(set_name, set)
 * --------------------------------------------------------------------------
 * Prints a human readable representation of set.
 * Format: set_name = { comma-separated list of tokens };
 * ----------------------------------------------------------------------- */

void m2c_tokenset_print_set (const char *set_name, m2c_tokenset_t set) {
  uint_t bit, seg_index, count;
  m2c_token_t token;
  
  printf("%s = {", set_name);
  
  if (set->elem_count == 0) {
    printf(" ");
  } /* end if */
  
  count = 0;
  token = 0;
  while ((count <= set->elem_count) && (token < TOKEN_END_MARK)) {
    seg_index = token / 32;
    bit = token % 32;
    if ((set->segment[seg_index] & (1 << bit)) != 0) {
      count++;
      if (count < set->elem_count) {
        printf("\n  %s,", m2c_name_for_token(token));
      }
      else {
        printf("\n  %s\n", m2c_name_for_token(token));
      } /* end if */
    } /* end if */
    token++;
  } /* end while */
  
  printf("};\n");
} /* m2c_tokenset_print_set */


/* --------------------------------------------------------------------------
 * procedure m2c_tokenset_print_list(set)
 * --------------------------------------------------------------------------
 * Prints a human readable list of symbols in set.
 * Format: first, second, third, ..., secondToLast or last
 * ----------------------------------------------------------------------- */

void m2c_tokenset_print_list (m2c_tokenset_t set) {
  uint_t bit, seg_index, count;
  m2c_token_t token;
  
  if (set->elem_count == 0) {
    printf("(nil)");
  } /* end if */
  
  count = 0;
  token = 0;
  while ((count <= set->elem_count) && (token < TOKEN_END_MARK)) {
    seg_index = token / 32;
    bit = token % 32;
    
    if ((set->segment[seg_index] & (1 << bit)) != 0) {
      count++;
      if (count > 1) {
        if (count < set->elem_count) {
          printf(", ");
        }
        else {
          printf(" or ");
        } /* end if */
      } /* end if */
      
      if (token == TOKEN_IDENTIFIER) {
        printf("identifier");
      }
      else if (token == TOKEN_STRING) {
        printf("string");
      }
      else if (token == TOKEN_INTEGER) {
        printf("integer");
      }
      else if (token == TOKEN_REAL) {
        printf("real number");
      }
      else if (token == TOKEN_CHAR) {
        printf("character code");
      }
      else if (m2c_is_resword_token(token)) {
        printf("%s", m2c_lexeme_for_resword(token));
      }
      else if (m2c_is_special_symbol_token(token)) {
        printf("'%s'", m2c_lexeme_for_special_symbol(token));
      }
      else if (token == TOKEN_END_OF_FILE) {
        printf("<EOF>");
      } /* end if */
    } /* end if */
    token++;
  } /* end while */
  
  printf(".\n");
} /* m2c_tokenset_print_list */


/* --------------------------------------------------------------------------
 * procedure m2c_tokenset_print_literal_struct(ident)
 * --------------------------------------------------------------------------
 * Prints a struct definition for tokenset literals.
 * Format: struct ident { uint_t s0, s1, s2, ..., n };
 * ----------------------------------------------------------------------- */

void m2c_tokenset_print_literal_struct (const char *ident) {
  uint_t seg_index;
  
  printf("struct %s { uint_t s0", ident);
  
  seg_index = 1;
  while (seg_index < M2C_TOKENSET_SEGMENT_COUNT) {
    printf(", s%u", seg_index);
    seg_index++;
  } /* end while */
  
  printf(", n; };\n");
  
  printf("typedef struct %s %s;\n", ident, ident);
} /* m2c_tokenset_print_literal_struct */


/* --------------------------------------------------------------------------
 * procedure m2c_tokenset_print_literal(set)
 * --------------------------------------------------------------------------
 * Prints a sequence of hex values representing the bit pattern of set.
 * Format: { 0xHHHHHHHH, 0xHHHHHHHH, ..., count };
 * ----------------------------------------------------------------------- */

void m2c_tokenset_print_literal (m2c_tokenset_t set) {
  uint_t seg_index;
  
  printf("{ /* bits: */ 0x%08X", set->segment[0]);
  
  seg_index = 1;
  while (seg_index < M2C_TOKENSET_SEGMENT_COUNT) {
    printf(", 0x%08X", set->segment[seg_index]);
    seg_index++;
  } /* end while */
  
  printf(", /* counter: */ %u }\n", set->elem_count);
} /* m2c_tokenset_print_literal */


/* --------------------------------------------------------------------------
 * procedure m2c_tokenset_release(set)
 * --------------------------------------------------------------------------
 * Deallocates tokenset set.
 * ----------------------------------------------------------------------- */

void m2c_tokenset_release (m2c_tokenset_t set) {
  if (set != NULL) {
    free(set);
  } /* end if */
} /* end m2c_tokenset_release */


/* --------------------------------------------------------------------------
 * private function count_bits_in_set(set)
 * --------------------------------------------------------------------------
 * Returns the number of set bits in set.
 * ----------------------------------------------------------------------- */

uint_t count_bits_in_set (m2c_tokenset_t set) {
  uint_t bit, seg_index, bit_count;
  
  bit_count = 0;
  seg_index = 0;
  
  while (seg_index < M2C_TOKENSET_SEGMENT_COUNT) {
    bit = 0;
    while (bit < 32) {
      if ((set->segment[seg_index] & (1 << bit)) != 0) {
        bit_count++;
      } /* end if */
      bit++;
    } /* end while */
    seg_index++;
  } /* end while */
  
  return bit_count;
} /* count_bits_in_set */

/* END OF FILE */