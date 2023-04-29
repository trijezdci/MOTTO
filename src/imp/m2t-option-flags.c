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
 * m2t-option-flags.c
 *
 * Implementation of M2T option flags.
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

#include "m2t-options-flags.h"

#include "m2t-common.h"
#include "m2t-error.h"

#include <stdio.h>


/* --------------------------------------------------------------------------
 * type m2t_compiler_options_struct_t
 * --------------------------------------------------------------------------
 * hidden type for compiler option flags
 * ----------------------------------------------------------------------- */

typedef struct {
  bool verbose;
  bool synonyms;
  bool octal_literals;
  bool escape_tab_and_newline;
  bool export_lists;
  bool subtype_cardinals;
  bool safe_string_termination;
  bool errant_semicolon;
  bool type_byte;
  bool type_longcard;
  bool unified_cast;
  bool coroutines;
  bool variant_records;
  bool local_modules;
  bool lexer_debug;
  bool parser_debug;
} m2t_compiler_options_struct_t;


/* --------------------------------------------------------------------------
 * Default option flags
 * ----------------------------------------------------------------------- */

#define M2T_DEFAULT_OPTIONS { \
  /* verbose */ false, \
  /* synonyms */ false, \
  /* octal-literals */ false, \
  /* escape-tab-and-newline */ true, \
  /* export_lists */ false, \
  /* subtype-cardinals */ false, \
  /* safe-string-termination */ true, \
  /* errant-semicolon */ false, \
  /* type_byte */ true, \
  /* type_longcard */ true, \
  /* unified-cast */ true, \
  /* coroutines  */ false, \
  /* variant-records */ false, \
  /* local-modules */ false, \
  /* lexer-debug */ false, \
  /* parser-debug */ false \
} /* default_options */

#define M2T_PIM2_OPTIONS { \
  /* verbose */ false, \
  /* synonyms */ true, \
  /* octal-literals */ true, \
  /* escape-tab-and-newline */ false, \
  /* export_lists */ true, \
  /* subtype-cardinals */ false, \
  /* safe-string-termination */ false, \
  /* errant-semicolon */ false, \
  /* type_byte */ false, \
  /* type_longcard */ false, \
  /* unified-cast */ false, \
  /* coroutines  */ true, \
  /* variant-records */ true, \
  /* local-modules */ true, \
  /* lexer-debug */ false, \
  /* parser-debug */ false \

#define M2T_PIM3_OPTIONS { \
  /* verbose */ false, \
  /* synonyms */ true, \
  /* octal-literals */ true, \
  /* escape-tab-and-newline */ false, \
  /* export_lists */ false, \
  /* subtype-cardinals */ false, \
  /* safe-string-termination */ false, \
  /* errant-semicolon */ false, \
  /* type_byte */ false, \
  /* type_longcard */ false, \
  /* unified-cast */ false, \
  /* coroutines  */ true, \
  /* variant-records */ true, \
  /* local-modules */ true, \
  /* lexer-debug */ false, \
  /* parser-debug */ false \
} /* default_options */

#define M2T_PIM4_OPTIONS { \
  /* verbose */ false, \
  /* synonyms */ true, \
  /* octal-literals */ true, \
  /* escape-tab-and-newline */ false, \
  /* export_lists */ false, \
  /* subtype-cardinals */ true, \
  /* safe-string-termination */ true, \
  /* errant-semicolon */ false, \
  /* type_byte */ false, \
  /* type_longcard */ false, \
  /* unified-cast */ false, \
  /* coroutines  */ true, \
  /* variant-records */ true, \
  /* local-modules */ true, \
  /* lexer-debug */ false, \
  /* parser-debug */ false \
} /* default_options */


/* --------------------------------------------------------------------------
 * hidden variable options
 * --------------------------------------------------------------------------
 * Option flags for current, pim2, pim3 and pim4
 * ----------------------------------------------------------------------- */

static m2t_compiler_options_struct_t options = M2T_DEFAULT_OPTIONS;

static m2t_compiler_options_struct_t pim2_options = M2T_PIM2_OPTIONS;

static m2t_compiler_options_struct_t pim3_options = M2T_PIM3_OPTIONS;

static m2t_compiler_options_struct_t pim4_options = M2T_PIM4_OPTIONS;


