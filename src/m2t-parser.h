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
 * m2t-parser.h
 *
 * Public interface for M2T parser module.
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

#ifndef M2T_PARSER_H
#define M2T_PARSER_H

#include "m2t-common.h"
#include "ast/m2t-ast.h"


/* --------------------------------------------------------------------------
 * type m2t_sourcetype_t
 * --------------------------------------------------------------------------
 * Enumeration representing type of input source.
 * ----------------------------------------------------------------------- */

typedef enum {
  M2T_ANY_SOURCE,
  M2T_DEF_SOURCE,
  M2T_MOD_SOURCE
} m2t_sourcetype_t;

#define M2T_FIRST_SOURCETYPE M2T_ANY_SOURCE
#define M2T_LAST_SOURCETYPE M2T_MOD_SOURCE


/* --------------------------------------------------------------------------
 * type m2t_parser_status_t
 * --------------------------------------------------------------------------
 * Status codes for operations on type m2t_parser_t.
 * ----------------------------------------------------------------------- */

typedef enum {
  M2T_PARSER_STATUS_SUCCESS,
  M2T_PARSER_STATUS_INVALID_REFERENCE,
  M2T_PARSER_STATUS_INVALID_SOURCETYPE,
  M2T_PARSER_STATUS_ALLOCATION_FAILED
} m2t_parser_status_t;


/* --------------------------------------------------------------------------
 * function m2t_parse_file(srctype, srcpath, ast, stats, status)
 * --------------------------------------------------------------------------
 * Parses a Modula-2 source file represented by srcpath and returns status.
 * Builds an abstract syntax tree and passes it back in ast, or NULL upon
 * failure.  Collects simple statistics and passes them back in stats.
 * ----------------------------------------------------------------------- */
 
 void m2t_parse_file
   (m2t_sourcetype_t srctype,      /* in */
    const char *srcpath,           /* in */
    m2t_ast_t *ast,                /* out */
    m2t_stats_t *stats,            /* out */
    m2t_parser_status_t *status);  /* out */

#endif /* M2T_PARSER_H */

/* END OF FILE */