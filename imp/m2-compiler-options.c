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

#include "m2-compiler-options.h"

#include "m2-common.h"
#include "m2-error.h"

#include <stdio.h>


/* --------------------------------------------------------------------------
 * type m2c_compiler_options_struct_t
 * --------------------------------------------------------------------------
 * hidden type for compiler option flags
 * ----------------------------------------------------------------------- */

typedef struct {
  bool verbose;
  bool synonyms;
  bool line_comments;
  bool prefix_literals;
  bool octal_literals;
  bool escape_tab_and_newline;
  bool subtype_cardinals;
  bool safe_string_termination;
  bool errant_semicolon;
  bool lowline_in_identifiers;
  bool const_parameters;
  bool additional_types;
  bool unified_conversion;
  bool unified_cast;
  bool coroutines;
  bool variant_records;
  bool local_modules;
  bool lexer_debug;
  bool parser_debug;
} m2c_compiler_options_struct_t;


/* --------------------------------------------------------------------------
 * Default option flags
 * ----------------------------------------------------------------------- */

#define M2C_DEFAULT_OPTIONS { \
  /* verbose */ false, \
  /* synonyms */ false, \
  /* line-comments */ true, \
  /* prefix-literals */ true, \
  /* octal-literals */ false, \
  /* escape-tab-and-newline */ true, \
  /* subtype-cardinals */ false, \
  /* safe-string-termination */ true, \
  /* errant-semicolon */ false, \
  /* lowline-identifiers */ true, \
  /* const-parameters */ true, \
  /* additional-types */ true, \
  /* unified-conversion */ true, \
  /* unified-cast */ true, \
  /* coroutines  */ false, \
  /* variant-records */ false, \
  /* local-modules */ false, \
  /* lexer-debug */ false, \
  /* parser-debug */ false \
} /* default_options */

#define M2C_PIM3_OPTIONS { \
  /* verbose */ false, \
  /* synonyms */ true, \
  /* line-comments */ false, \
  /* prefix-literals */ false, \
  /* octal-literals */ true, \
  /* escape-tab-and-newline */ false, \
  /* subtype-cardinals */ false, \
  /* safe-string-termination */ false, \
  /* errant-semicolon */ false, \
  /* lowline-identifiers */ false, \
  /* const-parameters */ false, \
  /* additional-types */ false, \
  /* unified-conversion */ false, \
  /* unified-cast */ false, \
  /* coroutines  */ true, \
  /* variant-records */ true, \
  /* local-modules */ true, \
  /* lexer-debug */ false, \
  /* parser-debug */ false \
} /* default_options */

#define M2C_PIM4_OPTIONS { \
  /* verbose */ false, \
  /* synonyms */ true, \
  /* line-comments */ false, \
  /* prefix-literals */ false, \
  /* octal-literals */ true, \
  /* escape-tab-and-newline */ false, \
  /* subtype-cardinals */ true, \
  /* safe-string-termination */ true, \
  /* errant-semicolon */ false, \
  /* lowline-identifiers */ false, \
  /* const-parameters */ false, \
  /* additional-types */ false, \
  /* unified-conversion */ false, \
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
 * Option flags for current, pim3 and pim4
 * ----------------------------------------------------------------------- */

static m2c_compiler_options_struct_t options = M2C_DEFAULT_OPTIONS;

static m2c_compiler_options_struct_t pim3_options = M2C_PIM3_OPTIONS;

static m2c_compiler_options_struct_t pim4_options = M2C_PIM4_OPTIONS;


/* --------------------------------------------------------------------------
 * forward declarations
 * ----------------------------------------------------------------------- */

static void print_bool (bool expr);

static bool opt_match (const char* str1, const char* str2);

static void report_invalid_option (const char *optstr);


/* --------------------------------------------------------------------------
 * function m2c_get_cli_args(argc, argv, status)
 * --------------------------------------------------------------------------
 * Processes command line arguments passed in argv, sets option flags and
 * returns a pointer to the argument that contains the input filename.
 * ----------------------------------------------------------------------- */

