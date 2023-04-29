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
 * m2-compiler-options.h
 *
 * Public interface for M2C compiler options.
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

#ifndef M2C_COMPILER_OPTIONS_H
#define M2C_COMPILER_OPTIONS_H

#include <stdbool.h>


/* --------------------------------------------------------------------------
 * type m2c_option_status_t
 * --------------------------------------------------------------------------
 * Status codes for operation m2c_get_cli_args.
 * ----------------------------------------------------------------------- */

typedef enum {
  M2C_OPTION_STATUS_SUCCESS,
  M2C_OPTION_STATUS_HELP_REQUESTED,
  M2C_OPTION_STATUS_VERSION_REQUESTED,
  M2C_OPTION_STATUS_FAILURE
} m2c_option_status_t;


/* --------------------------------------------------------------------------
 * function m2c_get_cli_args(argc, argv, status)
 * --------------------------------------------------------------------------
 * Processes command line arguments passed in argv, sets option flags and
 * returns a pointer to the argument that contains the input filename.
 * ----------------------------------------------------------------------- */

const char *m2c_get_cli_args
  (int argc, char *argv[], m2c_option_status_t *status);


/* --------------------------------------------------------------------------
 * procedure m2c_print_options()
 * --------------------------------------------------------------------------
 * Prints the current option values to the console.
 * ----------------------------------------------------------------------- */

void m2c_print_options(void);


/* --------------------------------------------------------------------------
 * procedure m2c_print_option_help()
 * --------------------------------------------------------------------------
 * Prints the option help to the console.
 * ----------------------------------------------------------------------- */

void m2c_print_option_help(void);


/* --------------------------------------------------------------------------
 * function m2c_option_verbose()
 * --------------------------------------------------------------------------
 * Returns true if option flag verbose is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_verbose (void);


/* --------------------------------------------------------------------------
 * function m2c_option_synonyms()
 * --------------------------------------------------------------------------
 * Returns true if option flag synonyms is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_synonyms (void);


/* --------------------------------------------------------------------------
 * function m2c_option_line_comments()
 * --------------------------------------------------------------------------
 * Returns true if option flag line_comments is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_line_comments (void);


/* --------------------------------------------------------------------------
 * function m2c_option_prefix_literals()
 * --------------------------------------------------------------------------
 * Returns true if option flag prefix_literals is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_prefix_literals (void);


/* --------------------------------------------------------------------------
 * function m2c_option_suffix_literals()
 * --------------------------------------------------------------------------
 * Returns true if option flag prefix_literals is unset, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_suffix_literals (void);


/* --------------------------------------------------------------------------
 * function m2c_option_octal_literals()
 * --------------------------------------------------------------------------
 * Returns true if option flag octal_literals is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_octal_literals (void);


/* --------------------------------------------------------------------------
 * function m2c_option_escape_tab_and_newline()
 * --------------------------------------------------------------------------
 * Returns true if option flag escape_tab_and_newline is set, else false.
 * ----------------------------------------------------------------------- */

bool m2c_option_escape_tab_and_newline (void);


/* --------------------------------------------------------------------------
 * function m2c_option_subtype_cardinals()
 * --------------------------------------------------------------------------
 * Returns true if option flag subtype_cardinals is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_subtype_cardinals (void);


/* --------------------------------------------------------------------------
 * function m2c_option_safe_string_termination()
 * --------------------------------------------------------------------------
 * Returns true if option flag safe_string_termination is set, else false.
 * ----------------------------------------------------------------------- */

bool m2c_option_safe_string_termination (void);


/* --------------------------------------------------------------------------
 * function m2c_option_errant_semicolon()
 * --------------------------------------------------------------------------
 * Returns true if option flag errant_semicolon is set, else false.
 * ----------------------------------------------------------------------- */

bool m2c_option_errant_semicolon (void);


/* --------------------------------------------------------------------------
 * function m2c_option_lowline_identifiers()
 * --------------------------------------------------------------------------
 * Returns true if option flag lowline_identifiers is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_lowline_identifiers (void);


/* --------------------------------------------------------------------------
 * function m2c_option_const_parameters()
 * --------------------------------------------------------------------------
 * Returns true if option flag const_parameters is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_const_parameters (void);


/* --------------------------------------------------------------------------
 * function m2c_option_additional_types()
 * --------------------------------------------------------------------------
 * Returns true if option flag additional_types is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_additional_types (void);


/* --------------------------------------------------------------------------
 * function m2c_option_unified_conversion()
 * --------------------------------------------------------------------------
 * Returns true if option flag unified_conversion is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_unified_conversion (void);


/* --------------------------------------------------------------------------
 * function m2c_option_unified_cast()
 * --------------------------------------------------------------------------
 * Returns true if option flag unified_cast is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_unified_cast (void);


/* --------------------------------------------------------------------------
 * function m2c_option_coroutines()
 * --------------------------------------------------------------------------
 * Returns true if option flag coroutines is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_coroutines (void);


/* --------------------------------------------------------------------------
 * function m2c_option_variant_records()
 * --------------------------------------------------------------------------
 * Returns true if option flag variant_records is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_variant_records (void);


/* --------------------------------------------------------------------------
 * function m2c_option_local_modules()
 * --------------------------------------------------------------------------
 * Returns true if option flag local_modules is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_local_modules (void);


/* --------------------------------------------------------------------------
 * function m2c_option_lexer_debug()
 * --------------------------------------------------------------------------
 * Returns true if option flag lexer_debug is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_lexer_debug (void);


/* --------------------------------------------------------------------------
 * function m2c_option_parser_debug()
 * --------------------------------------------------------------------------
 * Returns true if option flag parser_debug is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_parser_debug (void);


#endif /* M2C_COMPILER_OPTIONS_H */

/* END OF FILE */