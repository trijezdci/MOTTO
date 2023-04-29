/* M2C Modula-2 Compiler & Translator
 * Copyright (c) 2015, 2016 Benjamin Kowarsch
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
 * m2-identconv.h
 *
 * Public interface for identifier conversion.
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

#include "m2-identconv.h"

#include "cstring.h"
#include "c_reswords.h"

#include <stddef.h>
#include <limits.h>

#include <stdio.h>

static bool cmode = false;

void set_cstyle_option (void) {
  cmode = true;
} /* end set_cstyle_option */

bool m2c_option_cstyle_output (void) {
  return cmode;
} /* end m2c_option_cstyle_output */


/* --------------------------------------------------------------------------
 * function get_include_guard_ident(target, module_id)
 * --------------------------------------------------------------------------
 * Composes a C include guard macro identifier from module_id and passes it
 * as a NUL terminated C string in out-parameter target.
 *
 * includeGuard :=
 *   vModeIncludeGuard | cModeIncludeGuard
 *   ;
 *
 * Returns length of composed identifier on success.
 * Fails and returns zero if module_id is NULL or empty.
 * ----------------------------------------------------------------------- */

static uint_t get_vmode_include_guard_id
  (char_ptr_t *target, const char *module_id, uint_t limit);

static uint_t get_cmode_include_guard_id
  (char_ptr_t *target, const char *module_id, uint_t limit);

uint_t get_include_guard_ident (char_ptr_t *target, const char *module_id) {
  
  uint_t length;
  uint_t limit = M2C_MAX_C_MACRO_LENGTH;
  
  /* module_id must not be NULL nor empty */
  if ((module_id == NULL) || (CSTR_FIRST_CHAR(module_id)) == ASCII_NUL) {
    return 0;
  } /* end if */
  
  /* cModeIncludeGuard */
  if (m2c_option_cstyle_output()) {
    length = get_cmode_include_guard_id(target, module_id, limit);
  }
  /* | vModeIncludeGuard */
  else {
    length = get_vmode_include_guard_id(target, module_id, limit);
  } /* end if */
  
  return length;
} /* end get_include_guard_ident */


/* --------------------------------------------------------------------------
 * function get_public_const_ident(target, module_id, ident)
 * --------------------------------------------------------------------------
 * Composes a qualified public constant identifier from module_id and ident,
 * and passes it as a NUL terminated C string in out-parameter target.
 *
 * publicConstIdent :=
 *   vModePublicIdent | cModePublicConstIdent
 *   ;
 *
 * Returns length of composed identifier on success.
 * Fails and returns zero if ident is NULL or empty.
 * ----------------------------------------------------------------------- */

static uint_t get_vmode_public_id
  (char_ptr_t *target, const char *module_id, const char *id, uint_t limit);

static uint_t get_cmode_public_const_id
  (char_ptr_t *target, const char *module_id, const char *id, uint_t limit);

uint_t get_public_const_ident
  (char_ptr_t *target, const char *module_id, const char *ident) {
  
  uint_t length;
  uint_t limit = M2C_MAX_C_MACRO_LENGTH;
  
  /* module_id must not be NULL nor empty */
  if ((module_id == NULL) || (CSTR_FIRST_CHAR(module_id)) == ASCII_NUL) {
    return 0;
  } /* end if */
  
  /* ident must not be NULL nor empty */
  if ((ident == NULL) || (CSTR_FIRST_CHAR(ident)) == ASCII_NUL) {
    return 0;
  } /* end if */
  
  if (m2c_option_cstyle_output()) {
    length = get_cmode_public_const_id(target, module_id, ident, limit);
  }
  else /* verbatim output mode */ {
    length = get_vmode_public_id(target, module_id, ident, limit);
  } /* end if */
  
  return length;
} /* end get_public_const_ident */


/* --------------------------------------------------------------------------
 * function get_public_type_ident(target, module_id, ident)
 * --------------------------------------------------------------------------
 * Composes a qualified public type identifier from module_id and ident,
 * and passes it as a NUL terminated C string in out-parameter target.
 *
 * publicTypeIdent :=
 *   vModePublicIdent | cModePublicTypeIdent
 *   ;
 *
 * Returns length of composed identifier on success.
 * Fails and returns zero if ident is NULL or empty.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_public_type_id
  (char_ptr_t *target, const char *module_id, const char *id, uint_t limit);

uint_t get_public_type_ident
  (char_ptr_t *target, const char *module_id, const char *ident) {
  
  uint_t length;
  uint_t limit = M2C_MAX_C_IDENT_LENGTH;
  
  /* module_id must not be NULL nor empty */
  if ((module_id == NULL) || (CSTR_FIRST_CHAR(module_id) == ASCII_NUL)) {
    return 0;
  } /* end if */
  
  /* ident must not be NULL nor empty */
  if ((ident == NULL) || (CSTR_FIRST_CHAR(ident) == ASCII_NUL)) {
    return 0;
  } /* end if */
  
  if (m2c_option_cstyle_output()) {
    length = get_cmode_public_type_id(target, module_id, ident, limit);
  }
  else /* verbatim output mode */ {
    length = get_vmode_public_id(target, module_id, ident, limit);
  } /* end if */
  
  return length;
} /* end get_public_type_ident */


/* --------------------------------------------------------------------------
 * function get_public_vfn_ident(target, module_id, ident)
 * --------------------------------------------------------------------------
 * Composes a qualified public variable or function identifier from module_id
 * and ident, passes it as a NUL terminated C string in out-parameter target.
 *
 * publicVarOrFnIdent :=
 *   vModePublicIdent | cModePublicVarOrFnIdent
 *   ;
 *
 * Returns length of composed identifier on success.
 * Fails and returns zero if ident is NULL or empty.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_public_vfn_id
  (char_ptr_t *target, const char *module_id, const char *id, uint_t limit);

uint_t get_public_vfn_ident
  (char_ptr_t *target, const char *module_id, const char *ident) {
  
  uint_t length;
  uint_t limit = M2C_MAX_C_IDENT_LENGTH;
  
  /* module_id must not be NULL nor empty */
  if ((module_id == NULL) || (CSTR_FIRST_CHAR(module_id) == ASCII_NUL)) {
    return 0;
  } /* end if */
  
  /* ident must not be NULL nor empty */
  if ((ident == NULL) || (CSTR_FIRST_CHAR(ident) == ASCII_NUL)) {
    return 0;
  } /* end if */
  
  if (m2c_option_cstyle_output()) {
    length = get_cmode_public_vfn_id(target, module_id, ident, limit);
  }
  else /* verbatim output mode */ {
    length = get_vmode_public_id(target, module_id, ident, limit);
  } /* end if */
  
  return length;
} /* end get_public_vfn_ident */