/* --------------------------------------------------------------------------
 * forward declarations
 * ----------------------------------------------------------------------- */

static void print_bool (bool expr);

static bool opt_match (const char* str1, const char* str2);

static void report_invalid_option (const char *optstr);


/* --------------------------------------------------------------------------
 * function m2t_get_cli_args(argc, argv, status)
 * --------------------------------------------------------------------------
 * Processes command line arguments passed in argv, sets option flags and
 * returns a pointer to the argument that contains the input filename.
 * ----------------------------------------------------------------------- */

const char *m2t_get_cli_args
  (int argc, char *argv[], m2t_option_status_t *status) {
  
  const char *filename, *optstr;
  uint_t index, error_count;
  bool no_dialect_set = true;
  
  if ((argc < 2) || (argv == NULL)) {
    m2t_emit_error(M2T_ERROR_MISSING_FILENAME);
    SET_STATUS(status, M2T_OPTION_STATUS_FAILURE);
    return NULL;
  } /* end if */
  
  /* init error count */
  error_count = 0;
  
  /* get first command line argument */
  optstr = argv[1];
  
  /* get cli arguments */
  if (argc == 2) {
    
    if (optstr[0] != '-') {
      filename = optstr;
    }
    else if (opt_match(optstr, "-h") || opt_match(optstr, "--help")) {
      SET_STATUS(status, M2T_OPTION_STATUS_HELP_REQUESTED);
      return NULL;
    }
    else if (opt_match(optstr, "-V") || opt_match(optstr, "--version")) {
      SET_STATUS(status, M2T_OPTION_STATUS_VERSION_REQUESTED);
      return NULL;
    }
    else {
      m2t_emit_error(M2T_ERROR_MISSING_FILENAME);
      SET_STATUS(status, M2T_OPTION_STATUS_FAILURE);
      return NULL;
    } /* end if */
  }
  else /* argc > 2 */ {
  
    if (optstr[0] != '-') {
      filename = optstr;
    }
    else {
      m2t_emit_error(M2T_ERROR_MISSING_FILENAME);
      SET_STATUS(status, M2T_OPTION_STATUS_FAILURE);
      return NULL;
    } /* end if */
    
    /* process option arguments */    
    index = 2;
    while (index < argc) {
      optstr = argv[index];
      
      if (opt_match(optstr, "-v") || opt_match(optstr, "--verbose")) {
        options.verbose = true;
        pim2_options.verbose = true;
        pim3_options.verbose = true;
        pim4_options.verbose = true;
      }
      else if (opt_match(optstr, "--errant-semicolon")) {
        options.errant_semicolon = true;
        pim2_options.errant_semicolon = true;
        pim3_options.errant_semicolon = true;
        pim4_options.errant_semicolon = true;
      }
      else if (opt_match(optstr, "--no-errant-semicolon")) {
        options.errant_semicolon = false;
        pim2_options.errant_semicolon = false;
        pim3_options.errant_semicolon = false;
        pim4_options.errant_semicolon = false;
      }
      else if (opt_match(optstr, "--lexer-debug")) {
        options.lexer_debug = true;
        pim2_options.lexer_debug = true;
        pim3_options.lexer_debug = true;
        pim4_options.lexer_debug = true;
      }
      else if (opt_match(optstr, "--parser-debug")) {
        options.parser_debug = true;
        pim2_options.parser_debug = true;
        pim3_options.parser_debug = true;
        pim4_options.parser_debug = true;
      }
      else if ((permit_pim_option) && (opt_match(optstr, "--pim2"))) {
        options = pim2_options;
        no_dialect_set = false;
      }
      else if ((permit_pim_option) && (opt_match(optstr, "--pim3"))) {
        options = pim3_options;
        no_dialect_set = false;
     }
      else if ((permit_pim_option) && (opt_match(optstr, "--pim4"))) {
        options = pim4_options;
        no_dialect_set = false;
      }
      else if (opt_match(optstr, "--synonyms")) {
        options.synonyms = true;
      }
      else if (opt_match(optstr, "--no-synonyms")) {
        options.synonyms = false;
      }
      else if (opt_match(optstr, "--octal-literals")) {
        options.octal_literals = true;
      }
      else if (opt_match(optstr, "--no-octal-literals")) {
        options.octal_literals = false;
      }
      else if (opt_match(optstr, "--escape-tab-and-newline")) {
        options.escape_tab_and_newline = true;
      }
      else if (opt_match(optstr, "--no-escape-tab-and-newline")) {
        options.escape_tab_and_newline = false;
      }
      else if ((no_dialect_set) &&
               (opt_match(optstr, "--export-lists"))) {
        options.export_lists = true;
      }
      else if ((no_dialect_set) &&
               (opt_match(optstr, "--no-export-lists"))) {
        options.export_lists = false;
      }
       else if ((no_dialect_set) &&
               (opt_match(optstr, "--subtype-cardinals"))) {
        options.subtype_cardinals = true;
      }
      else if ((no_dialect_set) &&
               (opt_match(optstr, "--no-subtype-cardinals"))) {
        options.subtype_cardinals = false;
      }
      else if ((no_dialect_set) &&
               (opt_match(optstr, "--safe-string-termination"))) {
        options.safe_string_termination = true;
      }
      else if ((no_dialect_set) &&
               (opt_match(optstr, "--no-safe-string-termination"))) {
        options.safe_string_termination = false;
      }
      else if (opt_match(optstr, "--type-byte")) {
        options.type_byte = true;
      }
      else if (opt_match(optstr, "--no-type-byte")) {
        options.type_byte = false;
      }
      else if (opt_match(optstr, "--type-longcard")) {
        options.type_longcard = true;
      }
      else if (opt_match(optstr, "--no-type-longcard")) {
        options.type_longcard = false;
      }
      else if (opt_match(optstr, "--unified-cast"))) {
        options.unified_cast = true;
      }
      else if (opt_match(optstr, "--no-unified-cast")) {
        options.unified_cast = false;
      }
      else if (opt_match(optstr, "--coroutines")) {
        options.coroutines = true;
      }
      else if (opt_match(optstr, "--no-coroutines")) {
        options.coroutines = false;
      }
      else if (opt_match(optstr, "--variant-records")) {
        options.variant_records = true;
      }
      else if (opt_match(optstr, "--no-variant-records")) {
        options.variant_records = false;
      }
      else if (opt_match(optstr, "--local-modules")) {
        options.local_modules = true;
      }
      else if (opt_match(optstr, "--no-local-modules")) {
        options.local_modules = false;
      }
      else {
        report_invalid_option(optstr);
        error_count++;
      } /* end if */
      index++;
    } /* end while */
  } /* end if */
  
  if (error_count > 0) {
    SET_STATUS(status, M2T_OPTION_STATUS_FAILURE);
    return NULL;
  }
  else /* no errors */ {
    SET_STATUS(status, M2T_OPTION_STATUS_SUCCESS);
    return filename;
  } /* end */
} /* end m2t_get_cli_args */


