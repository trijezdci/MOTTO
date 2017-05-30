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
 * m2-symtab.c
 *
 * Implementation of M2C symbol table.
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

#include "m2-symtab.h"

#include "hash.h"
#include <stdlib.h>

#define M2C_SYMTAB_BUCKET_COUNT_TOPSCOPE 97

#define M2C_SYMTAB_BUCKET_COUNT_SUBSCOPE 17

/* --------------------------------------------------------------------------
 * Logical Representation of Symbol Table
 * --------------------------------------------------------------------------
 *
 * table         scope n                        scope 1
 *               +------------+                 +------------+
 *  current -->  |  previous  | -->  . . .  --> |  previous  | --> NULL
 *               +------------+                 +------------+
 *               |  symbol 1  |                 |  symbol 1  |
 *               +------------+                 +------------+
 *               |  symbol 2  |                 |  symbol 2  |
 *               +------------+                 +------------+
 *               .            .                 .            .
 *               +------------+                 +------------+
 *               |  symbol n  |                 |  symbol n  |
 *               +------------+                 +------------+
 *
 * symbol
 *               +------------+------------+------------+------------+
 *  fields:      | ident      | kind       | type_id    | definition |
 *               +------------+------------+------------+------------+
 * 
 * ----------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * private types m2c_symbol_s and m2c_symbol_t
 * --------------------------------------------------------------------------
 * Record and pointer type representing a symbol table entry.
 * ----------------------------------------------------------------------- */

typedef struct m2c_symbol_s *m2c_symbol_t;

struct m2c_symbol_s {
  /* next */ m2c_symbol_t next;
  /* ident */ const char *ident;
  /* kind */ m2c_symtype_t kind;
  /* type_id */ const char *type_id;
  /* definition */ m2c_astnode_t definition;
};

typedef struct m2c_symbol_s m2c_symbol_s;


/* --------------------------------------------------------------------------
 * private types m2c_symtab_scope_s and m2c_symtab_scope_t
 * --------------------------------------------------------------------------
 * Record and pointer type representing a symbol table scope.
 * ----------------------------------------------------------------------- */

typedef struct m2c_symtab_scope_s *m2c_symtab_scope_t;

struct m2c_symtab_scope_s {
  /* previous */ m2c_symtab_scope_t previous;
  /* ident */ const char *ident;
  /* bucket_count */ uint_t bucket_count;
  /* bucket (table) */ m2c_symbol_t bucket[];
};

typedef struct m2c_symtab_scope_s m2c_symtab_scope_s;


/* --------------------------------------------------------------------------
 * hidden type m2c_symtab_struct_t
 * --------------------------------------------------------------------------
 * Record type representing a symbol table object.
 * ----------------------------------------------------------------------- */

struct m2c_symtab_struct_t {
  /* top */ m2c_symtab_scope_t top;
  /* current */ m2c_symtab_scope_t current;
  /* scope_count */ uint_t scope_count;
  /* symbol_count */ uint_t symbol_count;
};

typedef struct m2c_symtab_struct_t m2c_symtab_struct_t;


/* --------------------------------------------------------------------------
 * forward declarations
 * ----------------------------------------------------------------------- */

typedef uint32_t m2c_hash_t;

static m2c_hash_t key_for_cstr (const char *cstr);

static m2c_symbol_t new_symbol (const char *ident,
  m2c_symtype_t kind, const char *type_id, m2c_astnode_t defn);

static m2c_symbol_t scope_lookup
  (m2c_symtab_scope_t scope, const char *ident, m2c_hash_t key);

static void remove_scope (m2c_symtab_t symtab, m2c_symtab_scope_t scope);


/* --------------------------------------------------------------------------
 * function m2c_new_symtab(scope_id)
 * --------------------------------------------------------------------------
 * Allocates and initialises a new symbol table.
 * ----------------------------------------------------------------------- */

m2c_symtab_t m2c_new_symtab (const char *top_level_scope_id) {
  
  m2c_symtab_t new_table;
  m2c_symtab_status_t status;
  
  if (top_level_scope_id == NULL) {
    return NULL;
  } /* end if */
  
  /* allocate new table */
  new_table = malloc(sizeof(m2c_symtab_struct_t));
  
  if (new_table == NULL) {
    return NULL;
  } /* end if */
  
  /* initialise table */
  new_table->top = NULL;
  new_table->current = NULL;
  new_table->scope_count = 0;
  new_table->symbol_count = 0;
  
  /* allocate and initialise top level scope */
  status = m2c_symtab_open_scope(new_table, top_level_scope_id);
  
  if (status != M2C_SYMTAB_STATUS_SUCCESS) {
    free(new_table);
    return NULL;
  } /* end if */
  
  return new_table;
} /* end m2c_new_symtab */