/* --------------------------------------------------------------------------
 * function get_private_const_ident(target, ident)
 * --------------------------------------------------------------------------
 * Composes a qualified private constant identifier from ident and passes
 * it as a NUL terminated C string in out-parameter target.
 *
 * privateConstIdent :=
 *   vModePrivateIdent | cModePrivateConstIdent
 *   ;
 *
 * Returns length of composed identifier on success.
 * Fails and returns zero if ident is NULL or empty.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_private_const_id
  (char_ptr_t *target, const char *id, uint_t limit);

static uint_t get_vmode_private_id
  (char_ptr_t *target, const char *id, uint_t limit);

uint_t get_private_const_ident (char_ptr_t *target, const char *ident) {
  
  uint_t length;
  uint_t limit = M2C_MAX_C_MACRO_LENGTH;
  
  /* ident must not be NULL nor empty */
  if ((ident == NULL) || (CSTR_FIRST_CHAR(ident) == ASCII_NUL)) {
    return 0;
  } /* end if */
  
  if (m2c_option_cstyle_output()) {
    length = get_cmode_private_const_id(target, ident, limit);
  }
  else /* verbatim output mode */ {
    length = get_vmode_private_id(target, ident, limit);
  } /* end if */
  
  return length;
} /* end get_private_const_ident */


/* --------------------------------------------------------------------------
 * function get_private_type_ident(target, ident)
 * --------------------------------------------------------------------------
 * Composes a qualified private type identifier from ident and passes it as
 * a NUL terminated C string in out-parameter target.
 *
 * privateTypeIdent :=
 *   vModePrivateIdent | cModePrivateTypeIdent
 *   ;
 *
 * Returns length of composed identifier on success.
 * Fails and returns zero if ident is NULL or empty.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_private_type_id
  (char_ptr_t *target, const char *id, uint_t limit);

uint_t get_private_type_ident (char_ptr_t *target, const char *ident) {
  
  uint_t length;
  uint_t limit = M2C_MAX_C_IDENT_LENGTH;
  
  /* ident must not be NULL nor empty */
  if ((ident == NULL) || (CSTR_FIRST_CHAR(ident) == ASCII_NUL)) {
    return 0;
  } /* end if */
  
  if (m2c_option_cstyle_output()) {
    length = get_cmode_private_type_id(target, ident, limit);
  }
  else /* verbatim output mode */ {
    length = get_vmode_private_id(target, ident, limit);
  } /* end if */
  
  return length;
} /* end get_private_type_ident */


/* --------------------------------------------------------------------------
 * function get_private_vfn_ident(target, ident)
 * --------------------------------------------------------------------------
 * Composes a qualified private variable or function identifier from ident
 * and passes it as a NUL terminated C string in out-parameter target.
 *
 * privateVarOrFnIdent :=
 *   vModePrivateIdent | cModePrivateVarOrFnIdent
 *   ;
 *
 * Returns length of composed identifier on success.
 * Fails and returns zero if ident is NULL or empty.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_private_vfn_id
  (char_ptr_t *target, const char *id, uint_t limit);

uint_t get_private_vfn_ident (char_ptr_t *target, const char *ident) {
  
  uint_t length;
  uint_t limit = M2C_MAX_C_IDENT_LENGTH;
  
  /* ident must not be NULL nor empty */
  if ((ident == NULL) || (CSTR_FIRST_CHAR(ident) == ASCII_NUL)) {
    return 0;
  } /* end if */
  
  if (m2c_option_cstyle_output()) {
    length = get_cmode_private_vfn_id(target, ident, limit);
  }
  else /* verbatim output mode */ {
    length = get_vmode_private_id(target, ident, limit);
  } /* end if */
  
  return length;
} /* end get_private_vfn_ident */


/* --------------------------------------------------------------------------
 * function get_local_const_ident(target, outer, ordinal, ident)
 * --------------------------------------------------------------------------
 * Composes a qualified local constant identifier from outer or ordinal and
 * ident, passes it as a NUL terminated C string in out-parameter target.
 *
 * localConstIdent :=
 *   vModeLocalIdent | cModeLocalConstIdent
 *   ;
 *
 * Returns length of composed identifier on success.
 * Fails and returns zero if ident is NULL or empty.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_local_const_id
  (char_ptr_t *target,
   const char *outer, uint_t ordinal, const char *id, uint_t limit);

static uint_t get_vmode_local_id
  (char_ptr_t *target,
   const char *outer, uint_t ordinal, const char *id, uint_t limit);

uint_t get_local_const_ident
  (char_ptr_t *target, const char *outer, uint_t ordinal, const char *ident) {
  
  uint_t length;
  uint_t limit = M2C_MAX_C_MACRO_LENGTH;
  
  /* ident must not be NULL nor empty */
  if ((ident == NULL) || (CSTR_FIRST_CHAR(ident) == ASCII_NUL)) {
    return 0;
  } /* end if */
  
  if (m2c_option_cstyle_output()) {
    length = get_cmode_local_const_id(target, outer, ordinal, ident, limit);
  }
  else /* verbatim output mode */ {
    length = get_vmode_local_id(target, outer, ordinal, ident, limit);
  } /* end if */
  
  return length;
} /* end get_local_const_ident */


/* --------------------------------------------------------------------------
 * function get_local_type_ident(target, outer, ordinal, ident)
 * --------------------------------------------------------------------------
 * Composes a qualified local type identifier from outer or ordinal and
 * ident, passes it as a NUL terminated C string in out-parameter target.
 *
 * localTypeIdent :=
 *   vModeLocalIdent | cModeLocalTypeIdent
 *   ;
 *
 * Returns length of composed identifier on success.
 * Fails and returns zero if ident is NULL or empty.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_local_type_id
  (char_ptr_t *target,
   const char *outer, uint_t ordinal, const char *id, uint_t limit);

uint_t get_local_type_ident
  (char_ptr_t *target, const char *outer, uint_t ordinal, const char *ident) {
  
  uint_t length;
  uint_t limit = M2C_MAX_C_IDENT_LENGTH;
  
  /* ident must not be NULL nor empty */
  if ((ident == NULL) || (CSTR_FIRST_CHAR(ident) == ASCII_NUL)) {
    return 0;
  } /* end if */
  
  if (m2c_option_cstyle_output()) {
    length = get_cmode_local_type_id(target, outer, ordinal, ident, limit);
  }
  else /* verbatim output mode */ {
    length = get_vmode_local_id(target, outer, ordinal, ident, limit);
  } /* end if */
  
  return length;
} /* end get_local_type_ident */