/* --------------------------------------------------------------------------
 * procedure m2t_print_options()
 * --------------------------------------------------------------------------
 * Prints the current option values to the console.
 * ----------------------------------------------------------------------- */

void m2t_print_options(void) {
  printf("option settings:\n");
  printf(" verbose: ");
    print_bool(options.verbose); printf("\n");
  printf(" synonyms: ");
    print_bool(options.synonyms); printf("\n");
  printf(" octal-literals: ");
    print_bool(options.octal_literals); printf("\n");
  printf(" escape-tab-and-newline: ");
    print_bool(options.escape_tab_and_newline); printf("\n");
  printf(" export-list: ");
    print_bool(options.export_lists); printf("\n");
  printf(" subtype-cardinals: ");
    print_bool(options.subtype_cardinals); printf("\n");
  printf(" safe-string-termination: ");
    print_bool(options.safe_string_termination); printf("\n");
  printf(" errant-semicolon: ");
    print_bool(options.errant_semicolon); printf("\n");
  printf(" type-byte: ");
    print_bool(options.type_byte); printf("\n");
  printf(" type-longcard: ");
    print_bool(options.type_longcard); printf("\n");
  printf(" unified-cast: ");
    print_bool(options.unified_cast); printf("\n");
  printf(" coroutines: ");
    print_bool(options.coroutines); printf("\n");
  printf(" variant-records: ");
    print_bool(options.variant_records); printf("\n");
  printf(" local-modules: ");
    print_bool(options.local_modules); printf("\n");
  printf(" parser-debug: ");
    print_bool(options.parser_debug); printf("\n");
} /* end m2t_print_options */