/* --------------------------------------------------------------------------
 * function m2c_symtab_open_scope(symtab, scope_id)
 * --------------------------------------------------------------------------
 * Opens a new scope in a symbol table.
 * ----------------------------------------------------------------------- */

m2c_symtab_status_t m2c_symtab_open_scope
  (m2c_symtab_t symtab, const char *scope_id) {
  
  uint_t index, bucket_count;
  m2c_symtab_scope_t new_scope;
  
  if (symtab == NULL) {
    return M2C_SYMTAB_STATUS_INVALID_REFERENCE;
  } /* end if */
  
  if (scope_id == NULL) {
    return M2C_SYMTAB_STATUS_INVALID_SCOPE;
  } /* end if */
  
  if (symtab->top == NULL) {
    bucket_count = M2C_SYMTAB_BUCKET_COUNT_TOPSCOPE;
  }
  else {
    bucket_count = M2C_SYMTAB_BUCKET_COUNT_SUBSCOPE;
  } /* end if */
  
  /* allocate new scope */
  new_scope =
    malloc(sizeof(m2c_symtab_scope_s) + bucket_count * sizeof(m2c_symbol_t));
  
  if (new_scope == NULL) {
    return M2C_SYMTAB_STATUS_ALLOCATION_FAILED;
  } /* end if */
  
  /* initialise scope */
  new_scope->ident = scope_id;
  new_scope->bucket_count = bucket_count;
  
  /* initialise buckets */
  for (index = 0; index < bucket_count; index++) {
    new_scope->bucket[index] = NULL;
  } /* end for */
  
  /* link new scope to symbol table */
  if (symtab->top == NULL) {
    symtab->top = new_scope;
    new_scope->previous = NULL;
  }
  else {
    new_scope->previous = symtab->current;
  } /* end if */
  
  symtab->current = new_scope;
  
  /* update counter */
  symtab->scope_count++;
  
  return M2C_SYMTAB_STATUS_SUCCESS;
} /* end m2c_symtab_open_scope */


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
   m2c_astnode_t definition) {
  
  uint_t index;
  m2c_hash_t key;
  m2c_symtab_scope_t scope;
  m2c_symbol_t prev_symbol, this_symbol;
  
  if (symtab == NULL) {
    return M2C_SYMTAB_STATUS_INVALID_REFERENCE;
  } /* end if */
  
  if (ident == NULL) {
    return M2C_SYMTAB_STATUS_INVALID_IDENT;
  } /* end if */
  
  scope = symtab->current;
  
  if (scope == NULL) {
    return M2C_SYMTAB_STATUS_MISSING_SCOPE;
  } /* end if */
    
  /* determine bucket index */
  key = key_for_cstr(ident);
  index = key % scope->bucket_count;
  
  /* get first symbol slot in bucket */
  this_symbol = scope->bucket[index];
  
  if /* bucket empty */ (this_symbol == NULL) {
    this_symbol = new_symbol(ident, kind, type_id, definition);
    scope->bucket[index] = this_symbol;
  }
  else /* bucket not empty */ {
    /* check if symbol is already in bucket */
    do /* traverse bucket */ {
      
      /* symbol is already present, bail out to avoid duplication */
      if (this_symbol->ident == ident) {
        return M2C_SYMTAB_STATUS_IDENT_NOT_UNIQUE;
      } /* end if */
      
      /* move to next symbol */
      prev_symbol = this_symbol;
      this_symbol = this_symbol->next;
    } while (this_symbol != NULL);
    
    /* symbol not found in bucket, allocate and add new symbol */
    this_symbol = new_symbol(ident, kind, type_id, definition);
    prev_symbol->next = this_symbol;
  } /* end if */
  
  /* update counter */
  symtab->symbol_count++;
  
  return M2C_SYMTAB_STATUS_SUCCESS;
} /* end m2c_symtab_insert */


