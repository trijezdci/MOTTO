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
 * m2-parser.h
 *
 * Public interface for M2C parser module.
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

#ifndef M2C_PARSER_H
#define M2C_PARSER_H

#include "m2-common.h"

#include "m2-ast.h"
#include "m2-stats.h"


/* --------------------------------------------------------------------------
 * type m2c_sourcetype_t
 * --------------------------------------------------------------------------
 * Enumeration representing type of input source.
 * ----------------------------------------------------------------------- */

typedef enum {
  M2C_ANY_SOURCE,
  M2C_DEF_SOURCE,
  M2C_MOD_SOURCE
} m2c_sourcetype_t;

#define M2C_FIRST_SOURCETYPE M2C_ANY_SOURCE
#define M2C_LAST_SOURCETYPE M2C_MOD_SOURCE


/* --------------------------------------------------------------------------
 * type m2c_parser_status_t
 * --------------------------------------------------------------------------
 * Status codes for operations on type m2c_parser_t.
 * ----------------------------------------------------------------------- */

typedef enum {
  M2C_PARSER_STATUS_SUCCESS,
  M2C_PARSER_STATUS_INVALID_REFERENCE,
  M2C_PARSER_STATUS_INVALID_SOURCETYPE,
  M2C_PARSER_STATUS_ALLOCATION_FAILED
} m2c_parser_status_t;


/* --------------------------------------------------------------------------
 * function m2c_parse_file(srctype, srcpath, ast, stats, status)
 * --------------------------------------------------------------------------
 * Parses a Modula-2 source file represented by srcpath and returns status.
 * Builds an abstract syntax tree and passes it back in ast, or NULL upon
 * failure.  Collects simple statistics and passes them back in stats.
 * ----------------------------------------------------------------------- */
 
 void m2c_parse_file
   (m2c_sourcetype_t srctype,      /* in */
    const char *srcpath,           /* in */
    m2c_ast_t *ast,                /* out */
    m2c_stats_t *stats,            /* out */
    m2c_parser_status_t *status);  /* out */

#endif /* M2C_PARSER_H */

/* END OF FILE */