/* --------------------------------------------------------------------------
 * function get_local_var_ident(target, ident)
 * --------------------------------------------------------------------------
 * Composes a disambiguated local variable identifier from ident,
 * passes it as a NUL terminated C string in out-parameter target.
 *
 * localVarIdent :=
 *   vModeLocalVarIdent | cModeLocalVarIdent
 *   ;
 *
 * Returns length of composed identifier on success.
 * Fails and returns zero if ident is NULL or empty.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_local_var_id
  (char_ptr_t *target, const char *id, uint_t limit);

static uint_t get_vmode_local_var_id
  (char_ptr_t *target, const char *id, uint_t limit);

uint_t get_local_var_ident (char_ptr_t *target, const char *ident) {
  
  uint_t length;
  uint_t limit = M2C_MAX_C_IDENT_LENGTH;
  
  /* ident must not be NULL nor empty */
  if ((ident == NULL) || (CSTR_FIRST_CHAR(ident) == ASCII_NUL)) {
    return 0;
  } /* end if */
  
  if (m2c_option_cstyle_output()) {
    length = get_cmode_local_var_id(target, ident, limit);
  }
  else /* verbatim output mode */ {
    length = get_vmode_local_var_id(target, ident, limit);
  } /* end if */
  
  return length;
} /* end get_local_var_ident */


/* --------------------------------------------------------------------------
 * function get_local_fn_ident(target, outer, ordinal, ident)
 * --------------------------------------------------------------------------
 * Composes a qualified local function identifier from outer or ordinal and
 * ident, passes it as a NUL terminated C string in out-parameter target.
 *
 * localFuncIdent :=
 *   vModeLocalIdent | cModeLocalFuncIdent
 *   ;
 *
 * Returns length of composed identifier on success.
 * Fails and returns zero if ident is NULL or empty.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_local_fn_id
  (char_ptr_t *target,
   const char *outer, uint_t ordinal, const char *id, uint_t limit);

uint_t get_local_fn_ident
  (char_ptr_t *target, const char *outer, uint_t ordinal, const char *ident) {
  
  uint_t length;
  uint_t limit = M2C_MAX_C_IDENT_LENGTH;
  
  /* ident must not be NULL nor empty */
  if ((ident == NULL) || (CSTR_FIRST_CHAR(ident) == ASCII_NUL)) {
    return 0;
  } /* end if */
  
  if (m2c_option_cstyle_output()) {
    length = get_cmode_local_fn_id(target, outer, ordinal, ident, limit);
  }
  else /* verbatim output mode */ {
    length = get_vmode_local_id(target, outer, ordinal, ident, limit);
  } /* end if */
  
  return length;
} /* end get_local_fn_ident */


/* *********************************************************************** *
 * Private Functions                                                       *
 * *********************************************************************** */

#define TO_UPPER(_ch) ((IS_LOWER(_ch)) ? ((_ch) - 32) : (_ch))

#define TO_LOWER(_ch) ((IS_UPPER(_ch)) ? ((_ch) + 32) : (_ch))

#define TO_LOWER_UNCHECKED(_ch) ((_ch) + 32)

#define MAX_UINT_STR_LEN ((sizeof(uint_t)*CHAR_BIT/4)+1)

static uint_t verbatim_copy_append
  (char_ptr_t *target, uint_t index, const char *source, uint_t limit);

static uint_t lower_transform_append
  (char_ptr_t *target, uint_t index, const char *source, uint_t limit);

static uint_t upper_transform_append
  (char_ptr_t *target, uint_t index, const char *source, uint_t limit);

static uint_t uint_transform_append
  (char_ptr_t *target, uint_t index, uint_t ordinal, uint_t limit);

static const char *collision_cleared_module_id (const char *ident);


/* ==========================================================================
 * Private functions for Verbatim output mode
 * ======================================================================= */

/* --------------------------------------------------------------------------
 * private function get_vmode_include_guard_id(target, module_id, limit)
 * --------------------------------------------------------------------------
 * Composes a C include guard macro identifier from module_id and passes it
 * as a NUL terminated C string in out-parameter target.
 *
 * vModeIncludeGuard :=
 *   'MODULE__' moduleIdent '__H'
 *   ;
 *
 * moduleIdent :=
 *   verbatim copy of module identifier
 *   ;
 *
 * Example:
 *   DEFINITION MODULE FooLib; => MODULE__FooLib__H
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_vmode_include_guard_id
  (char_ptr_t *target, const char *module_id, uint_t limit) {
  
  uint_t length = 0;
  
  /* 'MODULE__' */
  length = verbatim_copy_append(target, length, "MODULE__", limit);
  
  /* moduleIdent */
  length = verbatim_copy_append(target, length, module_id, limit - 3);
  
  /* '__H' */
  length = verbatim_copy_append(target, length, "__H", limit);
      
  return length;
} /* end get_vmode_include_guard_id */


/* --------------------------------------------------------------------------
 * private function get_vmode_public_id(target, module_id, ident, limit)
 * --------------------------------------------------------------------------
 * Composes a qualified public identifier from module_id and ident,
 * and passes it as a NUL terminated C string in out-parameter target.
 *
 * vModePublicIdent :=
 *   publicQualifier sourceIdent
 *   ;
 *
 * publicQualifier :=
 *   moduleIdent '__'
 *   ;
 *
 * moduleIdent :=
 *   verbatim copy of module identifier
 *   ;
 *
 * sourceIdent :=
 *   verbatim copy of identifier
 *   ;
 *
 * Example:
 *   DEFINITION MODULE FooLib; CONST barBaz; => FooLib__barBaz
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_vmode_public_id
  (char_ptr_t *target, const char *module_id, const char *id, uint_t limit) {
  
  uint_t length = 0;
  
  /* prevent collision with identifier prefixes */
  module_id = collision_cleared_module_id(module_id);
  
  /* publicQualifier */
  length = verbatim_copy_append(target, length, module_id, limit);
  length = verbatim_copy_append(target, length, "__", limit);
  
  /* sourceIdent */
  length = verbatim_copy_append(target, length, id, limit);
      
  return length;
} /* end get_vmode_public_id */


