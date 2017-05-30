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
 * m2-resync-sets.c
 *
 * Implementation of M2C resync set lookup.
 *
 * @license
 *
 * M2C is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation;  either version 2 of the License (GPL2),
 * or (at your option) any later version.
 *
 * M2C is distributed in the hope that it will be useful,  but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with m2c.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "m2-resync-sets.h"
#include "m2-resync-set-inits.h"

#include <stddef.h>


/* --------------------------------------------------------------------------
 * RESYNC set data structures
 * ----------------------------------------------------------------------- */

m2c_tokenset_literal_t
  rs_import_or_definition_or_end =
    INIT_SKIP_TO_IMPORT_OR_DEFINITON_OR_END,
  rs_import_or_ident_or_semicolon =
    INIT_SKIP_TO_IMPORT_OR_IDENT_OR_SEMICOLON,
  rs_ident_or_semicolon =
    INIT_SKIP_TO_IDENT_OR_SEMICOLON,
  rs_comma_or_semicolon =
    INIT_SKIP_TO_COMMA_OR_SEMICOLON,
  rs_definition_or_ident_or_semicolon =
    INIT_SKIP_TO_DEFINITION_OR_IDENT_OR_SEMICOLON,
  rs_definition_or_semicolon =
    INIT_SKIP_TO_DEFINITION_OR_SEMICOLON,
  rs_type_or_comma_or_of =
    INIT_SKIP_TO_TYPE_OR_COMMA_OR_OF,
  rs_semicolon_or_end =
    INIT_SKIP_TO_SEMICOLON_OR_END,
  rs_else_or_end =
    INIT_SKIP_TO_ELSE_OR_END,
  rs_comma_or_right_paren =
    INIT_SKIP_TO_COMMA_OR_RIGHT_PAREN,
  rs_colon_or_semicolon =
    INIT_SKIP_TO_COLON_OR_SEMICOLON,
  rs_import_or_block =
    INIT_SKIP_TO_IMPORT_OR_BLOCK,
  rs_declaration_or_ident_or_semicolon =
    INIT_SKIP_TO_DECLARATION_OR_IDENT_OR_SEMICOLON,
  rs_declaration_or_semicolon =
    INIT_SKIP_TO_DECLARATION_OR_SEMICOLON,
  rs_first_or_follow_of_statement =
    INIT_SKIP_TO_FIRST_OR_FOLLOW_OF_STATEMENT,
  rs_elsif_or_else_or_end =
    INIT_SKIP_TO_ELSIF_OR_ELSE_OR_END,
  rs_for_loop_body =
    INIT_SKIP_TO_FOR_LOOP_BODY;


/* --------------------------------------------------------------------------
 * private variable m2c_resync_set
 * --------------------------------------------------------------------------
 * Table of pointers to RESYNC set data structures
 * ----------------------------------------------------------------------- */

m2c_tokenset_literal_t *m2c_resync_set[] = {
  &rs_import_or_definition_or_end,
  &rs_import_or_ident_or_semicolon,
  &rs_ident_or_semicolon,
  &rs_comma_or_semicolon,
  &rs_definition_or_ident_or_semicolon,
  &rs_definition_or_semicolon,
  &rs_type_or_comma_or_of,
  &rs_semicolon_or_end,
  &rs_else_or_end,
  &rs_comma_or_right_paren,
  &rs_colon_or_semicolon,
  &rs_import_or_block,
  &rs_declaration_or_ident_or_semicolon,
  &rs_declaration_or_semicolon,
  &rs_first_or_follow_of_statement,
  &rs_elsif_or_else_or_end,
  &rs_for_loop_body
}; /* end m2c_resync_set */


/* --------------------------------------------------------------------------
 * private variable m2c_resync_set_name_table
 * --------------------------------------------------------------------------
 * Table of pointers to human readable resync set names
 * ----------------------------------------------------------------------- */

const char *m2c_resync_set_name_table[] = {
  "IMPORT_OR_DEFINITON_OR_END\0",
  "IMPORT_OR_IDENT_OR_SEMICOLON\0",
  "IDENT_OR_SEMICOLON\0",
  "COMMA_OR_SEMICOLON\0",
  "DEFINITION_OR_IDENT_OR_SEMICOLON\0",
  "DEFINITION_OR_SEMICOLON\0",
  "TYPE_OR_COMMA_OR_OF\0",
  "SEMICOLON_OR_END\0",
  "ELSE_OR_END\0",
  "COMMA_OR_RIGHT_PAREN\0",
  "COLON_OR_SEMICOLON\0",
  "IMPORT_OR_BLOCK\0",
  "DECLARATION_OR_IDENT_OR_SEMICOLON\0",
  "DECLARATION_OR_SEMICOLON\0",
  "FIRST_OR_FOLLOW_OF_STATEMENT\0",
  "ELSIF_OR_ELSE_OR_END\0",
  "FOR_LOOP_BODY\0"
}; /* end m2c_resync_set_name_table */


/* --------------------------------------------------------------------------
 * function m2c_is_valid_resync_set(rs)
 * --------------------------------------------------------------------------
 * Returns TRUE if rs represents a resync set, otherwise FALSE.
 * ----------------------------------------------------------------------- */

#define IS_VALID_RESYNC_SET(_rs) \
  ((_rs) < RESYNC_END_MARK)

#define IS_INVALID_RESYNC_SET(_rs) \
  ((_rs) >= RESYNC_END_MARK)

inline bool m2c_is_valid_resync_set (m2c_resync_enum_t rs) {
  return IS_VALID_RESYNC_SET(rs);
} /* end m2c_is_valid_resync_set */


/* --------------------------------------------------------------------------
 * function RESYNC(rs)
 * --------------------------------------------------------------------------
 * Returns a tokenset with resync set rs.
 * ----------------------------------------------------------------------- */

m2c_tokenset_t RESYNC (m2c_resync_enum_t rs) {
  if (IS_INVALID_RESYNC_SET(rs)) {
    return NULL;
  } /* end if */
  
  return (m2c_tokenset_t) m2c_resync_set[rs];
} /* end RESYNC */


/* --------------------------------------------------------------------------
 * function m2c_name_for_resync_set(rs)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to a NUL terminated character string with
 * a human readable name for rescync set rs.  Returns NULL if rs is invalid.
 * ----------------------------------------------------------------------- */

const char *m2c_name_for_resync_set (m2c_resync_enum_t rs) {
  if (IS_INVALID_RESYNC_SET(rs)) {
    return NULL;
  } /* end if */
  
  return m2c_resync_set_name_table[rs];
} /* end m2c_name_for_resync_set */

/* END OF FILE */