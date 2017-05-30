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
 * m2-resync-sets.h
 *
 * Public interface for M2C resync set lookup.
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

#ifndef M2C_RESYNC_SETS_H
#define M2C_RESYNC_SETS_H

#include "m2-common.h"
#include "m2-tokenset.h"

#include <stdbool.h>


/* --------------------------------------------------------------------------
 * type m2c_resync_index_t
 * --------------------------------------------------------------------------
 * Enumerated production values representing Modula-2 non-terminal symbols.
 * ----------------------------------------------------------------------- */

typedef enum {
  /* Enumerated Resynchronisation Sets */

  IMPORT_OR_DEFINITON_OR_END,
  IMPORT_OR_IDENT_OR_SEMICOLON,
  IDENT_OR_SEMICOLON,
  COMMA_OR_SEMICOLON,
  DEFINITION_OR_IDENT_OR_SEMICOLON,
  DEFINITION_OR_SEMICOLON,
  TYPE_OR_COMMA_OR_OF,
  SEMICOLON_OR_END,
  ELSE_OR_END,
  COMMA_OR_RIGHT_PAREN,
  COLON_OR_SEMICOLON,
  IMPORT_OR_BLOCK,
  DECLARATION_OR_IDENT_OR_SEMICOLON,
  DECLARATION_OR_SEMICOLON,
  FIRST_OR_FOLLOW_OF_STATEMENT,
  ELSIF_OR_ELSE_OR_END,
  FOR_LOOP_BODY,
  
  /* Enumeration Terminator */
  
  RESYNC_END_MARK /* marks the end of this enumeration */
} m2c_resync_enum_t;


/* --------------------------------------------------------------------------
 * Constant M2C_RESYNC_COUNT -- number of resync sets
 * ----------------------------------------------------------------------- */

#define M2C_RESYNC_SET_COUNT RESYNC_END_MARK


/* --------------------------------------------------------------------------
 * function m2c_is_valid_resync_set(rs)
 * --------------------------------------------------------------------------
 * Returns TRUE if rs represents a resync set, otherwise FALSE.
 * ----------------------------------------------------------------------- */

bool m2c_is_valid_resync_set (m2c_resync_enum_t rs);


/* --------------------------------------------------------------------------
 * function RESYNC(rs)
 * --------------------------------------------------------------------------
 * Returns a tokenset with resync set rs.
 * ----------------------------------------------------------------------- */

m2c_tokenset_t RESYNC (m2c_resync_enum_t rs);


/* --------------------------------------------------------------------------
 * function m2c_name_for_resync_set(rs)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to a NUL terminated character string with
 * a human readable name for rescync set rs.  Returns NULL if rs is invalid.
 * ----------------------------------------------------------------------- */

const char *m2c_name_for_resync_set (m2c_resync_enum_t rs);

#endif /* M2C_RESYNC_SETS_H */

/* END OF FILE */