/* --------------------------------------------------------------------------
 * private function get_vmode_private_id(target, ident, limit)
 * --------------------------------------------------------------------------
 * Composes a qualified private identifier from ident and passes it
 * as a NUL terminated C string in out-parameter target.
 *
 * vModePrivateIdent :=
 *   privateQualifier sourceIdent
 *   ;
 *
 * privateQualifier :=
 *   'Private__'
 *   ;
 *
 * sourceIdent :=
 *   verbatim copy of identifier
 *   ;
 *
 * Example:
 *   IMPLEMENTATION MODULE ...; CONST fooBar; => Private__fooBar
 *
 * Returns length of composed identifier on success.
 * ----------------------------------------------------------------------- */

static uint_t get_vmode_private_id
  (char_ptr_t *target, const char *id, uint_t limit) {
  
  uint_t length = 0;
  
  /* privateQualifier */
  length = verbatim_copy_append(target, length, "Private__", limit);
  
  /* sourceIdent */
  length = verbatim_copy_append(target, length, id, limit);
      
  return length;
} /* end get_vmode_private_id */


/* --------------------------------------------------------------------------
 * private function get_vmode_local_var_id(target, ident, limit)
 * --------------------------------------------------------------------------
 * Composes a disambiguated local variable identifier from ident,
 * passes it as a NUL terminated C string in out-parameter target.
 *
 * vModeLocalVarIdent :=
 *   sourceIdent collisionPreventionSuffix?
 *   ;
 *
 * collisionPreventionSuffix :=
 *   '_'
 *   ;
 *
 * sourceIdent :=
 *   verbatim copy of variable identifier
 *   ;
 *
 * Example:
 *   PROCEDURE outerProc; VAR fooBar; => fooBar
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_vmode_local_var_id
  (char_ptr_t *target, const char *id, uint_t limit) {
  
  uint_t length = 0;
  
  /* sourceIdent */
  length = verbatim_copy_append(target, length, id, limit);
  
  /* collisionPreventionSuffix? */
  if (is_c_resword(id)) {
    length = verbatim_copy_append(target, length, "_", limit);
  } /* end if */
  
  return length;
} /* end get_vmode_local_var_id */


/* --------------------------------------------------------------------------
 * private function get_vmode_local_id(target, outer, ordinal, ident. limit)
 * --------------------------------------------------------------------------
 * Composes a qualified local type identifier from outer or ordinal and
 * ident, passes it as a NUL terminated C string in out-parameter target.
 *
 * vModeLocalIdent :=
 *   localQualifier enclFuncQualifier sourceIdent
 *   ;
 *
 * localQualifier :=
 *   'Local__'
 *   ;
 *
 * enclFuncQualifier :=
 *   ( enclFunctionIdent | enclFunctionOrdinal ) '__'
 *   ;
 *
 * enclFunctionIdent
 *   verbatim copy of enclosing function identifier
 *   ;
 *
 * enclFunctionOrdinal :=
 *   Base10Digit Base16Digit*
 *   ;
 *
 * sourceIdent :=
 *   verbatim copy of identifier
 *   ;
 *
 * Example:
 *   PROCEDURE outerProc; TYPE FooBar; => Local__outerProc__FooBar
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_vmode_local_id
  (char_ptr_t *target,
   const char *outer, uint_t ordinal, const char *id, uint_t limit) {
  
  uint_t length = 0;
  
  /* localQualifier */
  length = verbatim_copy_append(target, length, "Local__", limit);
  
  /* enclFuncQualifier */
  length = verbatim_copy_append(target, length, outer, limit);
  length = verbatim_copy_append(target, length, "__", limit);
  
  /* sourceIdent */
  length = verbatim_copy_append(target, length, id, limit);
      
  return length;
} /* end get_vmode_local_id */


/* ==========================================================================
 * Private functions for C-style output mode
 * ======================================================================= */

/* --------------------------------------------------------------------------
 * private function get_cmode_include_guard_id(target, module_id, limit)
 * --------------------------------------------------------------------------
 * Composes a C include guard macro identifier from module_id and passes it
 * as a NUL terminated C string in out-parameter target.
 *
 * cModeIncludeGuard :=
 *   upperModuleIdent '_H'
 *   ;
 *
 * upperModuleIdent :=
 *   uppercase/lowline transformation of module identifier
 *   ;
 *
 * Example:
 *   DEFINITION MODULE FooLib; => FOO_LIB_H
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_include_guard_id
  (char_ptr_t *target, const char *module_id, uint_t limit) {
  
  uint_t length = 0;
    
  /* prevent collision for non-conformant name */
  if (IS_LOWER(CSTR_FIRST_CHAR(module_id))) {
    length = verbatim_copy_append(target, length, "x_", limit);
  } /* end if */
  
  /* upperModuleIdent */
  length = upper_transform_append(target, length, module_id, limit - 2);
  
  /* '_H' */
  length = verbatim_copy_append(target, length, "_H", limit);
      
  return length;
} /* end get_cmode_include_guard_id */


/* --------------------------------------------------------------------------
 * private function get_cmode_public_const_id(target, module_id, ident, lim)
 * --------------------------------------------------------------------------
 * Composes a qualified public constant identifier from module_id and ident,
 * and passes it as a NUL terminated C string in out-parameter target.
 *
 * cModePublicConstIdent :=
 *   upperPublicQualifier upperSourceIdent
 *   ;
 *
 * upperPublicQualifier :=
 *   upperModuleIdent '__'
 *   ;
 *
 * upperModuleIdent :
 *   uppercase/lowline transformation of module identifier
 *   ;
 *
 * upperSourceIdent :=
 *   uppercase/lowline transformation of constant identifier
 *   ;
 *
 * Example:
 *   DEFINITION MODULE FooLib; CONST barBaz; => FOO_LIB__BAR_BAZ
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_public_const_id
  (char_ptr_t *target, const char *module_id, const char *id, uint_t limit) {
  
  uint_t length = 0;
  
  /* prevent collision with identifier prefixes */
  module_id = collision_cleared_module_id(module_id);
  
  /* prevent collision for non-conformant name */
  if (IS_LOWER(CSTR_FIRST_CHAR(module_id))) {
    length = verbatim_copy_append(target, length, "x_", limit);
  } /* end if */
  
  /* upperPublicQualifier */
  length = upper_transform_append(target, length, module_id, limit);
  length = verbatim_copy_append(target, length, "__", limit);
  
  /* prevent collision for non-conformant name */
  if (IS_UPPER(CSTR_FIRST_CHAR(id))) {
    length = verbatim_copy_append(target, length, "x_", limit);
  } /* end if */
  
  /* upperSourceIdent */
  length = upper_transform_append(target, length, id, limit);
  
  /* prevent collision for identifiers ending in H */
  if (cstr_last_char(id) == 'H') {
    length = verbatim_copy_append(target, length, "_", limit);
  } /* end if */
  
  return length;
} /* end get_cmode_public_const_id */