/* --------------------------------------------------------------------------
 * function m2c_symtab_lookup(symtab, ident, attributes)
 * --------------------------------------------------------------------------
 * Looks up the symbol for ident and if found, passes back its attributes.
 * ----------------------------------------------------------------------- */

m2c_symtab_status_t m2c_symtab_lookup
  (m2c_symtab_t symtab, const char *ident, m2c_sym_attr_t *attributes) {
  
  m2c_hash_t key;
  m2c_symbol_t this_symbol;
  m2c_symtab_scope_t this_scope;
  
  if (symtab == NULL) {
    return M2C_SYMTAB_STATUS_INVALID_REFERENCE;
  } /* end if */
  
  if (ident == NULL) {
    return M2C_SYMTAB_STATUS_INVALID_IDENT;
  } /* end if */
  
  /* start with current scope */
  this_scope = symtab->current;
  
  key = key_for_cstr(ident);
  
  /* iterate over all scopes */
  while (this_scope != NULL) {
    /* lookup symbol in this scope */
    this_symbol = scope_lookup(this_scope, ident, key);
    
    /* exit loop if found */
    if (this_symbol != NULL) {
      break;
    } /* end if */
    
    /* move to previous scope */
    this_scope = this_scope->previous;
  } /* end while */
  
  if (this_scope == NULL) {
    return M2C_SYMTAB_STATUS_IDENT_NOT_FOUND;
  } /* end if */
  
  /* pass back symbol's attributes */
  if (attributes != NULL) {
    (*attributes).scope = this_scope->ident;
    (*attributes).kind = this_symbol->kind;
    (*attributes).type_id = this_symbol->type_id;
    (*attributes).definition = this_symbol->definition;
  } /* end if */
  
  return M2C_SYMTAB_STATUS_SUCCESS;
} /* end m2c_symtab_lookup */


/* --------------------------------------------------------------------------
 * function m2c_symtab_symbol_count(symtab)
 * --------------------------------------------------------------------------
 * Returns the number of symbols currently stored in symbol table symtab.
 * ----------------------------------------------------------------------- */

uint_t m2c_symtab_symbol_count (m2c_symtab_t symtab) {
  
  if (symtab == NULL) {
    return 0;
  } /* end if */
  
  return symtab->symbol_count;
} /* end m2c_symtab_symbol_count */


/* --------------------------------------------------------------------------
 * function m2c_symtab_scope_count(symtab)
 * --------------------------------------------------------------------------
 * Returns the number of scopes currently open in symbol table symtab.
 * ----------------------------------------------------------------------- */

uint_t m2c_symtab_scope_count (m2c_symtab_t symtab) {
  
  if (symtab == NULL) {
    return 0;
  } /* end if */
  
  return symtab->scope_count;
} /* end m2c_symtab_scope_count */


/* --------------------------------------------------------------------------
 * function m2c_symtab_close_scope(symtab, scope_id)
 * --------------------------------------------------------------------------
 * Closes a given scope in a symbol table.
 * ----------------------------------------------------------------------- */

m2c_symtab_status_t m2c_symtab_close_scope
  (m2c_symtab_t symtab, const char *scope_id) {
  
  m2c_symtab_scope_t target_scope, this_scope, prev_scope;
  
  /* determine if scope_id is valid */
  
  /* start with current scope */
  this_scope = symtab->current;
  
  /* search for scope whose identifier matches scope_id */
  while ((this_scope != NULL) && (this_scope->ident != scope_id)) {
    this_scope = this_scope->previous;
  } /* end while */
  
  if (this_scope == NULL) {
    return M2C_SYMTAB_STATUS_INVALID_SCOPE;
  } /* end if */
  
  /* remove all scopes up to and including the scope matching scope_id */
  
  target_scope = this_scope->previous;
  
  /* start with current scope */
  this_scope = symtab->current;
  
  /* remove all scopes up to target scope */
  while (this_scope != target_scope) {
    /* remove scope */
    prev_scope = this_scope->previous;
    remove_scope(symtab, this_scope);
    
    /* move to previous scope */
    this_scope = prev_scope;
  } /* end while */
  
  if (target_scope == NULL) {
    symtab->top = NULL;
  } /* end if */
  
  symtab->current = target_scope;
  
  return M2C_SYMTAB_STATUS_SUCCESS;  
} /* end m2c_symtab_close_scope */