const char *m2c_get_cli_args
  (int argc, char *argv[], m2c_option_status_t *status) {
  
  const char *filename, *optstr;
  uint_t index, error_count;
  bool permit_pim_option = true;
  bool permit_non_pim_option = true;
  
  if ((argc < 2) || (argv == NULL)) {
    m2c_emit_error(M2C_ERROR_MISSING_FILENAME);
    SET_STATUS(status, M2C_OPTION_STATUS_FAILURE);
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
      SET_STATUS(status, M2C_OPTION_STATUS_HELP_REQUESTED);
      return NULL;
    }
    else if (opt_match(optstr, "-V") || opt_match(optstr, "--version")) {
      SET_STATUS(status, M2C_OPTION_STATUS_VERSION_REQUESTED);
      return NULL;
    }
    else {
      m2c_emit_error(M2C_ERROR_MISSING_FILENAME);
      SET_STATUS(status, M2C_OPTION_STATUS_FAILURE);
      return NULL;
    } /* end if */
  }
  else /* argc > 2 */ {
  
    if (optstr[0] != '-') {
      filename = optstr;
    }
    else {
      m2c_emit_error(M2C_ERROR_MISSING_FILENAME);
      SET_STATUS(status, M2C_OPTION_STATUS_FAILURE);
      return NULL;
    } /* end if */
    
    /* process option arguments */    
    index = 2;
    while (index < argc) {
      optstr = argv[index];
      
      if (opt_match(optstr, "-v") || opt_match(optstr, "--verbose")) {
        options.verbose = true;
        pim3_options.verbose = true;
        pim4_options.verbose = true;
      }
      else if (opt_match(optstr, "--errant-semicolon")) {
        options.errant_semicolon = true;
        pim3_options.errant_semicolon = true;
        pim4_options.errant_semicolon = true;
      }
      else if (opt_match(optstr, "--no-errant-semicolon")) {
        options.errant_semicolon = false;
        pim3_options.errant_semicolon = false;
        pim4_options.errant_semicolon = false;
      }
      else if (opt_match(optstr, "--lexer-debug")) {
        options.lexer_debug = true;
        pim3_options.lexer_debug = true;
        pim4_options.lexer_debug = true;
      }
      else if (opt_match(optstr, "--parser-debug")) {
        options.parser_debug = true;
        pim3_options.parser_debug = true;
        pim4_options.parser_debug = true;
      }
      else if ((permit_pim_option) && (opt_match(optstr, "--pim3"))) {
        options = pim3_options;
        permit_pim_option = false;
        permit_non_pim_option = false;
      }
      else if ((permit_pim_option) && (opt_match(optstr, "--pim4"))) {
        options = pim4_options;
        permit_pim_option = false;
        permit_non_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--synonyms"))) {
        options.synonyms = true;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-synonyms"))) {
        options.synonyms = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--line-comments"))) {
            options.line_comments = true;
            permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-line-comments"))) {
        options.line_comments = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--prefix-literals"))) {
        options.prefix_literals = true;
        options.octal_literals = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--suffix-literals"))) {
        options.prefix_literals = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (options.prefix_literals == false) &&
               (opt_match(optstr, "--octal-literals"))) {
        options.octal_literals = true;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-octal-literals"))) {
        options.octal_literals = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--escape-tab-and-newline"))) {
        options.escape_tab_and_newline = true;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-escape-tab-and-newline"))) {
        options.escape_tab_and_newline = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--subtype-cardinals"))) {
        options.subtype_cardinals = true;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-subtype-cardinals"))) {
        options.subtype_cardinals = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--safe-string-termination"))) {
        options.safe_string_termination = true;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-safe-string-termination"))) {
        options.safe_string_termination = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--lowline"))) {
        options.lowline_in_identifiers = true;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-lowline"))) {
        options.lowline_in_identifiers = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--const-parameters"))) {
        options.const_parameters = true;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-const-parameters"))) {
        options.const_parameters = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--additional-types"))) {
        options.additional_types = true;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-additional-types"))) {
        options.additional_types = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--unified-conversion"))) {
        options.unified_conversion = true;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-unified-conversion"))) {
        options.unified_conversion = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--unified-cast"))) {
        options.unified_cast = true;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-unified-cast"))) {
        options.unified_cast = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--coroutines"))) {
        options.coroutines = true;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-coroutines"))) {
        options.coroutines = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--variant-records"))) {
        options.variant_records = true;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-variant-records"))) {
        options.variant_records = false;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--local-modules"))) {
        options.local_modules = true;
        permit_pim_option = false;
      }
      else if ((permit_non_pim_option) &&
               (opt_match(optstr, "--no-local-modules"))) {
        options.local_modules = false;
        permit_pim_option = false;
      }
      else {
        report_invalid_option(optstr);
        error_count++;
      } /* end if */
      index++;
    } /* end while */
  } /* end if */
  
  if (error_count > 0) {
    SET_STATUS(status, M2C_OPTION_STATUS_FAILURE);
    return NULL;
  }
  else /* no errors */ {
    SET_STATUS(status, M2C_OPTION_STATUS_SUCCESS);
    return filename;
  } /* end */
} /* end m2c_get_cli_args */


/* --------------------------------------------------------------------------
 * procedure m2c_print_options()
 * --------------------------------------------------------------------------
 * Prints the current option values to the console.
 * ----------------------------------------------------------------------- */