/* --------------------------------------------------------------------------
 * private function get_cmode_public_type_id(target, module_id, ident, limit)
 * --------------------------------------------------------------------------
 * Composes a qualified public type identifier from module_id and ident,
 * and passes it as a NUL terminated C string in out-parameter target.
 *
 * cModePublicTypeIdent :=
 *   lowerPublicQualifier lowerSourceIdent '_t'
 *   ;
 *
 * lowerPublicQualifier :=
 *   lowerModuleIdent '__'
 *   ;
 *
 * lowerModuleIdent :
 *   lowercase/lowline transformation of module identifier
 *   ;
 *
 * lowerSourceIdent :=
 *   lowercase/lowline transformation of constant identifier
 *   ;
 *
 * Example:
 *   DEFINITION MODULE FooLib; TYPE BarBaz; => foo_lib__bar_baz_t
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_public_type_id
  (char_ptr_t *target, const char *module_id, const char *id, uint_t limit) {
  
  uint_t length = 0;
  
  /* prevent collision with identifier prefixes */
  module_id = collision_cleared_module_id(module_id);
  
  /* prevent collision for non-conformant name */
  if (IS_LOWER(CSTR_FIRST_CHAR(module_id))) {
    length = verbatim_copy_append(target, length, "X_", limit);
  } /* end if */
  
  /* lowerPublicQualifier */
  length = lower_transform_append(target, length, module_id, limit - 2);
  length = verbatim_copy_append(target, length, "__", limit - 2);
  
  /* prevent collision for non-conformant name */
  if (IS_LOWER(CSTR_FIRST_CHAR(id))) {
    length = verbatim_copy_append(target, length, "X_", limit - 2);
  } /* end if */
  
  /* lowerSourceIdent */
  length = lower_transform_append(target, length, id, limit - 2);
  
  /* '_t' */
  length = verbatim_copy_append(target, length, "_t", limit);
  
  return length;
} /* end get_cmode_public_type_id */


/* --------------------------------------------------------------------------
 * private function get_cmode_public_vfn_id(target, module_id, ident, limit)
 * --------------------------------------------------------------------------
 * Composes a qualified public variable or function identifier from module_id
 * and ident, passes it as a NUL terminated C string in out-parameter target.
 *
 * cModePublicVarOrFnIdent :=
 *   lowerPublicQualifier lowerSourceIdent
 *   ;
 *
 * lowerPublicQualifier :=
 *   lowerModuleIdent '__'
 *   ;
 *
 * lowerModuleIdent :
 *   lowercase/lowline transformation of module identifier
 *   ;
 *
 * lowerSourceIdent :=
 *   lowercase/lowline transformation of variable/function identifier
 *   ;
 *
 * Example:
 *   DEFINITION MODULE FooLib; VAR barBaz; => foo_lib__bar_baz
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_public_vfn_id
  (char_ptr_t *target, const char *module_id, const char *id, uint_t limit) {
  
  uint_t length = 0;
  
  /* prevent collision with identifier prefixes */
  module_id = collision_cleared_module_id(module_id);
  
  /* prevent collision for non-conformant name */
  if (IS_LOWER(CSTR_FIRST_CHAR(module_id))) {
    length = verbatim_copy_append(target, length, "X_", limit);
  } /* end if */
  
  /* lowerPublicQualifier */
  length = lower_transform_append(target, length, module_id, limit);
  length = verbatim_copy_append(target, length, "__", limit);
  
  /* prevent collision for non-conformant name */
  if (IS_UPPER(CSTR_FIRST_CHAR(id))) {
    length = verbatim_copy_append(target, length, "X_", limit);
  } /* end if */
  
  /* lowerSourceIdent */
  length = lower_transform_append(target, length, id, limit);
  
  /* prevent collision for identifiers ending in T */
  if (cstr_last_char(id) == 'T') {
    length = verbatim_copy_append(target, length, "_", limit);
  } /* end if */
  
  return length;
} /* end get_cmode_public_vfn_id */


/* --------------------------------------------------------------------------
 * private function get_cmode_private_const_id(target, ident, limit)
 * --------------------------------------------------------------------------
 * Composes a qualified private constant identifier from ident and passes
 * it as a NUL terminated C string in out-parameter target.
 *
 * cModePrivateConstIdent :=
 *   upperPrivateQualifier upperSourceIdent
 *   ;
 *
 * upperPrivateQualifier :=
 *   'PRIVATE__'
 *   ;
 *
 * upperSourceIdent :=
 *   uppercase/lowline transformation of constant identifier
 *   ;
 *
 * Example:
 *   IMPLEMENTATION MODULE ...; CONST fooBar; => PRIVATE__FOO_BAR
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_private_const_id
  (char_ptr_t *target, const char *id, uint_t limit) {
  
  uint_t length = 0;
  
  /* upperPrivateQualifier */
  length = verbatim_copy_append(target, length, "PRIVATE__", limit);
  
  /* prevent collision for non-conformant name */
  if (IS_UPPER(CSTR_FIRST_CHAR(id))) {
    length = verbatim_copy_append(target, length, "x_", limit);
  } /* end if */
  
  /* upperSourceIdent */
  length = upper_transform_append(target, length, id, limit);
  
  /* prevent collision for identifier ending in H */
  if (cstr_last_char(id) == 'H') {
    length = verbatim_copy_append(target, length, "_", limit);
  } /* end if */
  
  return length;
} /* end get_cmode_private_const_id */


