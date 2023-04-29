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
 * m2t-tokenset.h
 *
 * Public interface for M2T tokenset type.
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

#ifndef M2T_TOKENSET_H
#define M2T_TOKENSET_H

#include <stdbool.h>
#include "m2t-common.h"
#include "m2t-token.h"


/* --------------------------------------------------------------------------
 * type m2t_tokenset_t
 * --------------------------------------------------------------------------
 * Opaque pointer type representing a Modula-2 token-set object.
 * --------------------------------------------------------------------------
 */

typedef struct m2t_tokenset_opaque_t *m2t_tokenset_t;


/* --------------------------------------------------------------------------
 * function m2t_new_tokenset_from_list(token_list)
 * --------------------------------------------------------------------------
 * Returns a newly allocated tokenset object that includes the tokens passed
 * as arguments of a non-empty variadic argument list.  The argument list
 * must be explicitly terminated with 0.
 * ----------------------------------------------------------------------- */

m2t_tokenset_t m2t_new_tokenset_from_list (m2t_token_t first_token, ...);


/* --------------------------------------------------------------------------
 * function m2t_new_tokenset_from_union(set_list)
 * --------------------------------------------------------------------------
 * Returns a newly allocated tokenset object that represents the set union of
 * the tokensets passed as arguments of a non-empty variadic argument list.
 * The argument list must be explicitly terminated with NULL.
 * ----------------------------------------------------------------------- */

m2t_tokenset_t m2t_new_tokenset_from_union (m2t_tokenset_t first_set, ...);


/* --------------------------------------------------------------------------
 * function m2t_tokenset_element(set, token)
 * --------------------------------------------------------------------------
 * Returns true if token is an element of set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_tokenset_element (m2t_tokenset_t set, m2t_token_t token);


/* --------------------------------------------------------------------------
 * procedure m2t_tokenset_element_count(set)
 * --------------------------------------------------------------------------
 * Returns the number of elements in set.
 * ----------------------------------------------------------------------- */

uint_t m2t_tokenset_element_count (m2t_tokenset_t set);


/* --------------------------------------------------------------------------
 * function m2t_tokenset_subset(set, subset)
 * --------------------------------------------------------------------------
 * Returns true if each element in subset is also in set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_tokenset_subset (m2t_tokenset_t set, m2t_tokenset_t subset);


/* --------------------------------------------------------------------------
 * function m2t_tokenset_disjunct(set1, set2)
 * --------------------------------------------------------------------------
 * Returns true if set1 and set2 have no common elements, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_tokenset_disjunct (m2t_tokenset_t set1, m2t_tokenset_t set2);


/* --------------------------------------------------------------------------
 * procedure m2t_tokenset_print_set(set_name, set)
 * --------------------------------------------------------------------------
 * Prints a human readable representation of set.
 * Format: set_name = { comma-separated list of tokens };
 * ----------------------------------------------------------------------- */

void m2t_tokenset_print_set (const char *set_name, m2t_tokenset_t set);


/* --------------------------------------------------------------------------
 * procedure m2t_tokenset_print_list(set)
 * --------------------------------------------------------------------------
 * Prints a human readable list of symbols in set.
 * Format: first, second, third, ..., secondToLast or last
 * ----------------------------------------------------------------------- */

void m2t_tokenset_print_list (m2t_tokenset_t set);


/* --------------------------------------------------------------------------
 * procedure m2t_tokenset_print_literal_struct(ident)
 * --------------------------------------------------------------------------
 * Prints a struct definition for tokenset literals.
 * Format: struct ident { uint_t s0, s1, s2, ..., n };
 * ----------------------------------------------------------------------- */

void m2t_tokenset_print_literal_struct (const char *ident);


/* --------------------------------------------------------------------------
 * procedure m2t_tokenset_print_literal(set)
 * --------------------------------------------------------------------------
 * Prints a sequence of hex values representing the bit pattern of set.
 * Format: { 0xHHHHHHHH, 0xHHHHHHHH, ..., count };
 * ----------------------------------------------------------------------- */

void m2t_tokenset_print_literal (m2t_tokenset_t set);


/* --------------------------------------------------------------------------
 * procedure m2t_tokenset_release(set)
 * --------------------------------------------------------------------------
 * Deallocates tokenset set.
 * ----------------------------------------------------------------------- */

void m2t_tokenset_release (m2t_tokenset_t set);


#endif /* M2t_TOKENSET_H */

/* END OF FILE */