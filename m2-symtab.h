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
 * m2-symtab.h
 *
 * Public interface for M2C symbol table.
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

#ifndef M2C_SYMTAB_H
#define M2C_SYMTAB_H

#include "m2-common.h"

//#include "m2-ast.h"
typedef void *m2c_astnode_t;


/* --------------------------------------------------------------------------
 * type m2c_symtab_status_t
 * --------------------------------------------------------------------------
 * Status codes for operations on type m2c_symtab_t.
 * ----------------------------------------------------------------------- */

typedef enum {
  M2C_SYMTAB_STATUS_SUCCESS,
  M2C_SYMTAB_STATUS_INVALID_REFERENCE,
  M2C_SYMTAB_STATUS_INVALID_SCOPE,
  M2C_SYMTAB_STATUS_INVALID_IDENT,
  M2C_SYMTAB_STATUS_IDENT_NOT_UNIQUE,
  M2C_SYMTAB_STATUS_IDENT_NOT_FOUND,
  M2C_SYMTAB_STATUS_MISSING_SCOPE,
  M2C_SYMTAB_STATUS_ALLOCATION_FAILED,
} m2c_symtab_status_t;


/* --------------------------------------------------------------------------
 * type m2c_symtype_t
 * --------------------------------------------------------------------------
 * Enumeration type representing the entry type of a symbol table entry.
 * ----------------------------------------------------------------------- */

typedef enum {
  M2C_SYMTYPE_MODULE,      /* symbol is a module */
  M2C_SYMTYPE_CONST,       /* symbol is a constant */
  M2C_SYMTYPE_TYPE,        /* symbol is a type */
  M2C_SYMTYPE_VAR,         /* symbol is a variable */
  M2C_SYMTYPE_PROC,        /* symbol is a procedure */
  M2C_SYMTYPE_FIELD,       /* symbol is a record field */
  M2C_SYMTYPE_VAL_PARAM,   /* symbol is a formal parameter */
  M2C_SYMTYPE_VAR_PARAM,   /* symbol is a formal VAR parameter */
  M2C_SYMTYPE_CONST_PARAM, /* symbol is a formal CONST parameter */
} m2c_symtype_t;


/* --------------------------------------------------------------------------
 * type m2c_sym_attr_t
 * --------------------------------------------------------------------------
 * Record type representing the attributes returned by a symbol lookup.
 *
 * Its fields are:
 * - scope, the name of the scope where the symbol was found
 * - kind, the classification of the matched symbol
 * - type_id, the identifier of the type of the matched symbol
 * - definition, the AST node with the definition of the matched symbol 
 * ----------------------------------------------------------------------- */

typedef struct {
  /* scope */ const char *scope;
  /* kind */ m2c_symtype_t kind;
  /* type_id */ const char *type_id;
  /* definition */ m2c_astnode_t definition;
} m2c_sym_attr_t;


/* --------------------------------------------------------------------------
 * opaque type m2c_symtab_t
 * --------------------------------------------------------------------------
 * Opaque pointer type representing a symbol table object.
 * ----------------------------------------------------------------------- */

typedef struct m2c_symtab_struct_t *m2c_symtab_t;


/* --------------------------------------------------------------------------
 * function m2c_new_symtab(scope_id)
 * --------------------------------------------------------------------------
 * Allocates and initialises a new symbol table.
 * ----------------------------------------------------------------------- */

m2c_symtab_t m2c_new_symtab (const char *top_level_scope_id);


/* --------------------------------------------------------------------------
 * function m2c_symtab_open_scope(symtab, scope_id)
 * --------------------------------------------------------------------------
 * Opens a new scope in a symbol table.
 * ----------------------------------------------------------------------- */

m2c_symtab_status_t m2c_symtab_open_scope
  (m2c_symtab_t symtab, const char *scope_id);


/* --------------------------------------------------------------------------
 * function m2c_symtab_insert(symtab, ident, kind, type_id, definition)
 * --------------------------------------------------------------------------
 * Attempts to insert new symbol into the current scope of a symbol table.
 * ----------------------------------------------------------------------- */

m2c_symtab_status_t m2c_symtab_insert
  (m2c_symtab_t symtab,
   const char *ident,
   m2c_symtype_t kind,
   const char *type_id,
   m2c_astnode_t definition);


/* --------------------------------------------------------------------------
 * function m2c_symtab_lookup(symtab, ident, attributes)
 * --------------------------------------------------------------------------
 * Looks up the symbol for ident and if found, passes back its attributes.
 * ----------------------------------------------------------------------- */

m2c_symtab_status_t m2c_symtab_lookup
  (m2c_symtab_t symtab, const char *ident, m2c_sym_attr_t *attributes);


/* --------------------------------------------------------------------------
 * function m2c_symtab_symbol_count(symtab)
 * --------------------------------------------------------------------------
 * Returns the number of symbols currently stored in symbol table symtab.
 * ----------------------------------------------------------------------- */

uint_t m2c_symtab_entry_count (m2c_symtab_t symtab);


/* --------------------------------------------------------------------------
 * function m2c_symtab_scope_count(symtab)
 * --------------------------------------------------------------------------
 * Returns the number of scopes currently open in symbol table symtab.
 * ----------------------------------------------------------------------- */

uint_t m2c_symtab_scope_count (m2c_symtab_t symtab);


/* --------------------------------------------------------------------------
 * function m2c_symtab_open_scope(symtab, scope_id)
 * --------------------------------------------------------------------------
 * Closes a given scope in a symbol table.
 * ----------------------------------------------------------------------- */

m2c_symtab_status_t m2c_symtab_close_scope
  (m2c_symtab_t symtab, const char *scope_id);


/* --------------------------------------------------------------------------
 * function m2c_release_symtab(symtab)
 * --------------------------------------------------------------------------
 * Deallocates a given symbol table.
 * ----------------------------------------------------------------------- */

m2c_symtab_status_t m2c_release_symtab (m2c_symtab_t symtab);


#endif /* M2C_SYMTAB_H */

/* END OF FILE */