/* --------------------------------------------------------------------------
 * private function get_cmode_private_type_id(target, ident, limit)
 * --------------------------------------------------------------------------
 * Composes a qualified private type identifier from ident and passes it as
 * a NUL terminated C string in out-parameter target.
 *
 * cModePrivateTypeIdent :=
 *   lowerPrivateQualifier lowerSourceIdent '_t'
 *   ;
 *
 * lowerPrivateQualifier :=
 *   'private__'
 *   ;
 *
 * lowerSourceIdent :=
 *   lowercase/lowline transformation of type identifier
 *   ;
 *
 * Example:
 *   IMPLEMENTATION MODULE ...; TYPE FooBar; => private__foo_bar_t
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_private_type_id
  (char_ptr_t *target, const char *id, uint_t limit) {
  
  uint_t length = 0;
    
  /* lowerPrivateQualifier */
  length = verbatim_copy_append(target, length, "private__", limit);
  
  /* prevent collision for non-conformant name */
  if (IS_LOWER(CSTR_FIRST_CHAR(id))) {
    length = verbatim_copy_append(target, length, "X_", limit);
  } /* end if */
  
  /* lowerSourceIdent */
  length = lower_transform_append(target, length, id, limit - 2);
  
  /* '_t' */
  length = verbatim_copy_append(target, length, "_t", limit);
  
  return length;
} /* end get_cmode_private_type_id */


/* --------------------------------------------------------------------------
 * function get_cmode_private_vfn_id(target, ident, limit)
 * --------------------------------------------------------------------------
 * Composes a qualified private variable or function identifier from ident
 * and passes it as a NUL terminated C string in out-parameter target.
 *
 * cModePrivateVarOrFnIdent :=
 *   lowerPrivateQualifier lowerSourceIdent
 *   ;
 *
 * lowerPrivateQualifier :=
 *   'private__'
 *   ;
 *
 * lowerSourceIdent :=
 *   lowercase/lowline transformation of variable/function identifier
 *   ;
 *
 * Example:
 *   IMPLEMENTATION MODULE ...; VAR fooBar;  => private__foo_bar
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_private_vfn_id
  (char_ptr_t *target, const char *id, uint_t limit) {
  
  uint_t length = 0;
    
  /* lowerPrivateQualifier */
  length = verbatim_copy_append(target, length, "private__", limit);
  
  /* prevent collision for non-conformant name */
  if (IS_UPPER(CSTR_FIRST_CHAR(id))) {
    length = verbatim_copy_append(target, length, "X_", limit);
  } /* end if */
  
  /* lowerSourceIdent */
  length = lower_transform_append(target, length, id, limit);
  
  return length;
} /* end get_cmode_private_vfn_id */


/* --------------------------------------------------------------------------
 * private function get_cmode_local_const_id(target, outer, ord, ident, lim)
 * --------------------------------------------------------------------------
 * Composes a qualified local constant identifier from outer or ordinal and
 * ident, passes it as a NUL terminated C string in out-parameter target.
 *
 * cModeLocalConstIdent :=
 *   upperLocalQualifier upperEnclFuncQualifier upperSourceIdent
 *   ;
 *
 * upperLocalQualifier :=
 *   'LOCAL__'
 *   ;
 *
 * upperEnclFuncQualifier :=
 *   ( upperEnclFunctionIdent | enclFunctionOrdinal ) '__'
 *   ;
 *
 * upperEnclFunctionIdent
 *   uppercase/lowline transformation of enclosing function identifier
 *   ;
 *
 * upperSourceIdent :=
 *   uppercase/lowline transformation of constant identifier
 *   ;
 *
 * Example:
 *   PROCEDURE outerProc; CONST fooBar; => LOCAL__OUTER_PROC__FOO_BAR
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_local_const_id
  (char_ptr_t *target,
   const char *outer, uint_t ordinal, const char *id, uint_t limit) {
  
  uint_t length = 0;
  
  /* upperLocalQualifier */
  length = verbatim_copy_append(target, length, "LOCAL__", limit);
  
  /* prevent collision for non-conformant name */
  if (IS_UPPER(CSTR_FIRST_CHAR(outer))) {
    length = verbatim_copy_append(target, length, "x_", limit);
  } /* end if */
  
  /* upperEnclFuncQualifier */
  length = upper_transform_append(target, length, outer, limit);
  length = verbatim_copy_append(target, length, "__", limit);
  
  /* prevent collision for non-conformant name */
  if (IS_UPPER(CSTR_FIRST_CHAR(id))) {
    length = verbatim_copy_append(target, length, "x_", limit);
  } /* end if */
  
  /* upperSourceIdent */
  length = upper_transform_append(target, length, id, limit);
  
  return length;
} /* end get_cmode_local_const_id */


/* --------------------------------------------------------------------------
 * function get_local_type_ident(target, outer, ordinal, ident, limit)
 * --------------------------------------------------------------------------
 * Composes a qualified local type identifier from outer or ordinal and
 * ident, passes it as a NUL terminated C string in out-parameter target.
 *
 * cModeLocalConstIdent :=
 *   lowerLocalQualifier lowerEnclFuncQualifier lowerSourceIdent '_t'
 *   ;
 *
 * lowerLocalQualifier :=
 *   'local__'
 *   ;
 *
 * lowerEnclFuncQualifier :=
 *   ( lowerEnclFunctionIdent | enclFunctionOrdinal ) '__'
 *   ;
 *
 * lowerEnclFunctionIdent
 *   lowercase/lowline transformation of enclosing function identifier
 *   ;
 *
 * lowerSourceIdent :=
 *   lowercase/lowline transformation of type identifier
 *   ;
 *
 * Example:
 *   PROCEDURE outerProc; TYPE FooBar; => local__outer_proc__foo_bar_t
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_local_type_id
  (char_ptr_t *target,
   const char *outer, uint_t ordinal, const char *id, uint_t limit) {
  
  uint_t length = 0;
  
  /* lowerLocalQualifier */
  length = verbatim_copy_append(target, length, "local__", limit);
  
  /* prevent collision for non-conformant name */
  if (IS_UPPER(CSTR_FIRST_CHAR(outer))) {
    length = verbatim_copy_append(target, length, "X_", limit);
  } /* end if */
  
  /* lowerEnclFuncQualifier */
  length = lower_transform_append(target, length, outer, limit);
  length = verbatim_copy_append(target, length, "__", limit - 2);
  
  /* prevent collision for non-conformant name */
  if (IS_LOWER(CSTR_FIRST_CHAR(id))) {
    length = verbatim_copy_append(target, length, "X_", limit - 2);
  } /* end if */
  
  /* lowerSourceIdent */
  length = lower_transform_append(target, length, id, limit - 2);
  
  /* '_t' */
  length = verbatim_copy_append(target, length, "_t", limit);
  
  return length;
} /* end get_cmode_local_type_id */