/* --------------------------------------------------------------------------
 * function m2c_release_symtab(table)
 * --------------------------------------------------------------------------
 * Deallocates a given symbol table.
 * ----------------------------------------------------------------------- */

m2c_symtab_status_t m2c_release_symtab (m2c_symtab_t symtab) {
  
  m2c_symtab_scope_t this_scope, prev_scope;
  
  if (symtab == NULL) {
   return M2C_SYMTAB_STATUS_INVALID_REFERENCE;
  } /* end if */
  
  this_scope = symtab->current;
  
  while (this_scope != NULL) {
    prev_scope = this_scope->previous;
    remove_scope(symtab, this_scope);
    this_scope = prev_scope;
  } /* end while */
  
  symtab->current = NULL;
  free(symtab);
  
  return M2C_SYMTAB_STATUS_SUCCESS;
} /* end m2c_release_symtab */


/* *********************************************************************** *
 * Private Functions                                                       *
 * *********************************************************************** */

/* --------------------------------------------------------------------------
 * private function key_for_cstr(cstr)
 * --------------------------------------------------------------------------
 * Returns hash key for a given NUL terminated C string.
 * ----------------------------------------------------------------------- */

static m2c_hash_t key_for_cstr (const char *cstr) {
  
  m2c_hash_t key;
  uint_t index;
  char ch;
  
  index = 0;
  ch = cstr[index];
  key = HASH_INITIAL;
  while (ch != ASCII_NUL) {
    key = HASH_NEXT_CHAR(key, ch);
    index++;
    ch = cstr[index];
  } /* end while */
  
  return HASH_FINAL(key);
} /* end key_for_cstr */


/* --------------------------------------------------------------------------
 * private function scope_lookup(scope, ident)
 * --------------------------------------------------------------------------
 * Searches ident in scope and returns matching symbol or NULL if not found.
 * ----------------------------------------------------------------------- */

static m2c_symbol_t scope_lookup
  (m2c_symtab_scope_t scope, const char *ident, m2c_hash_t key) {
  
  uint_t index;
  m2c_symbol_t this_symbol;
    
  /* determine bucket index */
  index = key % scope->bucket_count;
  
  this_symbol = scope->bucket[index];
  
  while ((this_symbol != NULL) && (this_symbol->ident != ident)) {
    this_symbol = this_symbol->next;
  } /* end while */
  
  return this_symbol;
} /* end scope_lookup */


/* --------------------------------------------------------------------------
 * private function new_symbol(ident, kind, type_id, defn)
 * --------------------------------------------------------------------------
 * Returns a newly allocated symbol, initialised with parameters ident, kind,
 * type_id and defn.  Returns NULL if allocation fails.
 * ----------------------------------------------------------------------- */

static m2c_symbol_t new_symbol
  (const char *ident,
   m2c_symtype_t kind,
   const char *type_id,
   m2c_astnode_t defn) {
  
  m2c_symbol_t new_sym;
  
  new_sym = malloc(sizeof(m2c_symbol_s));
  
  if (new_sym == NULL) {
    return NULL;
  } /* end if */
  
  new_sym->next = NULL;
  new_sym->ident = ident;
  new_sym->kind = kind;
  new_sym->type_id = type_id;
  new_sym->definition = defn;
  
  return new_sym;
} /* end new_symbol */


/* --------------------------------------------------------------------------
 * private function remove_scope(symtab, scope)
 * --------------------------------------------------------------------------
 * Deallocates a given scope.  Updates counters of symtab.
 * ----------------------------------------------------------------------- */

static void remove_scope (m2c_symtab_t symtab, m2c_symtab_scope_t scope) {
  
  uint_t index;
  m2c_symbol_t this_symbol, next_symbol;
  
  for (index = 0; index < scope->bucket_count; index++) {
    /* get first symbol slot in bucket */
    this_symbol = scope->bucket[index];
    
    /* remove all entries in bucket */
    while (this_symbol != NULL) {
      next_symbol = this_symbol->next;
      free(this_symbol);
      symtab->symbol_count--;
      this_symbol = next_symbol;
    } /* end while */
    
    /* clear first symbol slot */
    scope->bucket[index] = NULL;
  } /* end for */
  
  free(scope);
  symtab->scope_count--;
} /* end remove_scope */

/* END OF FILE */