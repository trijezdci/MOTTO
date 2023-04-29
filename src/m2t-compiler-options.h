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
 * m2t-compiler-options.h
 *
 * Public interface for M2T compiler options.
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

#ifndef M2T_COMPILER_OPTIONS_H
#define M2T_COMPILER_OPTIONS_H

#include <stdbool.h>


/* --------------------------------------------------------------------------
 * type m2t_option_status_t
 * --------------------------------------------------------------------------
 * Status codes for operation m2t_get_cli_args.
 * ----------------------------------------------------------------------- */

typedef enum {
  M2T_OPTION_STATUS_SUCCESS,
  M2T_OPTION_STATUS_HELP_REQUESTED,
  M2T_OPTION_STATUS_VERSION_REQUESTED,
  M2T_OPTION_STATUS_FAILURE
} m2t_option_status_t;


/* --------------------------------------------------------------------------
 * function m2t_get_cli_args(argc, argv, status)
 * --------------------------------------------------------------------------
 * Processes command line arguments passed in argv, sets option flags and
 * returns a pointer to the argument that contains the input filename.
 * ----------------------------------------------------------------------- */

const char *m2t_get_cli_args
  (int argc, char *argv[], m2t_option_status_t *status);


/* --------------------------------------------------------------------------
 * procedure m2t_print_options()
 * --------------------------------------------------------------------------
 * Prints the current option values to the console.
 * ----------------------------------------------------------------------- */

void m2t_print_options(void);


/* --------------------------------------------------------------------------
 * procedure m2t_print_option_help()
 * --------------------------------------------------------------------------
 * Prints the option help to the console.
 * ----------------------------------------------------------------------- */

void m2t_print_option_help(void);


/* --------------------------------------------------------------------------
 * function m2t_option_verbose()
 * --------------------------------------------------------------------------
 * Returns true if option flag verbose is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_verbose (void);


/* --------------------------------------------------------------------------
 * function m2t_option_synonyms()
 * --------------------------------------------------------------------------
 * Returns true if option flag synonyms is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_synonyms (void);


/* --------------------------------------------------------------------------
 * function m2t_option_octal_literals()
 * --------------------------------------------------------------------------
 * Returns true if option flag octal_literals is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_octal_literals (void);


/* --------------------------------------------------------------------------
 * function m2t_option_escape_tab_and_newline()
 * --------------------------------------------------------------------------
 * Returns true if option flag escape_tab_and_newline is set, else false.
 * ----------------------------------------------------------------------- */

bool m2t_option_escape_tab_and_newline (void);


/* --------------------------------------------------------------------------
 * function m2t_option_subtype_cardinals()
 * --------------------------------------------------------------------------
 * Returns true if option flag subtype_cardinals is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_subtype_cardinals (void);


/* --------------------------------------------------------------------------
 * function m2t_option_safe_string_termination()
 * --------------------------------------------------------------------------
 * Returns true if option flag safe_string_termination is set, else false.
 * ----------------------------------------------------------------------- */

bool m2t_option_safe_string_termination (void);


/* --------------------------------------------------------------------------
 * function m2t_option_errant_semicolon()
 * --------------------------------------------------------------------------
 * Returns true if option flag errant_semicolon is set, else false.
 * ----------------------------------------------------------------------- */

bool m2t_option_errant_semicolon (void);


/* --------------------------------------------------------------------------
 * function m2t_option_byte()
 * --------------------------------------------------------------------------
 * Returns true if option flag for type BYTE is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_byte (void);


/* --------------------------------------------------------------------------
 * function m2t_option_longcard()
 * --------------------------------------------------------------------------
 * Returns true if option flag for type LONGCARD is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_longcard (void);


/* --------------------------------------------------------------------------
 * function m2t_option_unified_cast()
 * --------------------------------------------------------------------------
 * Returns true if option flag unified_cast is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_unified_cast (void);


/* --------------------------------------------------------------------------
 * function m2t_option_coroutines()
 * --------------------------------------------------------------------------
 * Returns true if option flag coroutines is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_coroutines (void);


/* --------------------------------------------------------------------------
 * function m2t_option_variant_records()
 * --------------------------------------------------------------------------
 * Returns true if option flag variant_records is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_variant_records (void);


/* --------------------------------------------------------------------------
 * function m2t_option_local_modules()
 * --------------------------------------------------------------------------
 * Returns true if option flag local_modules is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_local_modules (void);


/* --------------------------------------------------------------------------
 * function m2t_option_lexer_debug()
 * --------------------------------------------------------------------------
 * Returns true if option flag lexer_debug is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_lexer_debug (void);


/* --------------------------------------------------------------------------
 * function m2t_option_parser_debug()
 * --------------------------------------------------------------------------
 * Returns true if option flag parser_debug is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_parser_debug (void);


#endif /* M2T_COMPILER_OPTIONS_H */

/* END OF FILE */