/* --------------------------------------------------------------------------
 * function get_local_var_ident(target, ident, limit)
 * --------------------------------------------------------------------------
 * Composes a disambiguated local variable identifier from ident,
 * passes it as a NUL terminated C string in out-parameter target.
 *
 * cModeLocalVarIdent :=
 *   lowerSourceIdent collisionPreventionSuffix?
 *   ;
 *
 * lowerSourceIdent :=
 *   lowercase/lowline transformation of variable identifier
 *   ;
 *
 * Example:
 *   PROCEDURE outerProc; VAR fooBar; => foo_bar
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_local_var_id
  (char_ptr_t *target, const char *id, uint_t limit) {
  
  uint_t length = 0;
  
  /* prevent collision for non-conformant name */
  if (IS_UPPER(CSTR_FIRST_CHAR(id))) {
    length = verbatim_copy_append(target, length, "X_", limit);
  } /* end if */
  
  /* lowerSourceIdent */
  length = lower_transform_append(target, length, id, limit);
  
  /* collisionPreventionSuffix? */
  if (is_c_resword(id)) {
    length = verbatim_copy_append(target, length, "_", limit);
  } /* end if */
  
  return length;
} /* end get_cmode_local_var_id */


/* --------------------------------------------------------------------------
 * function get_local_fn_ident(target, outer, ordinal, ident, limit)
 * --------------------------------------------------------------------------
 * Composes a qualified local function identifier from outer or ordinal and
 * ident, passes it as a NUL terminated C string in out-parameter target.
 *
 * cModeLocalFuncIdent :=
 *   lowerLocalQualifier lowerEnclFuncQualifier lowerSourceIdent
 *   ;
 *
 * lowerLocalQualifier :=
 *   'local__'
 *   ;
 *
 * lowerEnclFuncQualifier :=
 *   ( lowerEnclFunctionIdent | enclFunctionOrdinal ) '__'
 *   ;
 *
 * lowerEnclFunctionIdent
 *   lowercase/lowline transformation of enclosing function identifier
 *   ;
 *
 * lowerSourceIdent :=
 *   lowercase/lowline transformation of enclosed function identifier
 *   ;
 *
 * Example:
 *   PROCEDURE outerProc; PROCEDURE innerProc;
 *   => local__outer_proc__inner_proc
 *
 * Returns length of composed identifier.
 * ----------------------------------------------------------------------- */

static uint_t get_cmode_local_fn_id
  (char_ptr_t *target,
   const char *outer, uint_t ordinal, const char *id, uint_t limit) {
  
  uint_t length = 0;
  
  /* lowerLocalQualifier */
  length = verbatim_copy_append(target, length, "local__", limit);
  
  /* prevent collision for non-conformant name */
  if (IS_UPPER(CSTR_FIRST_CHAR(outer))) {
    length = verbatim_copy_append(target, length, "X_", limit);
  } /* end if */
  
  /* lowerEnclFuncQualifier */
  length = lower_transform_append(target, length, outer, limit);
  length = verbatim_copy_append(target, length, "__", limit);
  
  /* prevent collision for non-conformant name */
  if (IS_UPPER(CSTR_FIRST_CHAR(id))) {
    length = verbatim_copy_append(target, length, "X_", limit);
  } /* end if */
  
  /* lowerSourceIdent */
  length = lower_transform_append(target, length, id, limit);
  
  return length;
} /* end get_cmode_local_fn_id */


/* ==========================================================================
 * Lower level private functions
 * ======================================================================= */

/* --------------------------------------------------------------------------
 * private verbatim_copy_append(target, index, source, limit)
 * ----------------------------------------------------------------------- */

static uint_t verbatim_copy_append
  (char_ptr_t *target, uint_t index, const char *source, uint_t limit) {
  
  char ch;
  uint_t source_index = 0;
  
  if (CSTR_FIRST_CHAR(source) == ASCII_NUL) {
    return 0;
  } /* end if */
  
  ch = source[source_index];
  
  while ((ch != ASCII_NUL) && (index < limit)) {
    
    /* copy verbatim */
    (*target)[index] = ch;
    
    /* move to next char */
    index++;
    source_index++;
    ch = source[source_index];
  } /* end while */
  
  /* terminate */
  (*target)[index] = ASCII_NUL;
  
  return index;
} /* end verbatim_copy_append */


/* --------------------------------------------------------------------------
 * private function lower_transform_append(target, index, source, limit)
 * --------------------------------------------------------------------------
 * Transforms source to lowercase/lowline and appends it to target starting
 * at target position index, but limiting the total length to limit.
 * Returns the length of the resulting target string.
 * 
 * Lowercase/lowline transformation rules are as follows: 
 * - capital letters are prepended with lowline, except first character
 * - all capital letters are transformed to lowercase
 * - lowercase letters and digits are copied verbatim
 * ----------------------------------------------------------------------- */

static uint_t lower_transform_append
  (char_ptr_t *target, uint_t index, const char *source, uint_t limit) {
  
  char ch;
  uint_t source_index = 0;
  
  if ((index >= limit) || (CSTR_FIRST_CHAR(source) == ASCII_NUL)) {
    return 0;
  } /* end if */
  
  ch = source[source_index];
  (*target)[index] = TO_LOWER(ch);
  
  index++;
  source_index++;
  ch = source[source_index];
  
  /* following chars */
  while ((ch != ASCII_NUL) && (index < limit)) {
    
    if (IS_UPPER(ch)) {
      /* check length limit again with lowline prepended */
      if (index+1 >= limit) {
        break;
      } /* end if */
      
      /* prepend lowline */
      (*target)[index] = '_';
      index++;
      
      /* transform to lower and copy */
      (*target)[index] = TO_LOWER_UNCHECKED(ch);
    }
    else /* lower or digit */ {
      (*target)[index] = ch;
    } /* end if */
    
    /* move to next char */
    index++;
    source_index++;
    ch = source[source_index];
  } /* end while */
  
  /* terminate */
  (*target)[index] = ASCII_NUL;
    
  return index;
} /* end lower_transform_append */


/* --------------------------------------------------------------------------
 * private function upper_transform_append(target, index, source, limit)
 * --------------------------------------------------------------------------
 * Transforms source to uppercase/lowline and appends it to target starting
 * at target position index, but limiting the total length to limit.
 * Returns the length of the resulting target string.
 * 
 * Uppercase/lowline transformation rules are as follows: 
 * - capital letters are prepended with lowline, except first character
 * - lowercase letters are transformed to uppercase
 * - digits are copied verbatim
 * ----------------------------------------------------------------------- */