/* --------------------------------------------------------------------------
 * procedure m2t_print_option_help()
 * --------------------------------------------------------------------------
 * Prints the option help to the console.
 * ----------------------------------------------------------------------- */

void m2t_print_option_help(void) {
  printf("options:\n");
  printf("-h or --help\n");
  printf(" print this help\n");
  printf("-V or --version\n");
  printf(" print version info\n");
  printf("-v or --verbose\n");
  printf(" enable verbose diagnostics\n");
  printf("--errant-semicolon or --no-errant-semicolon\n");
  printf(" treat semicolon after statement sequence as warning or error\n");
  printf("--pim2, --pim3 and --pim4\n");
  printf(" strictly follow PIM second, third or fourth edition\n");
  printf(" mutually exclusive with each other and all options below\n");
  printf("--synonyms and --no-synonyms\n");
  printf(" allow or disallow use of lexical synonyms ~, & and <>\n");
  printf("--octal-literals and --no-octal-literals\n");
  printf(" allow or disallow octal literals, only with --suffix-literals\n");
  printf("--escape-tab-and-newline and --no-escape-tab-and-newline\n");
  printf(" interpret '\\\\', '\\t' and '\\n' in string literals or not\n");
  printf("--export-lists and --no-export-lists\n");
  printf(" allow export lists in definition modules or not\n");
  printf("--subtype-cardinals and --no-subtype-cardinals\n");
  printf(" cardinal types are subtypes of integers or not\n");
  printf("--safe-string-termination and --no-safe-string-termination\n");
  printf(" enforce ASCII-NUL termination for all strings or not\n");
  printf("--type-byte and --no-type-byte\n");
  printf(" whether module SYSTEM provides type BYTE or not\n");
  printf("--type-longcard and --no-type-longcard\n");
  printf(" support for pervasive type LONGCARD\n");
  printf("--unified-cast and --no-unified-cast\n");
  printf(" support for unified cast function vs separate functions\n");
  printf("--coroutines and --no-coroutines\n");
  printf(" enable or disable coroutine support\n");
  printf("--variant-records and --no-variant-records\n");
  printf(" enable or disable variant record support\n");
  printf("--local-modules and --no-local-modules\n");
  printf(" enable or disable local module support\n");
} /* end m2t_print_option_help */


/* --------------------------------------------------------------------------
 * function m2t_option_verbose()
 * --------------------------------------------------------------------------
 * Returns true if option flag verbose is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_verbose (void) {
  return options.verbose;
} /* end m2t_option_verbose */


/* --------------------------------------------------------------------------
 * function m2t_option_synonyms()
 * --------------------------------------------------------------------------
 * Returns true if option flag synonyms is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_synonyms (void) {
  return options.synonyms;
} /* end m2t_option_synonyms */


/* --------------------------------------------------------------------------
 * function m2t_option_octal_literals()
 * --------------------------------------------------------------------------
 * Returns true if option flag octal_literals is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_octal_literals (void) {
  return options.octal_literals;
} /* end m2t_option_octal_literals */


/* --------------------------------------------------------------------------
 * function m2t_option_escape_tab_and_newline()
 * --------------------------------------------------------------------------
 * Returns true if option flag escape_tab_and_newline is set, else false.
 * ----------------------------------------------------------------------- */

bool m2t_option_escape_tab_and_newline (void) {
  return options.escape_tab_and_newline;
} /* end m2t_option_escape_tab_and_newline */


/* --------------------------------------------------------------------------
 * function m2t_option_export_lists()
 * --------------------------------------------------------------------------
 * Returns true if option flag export_lists is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_subtypem2t_option_export_list_cardinals (void) {
  return options.export-lists;
} /* end m2t_option_export_lists */


/* --------------------------------------------------------------------------
 * function m2t_option_subtype_cardinals()
 * --------------------------------------------------------------------------
 * Returns true if option flag subtype_cardinals is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_subtype_cardinals (void) {
  return options.subtype_cardinals;
} /* end m2t_option_subtype_cardinals */