void m2c_print_options(void) {
  printf("option settings:\n");
  printf(" verbose: ");
    print_bool(options.verbose); printf("\n");
  printf(" synonyms: ");
    print_bool(options.synonyms); printf("\n");
  printf(" line-comments: ");
    print_bool(options.line_comments); printf("\n");
  printf(" prefix-literals: ");
    print_bool(options.prefix_literals); printf("\n");
  printf(" octal-literals: ");
    print_bool(options.octal_literals); printf("\n");
  printf(" escape-tab-and-newline: ");
    print_bool(options.escape_tab_and_newline); printf("\n");
  printf(" subtype-cardinals: ");
    print_bool(options.subtype_cardinals); printf("\n");
  printf(" safe-string-termination: ");
    print_bool(options.safe_string_termination); printf("\n");
  printf(" errant-semicolon: ");
    print_bool(options.errant_semicolon); printf("\n");
  printf(" lowline: ");
    print_bool(options.lowline_in_identifiers); printf("\n");
  printf(" const-parameters: ");
    print_bool(options.const_parameters); printf("\n");
  printf(" additional-types: ");
    print_bool(options.additional_types); printf("\n");
  printf(" unified-conversion: ");
    print_bool(options.unified_conversion); printf("\n");
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
} /* end m2c_print_options */


/* --------------------------------------------------------------------------
 * procedure m2c_print_option_help()
 * --------------------------------------------------------------------------
 * Prints the option help to the console.
 * ----------------------------------------------------------------------- */

void m2c_print_option_help(void) {
  printf("options:\n");
  printf("-h or --help\n");
  printf(" print this help\n");
  printf("-V or --version\n");
  printf(" print version info\n");
  printf("-v or --verbose\n");
  printf(" enable verbose diagnostics\n");
  printf("--errant-semicolon or --no-errant-semicolon\n");
  printf(" treat semicolon after statement sequence as warning or error\n");
  printf("--pim3 and --pim4\n");
  printf(" strictly follow PIM third or fourth edition\n");
  printf(" mutually exclusive with each other and all options below\n");
  printf("--synonyms and --no-synonyms\n");
  printf(" allow or disallow use of lexical synonyms ~, & and <>\n");
  printf("--line-comments and --no-line-comments\n");
  printf(" allow or disallow line comments\n");
  printf("--prefix-literals and --suffix-literals\n");
  printf(" allow use of either prefixed or suffixed number literals\n");
  printf("--octal-literals and --no-octal-literals\n");
  printf(" allow or disallow octal literals, only with --suffix-literals\n");
  printf("--escape-tab-and-newline and --no-escape-tab-and-newline\n");
  printf(" interpret '\\\\', '\\t' and '\\n' in string literals or not\n");
  printf("--subtype-cardinals and --no-subtype-cardinals\n");
  printf(" cardinal types are subtypes of integers or not\n");
  printf("--safe-string-termination and --no-safe-string-termination\n");
  printf(" enforce ASCII-NUL termination for all strings or not\n");
  printf("--lowline and --no-lowline\n");
  printf(" allow or disallow lowline characters in identifiers\n");
  printf("--const-parameters and --no-const-parameters\n");
  printf(" allow or disallow CONST attribute in formal parameters\n");
  printf("--additional-types and --no-additional-types\n");
  printf(" support for BYTE, SHORTCARD, SHORTINT, SHORTREAL and LONGCARD\n");
  printf("--unified-conversion and --no-unified-conversion\n");
  printf(" support for unified conversion function vs separate functions\n");
  printf("--unified-cast and --no-unified-cast\n");
  printf(" support for unified cast function vs separate functions\n");
  printf("--coroutines and --no-coroutines\n");
  printf(" enable or disable coroutine support\n");
  printf("--variant-records and --no-variant-records\n");
  printf(" enable or disable variant record support\n");
  printf("--local-modules and --no-local-modules\n");
  printf(" enable or disable local module support\n");
} /* end m2c_print_option_help */


/* --------------------------------------------------------------------------
 * function m2c_option_verbose()
 * --------------------------------------------------------------------------
 * Returns true if option flag verbose is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_verbose (void) {
  return options.verbose;
} /* end m2c_option_verbose */


/* --------------------------------------------------------------------------
 * function m2c_option_synonyms()
 * --------------------------------------------------------------------------
 * Returns true if option flag synonyms is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_synonyms (void) {
  return options.synonyms;
} /* end m2c_option_synonyms */


/* --------------------------------------------------------------------------
 * function m2c_option_line_comments()
 * --------------------------------------------------------------------------
 * Returns true if option flag line_comments is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_line_comments (void) {
  return options.line_comments;
} /* end m2c_option_line_comments */


/* --------------------------------------------------------------------------
 * function m2c_option_prefix_literals()
 * --------------------------------------------------------------------------
 * Returns true if option flag prefix_literals is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_prefix_literals (void) {
  return options.prefix_literals;
} /* end m2c_option_prefix_literals */