static uint_t upper_transform_append
  (char_ptr_t *target, uint_t index, const char *source, uint_t limit) {
  
  char ch;
  uint_t source_index = 0;
    
  if ((index >= limit) || (CSTR_FIRST_CHAR(source) == ASCII_NUL)) {
    return 0;
  } /* end if */
  
  ch = source[source_index];
  (*target)[index] = TO_UPPER(ch);
  
  index++;
  source_index++;
  ch = source[source_index];
  
  /* following chars */
  while ((ch != ASCII_NUL) && (index < limit)) {
      
    if (IS_UPPER(ch)) {
      /* check length limit again with lowline prepended */
      if (index+1 >= limit) {
        break;
      } /* end if */
      
      /* prepend lowline */
      (*target)[index] = '_';
      index++;
      
      /* copy verbatim */
      (*target)[index] = ch;
    }
    else /* lower or digit */ {
      (*target)[index] = TO_UPPER(ch);
    } /* end if */
    
    /* move to next char */
    index++;
    source_index++;
    ch = source[source_index];
  } /* end while */
  
  /* terminate */
  (*target)[index] = ASCII_NUL;
    
  return index;
} /* end upper_transform_append */


/* --------------------------------------------------------------------------
 * private function get_base16_for_uint(target, ordinal, limit)
 * ----------------------------------------------------------------------- */

static const char BASE16DIGIT[16] = "0123456789abcdef";

static uint_t get_base16_for_uint
  (char_ptr_t *target, uint_t ordinal, uint_t limit) {
  
  uint_t factor, mask;
  uint_t value = ordinal;
  uint_t digits = 1;
  uint_t index = 0;
  
  /* determine number of digits */
  while (value > 0) {
    factor = value & 0xf;
    value = value >> 4;
    digits++;
  } /* end while */
  
  /* no leading zero if first digit is decimal */
  if (factor < 10) {
    digits--;
  } /* end if */
  
  /* abort if digits exceed length limit */
  if (digits > limit) {
    return 0;
  } /* end if */
  
  /* set 4-bit mask for leftmost digit */
  mask = 0xf << (4*digits);
  
  /* copy digits to target */
  while (index < digits) {
    (*target)[index] = BASE16DIGIT[ordinal & mask];
    mask = mask >> 4;
    index++;
  } /* end while */
  
  /* terminate target */
  (*target)[index] = ASCII_NUL;
  
  /* return length */
  return index;
} /* end get_base16_for_uint */


/* --------------------------------------------------------------------------
 * private function collision_cleared_module_id(ident)
 * ----------------------------------------------------------------------- */

typedef enum {
  /* BUILTIN */ ALLCAPS_BUILTIN,
  /* Builtin */ CAPITAL_BUILTIN,
  /* builtin */ LOWER_BUILTIN,
  /* LOCAL   */ ALLCAPS_LOCAL,
  /* Local   */ CAPITAL_LOCAL,
  /* local   */ LOWER_LOCAL,
  /* PRIVATE */ ALLCAPS_PRIVATE,
  /* Private */ CAPITAL_PRIVATE,
  /* private */ LOWER_PRIVATE,
  /* Var     */ CAPITAL_VAR,
  /* var     */ LOWER_VAR,
} collision_code;

static const char *collision_replacement[] = {
  /* BUILTIN => */ "MOD__4255",
  /* Builtin => */ "MOD__4275",
  /* builtin => */ "MOD__6275",
  /* LOCAL   => */ "MOD__4C4F",
  /* Local   => */ "MOD__4C6F",
  /* local   => */ "MOD__6C6F",
  /* PRIVATE => */ "MOD__5052",
  /* Private => */ "MOD__5072",
  /* private => */ "MOD__7072",
  /* Var     => */ "MOD__5661",
  /* var     => */ "MOD__7661",
}; /* collision_replacement */

static const char *collision_cleared_module_id (const char *ident) {
  
  if ((ident == NULL) || (CSTR_FIRST_CHAR(ident) == ASCII_NUL)) {
    return ident;
  } /* end if */
  
  switch (CSTR_FIRST_CHAR(ident)) {
    /* BUILTIN, Builtin */
    case 'B' :
      if (ident[1] == 'U') {
        if (cstr_match("BUILTIN", ident)) {
          return collision_replacement[ALLCAPS_BUILTIN];
        } /* end if */
      }
      else if (ident[1] == 'u') {
        if (cstr_match("Builtin", ident)) {
          return collision_replacement[CAPITAL_BUILTIN];
        } /* end if */
      } /* end if */
      break;
    
    /* LOCAL, Local */
    case 'L' :
      if (ident[1] == 'O') {
        if (cstr_match("LOCAL", ident)) {
          return collision_replacement[ALLCAPS_LOCAL];
        } /* end if */
      }
      else if (ident[1] == 'o') {
        if (cstr_match("Local", ident)) {
          return collision_replacement[CAPITAL_LOCAL];
        } /* end if */
      } /* end if */
      break;
    
    /* PRIVATE, Private */
    case 'P' :
      if (ident[1] == 'R') {
        if (cstr_match("PRIVATE", ident)) {
          return collision_replacement[ALLCAPS_PRIVATE];
        } /* end if */
      }
      else if (ident[1] == 'r') {
        if (cstr_match("Private", ident)) {
          return collision_replacement[CAPITAL_PRIVATE];
        } /* end if */
      } /* end if */
      break;
    
    /* Var */
    case 'V' :
      if (cstr_match("Var", ident)) {
        return collision_replacement[CAPITAL_VAR];
      } /* end if */
      break;
    
    /* builtin */
    case 'b' :
      if (cstr_match("builtin", ident)) {
        return collision_replacement[LOWER_BUILTIN];
      } /* end if */
      break;
      
    /* local */
    case 'l' :
      if (cstr_match("local", ident)) {
        return collision_replacement[LOWER_LOCAL];
      } /* end if */
      break;
    
    /* private */
    case 'p' :
      if (cstr_match("private", ident)) {
        return collision_replacement[LOWER_PRIVATE];
      } /* end if */
    
    /* var */
    case 'v' :
      if (cstr_match("var", ident)) {
        return collision_replacement[LOWER_VAR];
      } /* end if */
  } /* end switch */
  
  return ident;
} /* end collision_cleared_module_id */

/* END OF FILE */