/* --------------------------------------------------------------------------
 * function m2t_option_safe_string_termination()
 * --------------------------------------------------------------------------
 * Returns true if option flag safe_string_termination is set, else false.
 * ----------------------------------------------------------------------- */

bool m2t_option_safe_string_termination (void) {
  return options.safe_string_termination;
} /* end m2t_option_safe_string_termination */


/* --------------------------------------------------------------------------
 * function m2t_option_errant_semicolon()
 * --------------------------------------------------------------------------
 * Returns true if option flag errant_semicolon is set, else false.
 * ----------------------------------------------------------------------- */

bool m2t_option_errant_semicolon (void) {
  return options.errant_semicolon;
} /* end m2t_option_errant_semicolon */


/* --------------------------------------------------------------------------
 * function m2t_option_type_byte()
 * --------------------------------------------------------------------------
 * Returns true if option flag type_byte is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_type_byte (void) {
  return options.type_byte;
} /* end m2t_option_type_byte */


/* --------------------------------------------------------------------------
 * function m2t_option_type_longcard()
 * --------------------------------------------------------------------------
 * Returns true if option flag type_longcard is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_type_longcard (void) {
  return options.type_longcard;
} /* end m2t_option_type_longcard */


/* --------------------------------------------------------------------------
 * function m2t_option_unified_cast()
 * --------------------------------------------------------------------------
 * Returns true if option flag unified_cast is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_unified_cast (void) {
  return options.unified_cast;
} /* end m2t_option_unified_cast */


/* --------------------------------------------------------------------------
 * function m2t_option_coroutines()
 * --------------------------------------------------------------------------
 * Returns true if option flag coroutines is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_coroutines (void) {
  return options.coroutines;
} /* end m2t_option_coroutines */


/* --------------------------------------------------------------------------
 * function m2t_option_variant_records()
 * --------------------------------------------------------------------------
 * Returns true if option flag variant_records is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_variant_records (void) {
  return options.variant_records;
} /* end m2t_option_variant_records */


/* --------------------------------------------------------------------------
 * function m2t_option_local_modules()
 * --------------------------------------------------------------------------
 * Returns true if option flag local_modules is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_local_modules (void) {
  return options.local_modules;
} /* end m2t_option_local_modules */


/* --------------------------------------------------------------------------
 * function m2t_option_lexer_debug()
 * --------------------------------------------------------------------------
 * Returns true if option flag lexer_debug is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_lexer_debug (void) {
  return options.lexer_debug;
} /* end m2t_option_lexer_debug */


/* --------------------------------------------------------------------------
 * function m2t_option_parser_debug()
 * --------------------------------------------------------------------------
 * Returns true if option flag parser_debug is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2t_option_parser_debug (void) {
  return options.parser_debug;
} /* end m2t_option_parser_debug */


/* --------------------------------------------------------------------------
 * private procedure print_bool(expr)
 * --------------------------------------------------------------------------
 * Prints "on" to the console if expr is true, otherwise prints "off".
 * ----------------------------------------------------------------------- */

static void print_bool(bool expr) {
  if (expr) {
    printf("on");
  }
  else {
    printf("off");
  } /* end if */
} /* end print_bool */


/* --------------------------------------------------------------------------
 * private function opt_match(str1, str2)
 * --------------------------------------------------------------------------
 * Returns true if str1 matches str2, otherwise false.
 * ----------------------------------------------------------------------- */

static bool opt_match(const char* str1, const char* str2) {
  uint_t index = 0;
  
  while (true) {
    if (str1[index] != str2[index]) {
      return false;
    }
    else if (str1[index] == ASCII_NUL) {
      return true;
    }
    else {
      index++;
    } /* end if */
  } /* end while */
} /* end opt_match */


/* --------------------------------------------------------------------------
 * private procedure report_invalid_option(optstr)
 * --------------------------------------------------------------------------
 * Reports optstr as invalid option or argument.
 * ----------------------------------------------------------------------- */

static void report_invalid_option(const char *optstr) {
  if (optstr[0] == '-') {
    m2t_emit_error_w_str(M2T_ERROR_INVALID_OPTION, optstr);
  }
  else {
    m2t_emit_error_w_str(M2T_ERROR_INVALID_ARGUMENT, optstr);
  } /* end if */
} /* end report_invalid_option */

/* END OF FILE */