/* --------------------------------------------------------------------------
 * function m2c_option_suffix_literals()
 * --------------------------------------------------------------------------
 * Returns true if option flag prefix_literals is unset, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_suffix_literals (void) {
  return !(options.prefix_literals);
} /* end m2c_option_suffix_literals */


/* --------------------------------------------------------------------------
 * function m2c_option_octal_literals()
 * --------------------------------------------------------------------------
 * Returns true if option flag octal_literals is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_octal_literals (void) {
  return options.octal_literals;
} /* end m2c_option_octal_literals */


/* --------------------------------------------------------------------------
 * function m2c_option_escape_tab_and_newline()
 * --------------------------------------------------------------------------
 * Returns true if option flag escape_tab_and_newline is set, else false.
 * ----------------------------------------------------------------------- */

bool m2c_option_escape_tab_and_newline (void) {
  return options.escape_tab_and_newline;
} /* end m2c_option_escape_tab_and_newline */


/* --------------------------------------------------------------------------
 * function m2c_option_subtype_cardinals()
 * --------------------------------------------------------------------------
 * Returns true if option flag subtype_cardinals is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_subtype_cardinals (void) {
  return options.subtype_cardinals;
} /* end m2c_option_subtype_cardinals */


/* --------------------------------------------------------------------------
 * function m2c_option_safe_string_termination()
 * --------------------------------------------------------------------------
 * Returns true if option flag safe_string_termination is set, else false.
 * ----------------------------------------------------------------------- */

bool m2c_option_safe_string_termination (void) {
  return options.safe_string_termination;
} /* end m2c_option_safe_string_termination */


/* --------------------------------------------------------------------------
 * function m2c_option_errant_semicolon()
 * --------------------------------------------------------------------------
 * Returns true if option flag errant_semicolon is set, else false.
 * ----------------------------------------------------------------------- */

bool m2c_option_errant_semicolon (void) {
  return options.errant_semicolon;
} /* end m2c_option_errant_semicolon */


/* --------------------------------------------------------------------------
 * function m2c_option_lowline_identifiers()
 * --------------------------------------------------------------------------
 * Returns true if option flag lowline_identifiers is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_lowline_identifiers (void) {
  return options.lowline_in_identifiers;
} /* end m2c_option_lowline_identifiers */


/* --------------------------------------------------------------------------
 * function m2c_option_const_parameters()
 * --------------------------------------------------------------------------
 * Returns true if option flag const_parameters is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_const_parameters (void) {
  return options.const_parameters;
} /* end m2c_option_const_parameters */


/* --------------------------------------------------------------------------
 * function m2c_option_additional_types()
 * --------------------------------------------------------------------------
 * Returns true if option flag additional_types is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_additional_types (void) {
  return options.additional_types;
} /* end m2c_option_additional_types */


/* --------------------------------------------------------------------------
 * function m2c_option_unified_conversion()
 * --------------------------------------------------------------------------
 * Returns true if option flag unified_conversion is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_unified_conversion (void) {
  return options.unified_conversion;
} /* end m2c_option_unified_conversion */


/* --------------------------------------------------------------------------
 * function m2c_option_unified_cast()
 * --------------------------------------------------------------------------
 * Returns true if option flag unified_cast is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_unified_cast (void) {
  return options.unified_cast;
} /* end m2c_option_unified_cast */


/* --------------------------------------------------------------------------
 * function m2c_option_coroutines()
 * --------------------------------------------------------------------------
 * Returns true if option flag coroutines is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_coroutines (void) {
  return options.coroutines;
} /* end m2c_option_coroutines */


/* --------------------------------------------------------------------------
 * function m2c_option_variant_records()
 * --------------------------------------------------------------------------
 * Returns true if option flag variant_records is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_variant_records (void) {
  return options.variant_records;
} /* end m2c_option_variant_records */


/* --------------------------------------------------------------------------
 * function m2c_option_local_modules()
 * --------------------------------------------------------------------------
 * Returns true if option flag local_modules is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_local_modules (void) {
  return options.local_modules;
} /* end m2c_option_local_modules */


/* --------------------------------------------------------------------------
 * function m2c_option_lexer_debug()
 * --------------------------------------------------------------------------
 * Returns true if option flag lexer_debug is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_lexer_debug (void) {
  return options.lexer_debug;
} /* end m2c_option_lexer_debug */


/* --------------------------------------------------------------------------
 * function m2c_option_parser_debug()
 * --------------------------------------------------------------------------
 * Returns true if option flag parser_debug is set, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_option_parser_debug (void) {
  return options.parser_debug;
} /* end m2c_option_parser_debug */


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
    m2c_emit_error_w_str(M2C_ERROR_INVALID_OPTION, optstr);
  }
  else {
    m2c_emit_error_w_str(M2C_ERROR_INVALID_ARGUMENT, optstr);
  } /* end if */
} /* end report_invalid_option */

/* END OF FILE */