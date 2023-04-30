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
 * m2c.c
 *
 * M2C main program.
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

#include "fileutils.h"
#include "m2-lexer.h"
#include "m2-error.h"
#include "m2-parser.h"
#include "m2-pathnames.h"
#include "m2-unique-string.h"
#include "m2-compiler-options.h"

#include <stdio.h>
#include <stdlib.h>


#define M2C_IDENTIFICATION "m2c Modula-2 Compiler & Translator"

#define M2C_VERSION_INFO "version 1.00"

#define M2C_COPYRIGHT \
  "copyright (c) 2015-2016 B.Kowarsch"

#define M2C_LICENSE \
  "licensed under the GNU Lesser General Public License v.2 and v.3"


static void print_identification(void) {
  printf(M2C_IDENTIFICATION ", " M2C_VERSION_INFO "\n");
} /* end print_identification */


static void print_version(void) {
  printf(M2C_VERSION_INFO ", build (%05u)\n", M2C_BUILD_NUMBER);
} /* end print_version */


static void print_copyright(void) {
  printf(M2C_COPYRIGHT "\n");
} /* end print_copyright */


static void print_license(void) {
  printf(M2C_LICENSE "\n");
} /* end print_license */


static void print_usage(void) {
  printf("usage:\n");
  printf(" m2c sourcefile [options]\n");
} /* end print_usage */


static void exit_with_usage(void) {
  print_usage();
  printf(" or m2c -h for help\n");
  exit(EXIT_FAILURE);
} /* end exit_with_usage */


static void exit_with_help(void) {
  print_identification();
  print_copyright();
  print_license();
  print_usage();
  m2c_print_option_help();
  exit(EXIT_SUCCESS);
} /* end print_help */


static void exit_with_version(void) {
  print_version();
  exit(EXIT_SUCCESS);
} /* end exit_with_version */


int main (int argc, char *argv[]) {
  /* path of working directory */
  const char *workdir = NULL;
  
  /* full path to source file */
  const char *srcpath = NULL;

  /* directory path of srcpath */
  const char *dirpath = NULL;
  
  /* filename of srcpath */
  const char *filename = NULL;
  
  /* source file's base name excluding suffix */
  const char *basename = NULL;
  
  /* source file's suffix */
  const char *suffix = NULL;
  
  /* path to log file */
  const char *logpath = NULL;
  
  /* path to AST output file */
  const char *astpath = NULL;
  
  /* path to DOT output file */
  const char *dotpath = NULL;
  
  /* path to SYM output file */
  const char *sympath = NULL;
  
  /* path to C output file */
  const char *tgtpath = NULL;
  
  uint_t index;
  m2c_stats_t stats;
  m2c_option_status_t cli_status;
  m2c_parser_status_t parser_status;
  m2c_pathname_status_t pathname_status;
  
  if (argc < 2) {
    exit_with_usage();
  } /* end if */
  
  /* get command line arguments and filename */
  srcpath = m2c_get_cli_args(argc, argv, &cli_status);
  
  /* check for failure, help or version request */
  if (cli_status == M2C_OPTION_STATUS_FAILURE) {
    exit_with_usage();
  }
  else if (cli_status == M2C_OPTION_STATUS_HELP_REQUESTED) {
    exit_with_help();
  }
  else if (cli_status == M2C_OPTION_STATUS_VERSION_REQUESTED) {
    exit_with_version();
  } /* end if */
  
  /* check source path validity */
  if ((srcpath == NULL) || (srcpath[0] == ASCII_NUL)) {
    m2c_emit_error(M2C_ERROR_MISSING_FILENAME);
    exit(EXIT_FAILURE);
  } /* end if */
  
  /* get directory path and filename */
  pathname_status = split_pathname(srcpath, &dirpath, &filename, &index);
    
  if (pathname_status == M2C_PATHNAME_STATUS_INVALID_PATH) {
    m2c_emit_error_w_str(M2C_ERROR_INVALID_FILENAME, srcpath);
    exit(EXIT_FAILURE);
  } /* end if */
  
  if ((filename == NULL) || (filename[0] == ASCII_NUL)) {
    m2c_emit_error_w_str(M2C_ERROR_INVALID_FILENAME, srcpath);
    exit(EXIT_FAILURE);
  } /* end if */
  
  /* get basename and suffix */
  pathname_status = split_filename(filename, &basename, &suffix, &index);
  
  if (pathname_status == M2C_PATHNAME_STATUS_INVALID_FILENAME) {
    m2c_emit_error_w_str(M2C_ERROR_INVALID_FILENAME, filename);
    exit(EXIT_FAILURE);
  } /* end if */
  
  if ((basename == NULL) || (basename[0] == ASCII_NUL)) {
    m2c_emit_error_w_str(M2C_ERROR_INVALID_FILENAME, filename);
    exit(EXIT_FAILURE);
  } /* end if */
    
  /* check suffix validity */
  if (suffix == NULL) {
    m2c_emit_error(M2C_ERROR_INVALID_FILENAME_SUFFIX);
    exit(EXIT_FAILURE);
  }
  else if (is_def_suffix(suffix)) {
    srctype = M2C_DEF_SOURCE;
  }
  else if (is_mod_suffix(suffix)) {
    srctype = M2C_MOD_SOURCE;
  }
  else /* invalid suffix */ {
    m2c_emit_error(M2C_ERROR_INVALID_FILENAME_SUFFIX);
    exit(EXIT_FAILURE);
  } /* end if */
    
  /* check source file availability */
  if (NOT(file_exists(srcpath))) {
    m2c_emit_error_w_str(M2C_ERROR_INPUT_FILE_NOT_FOUND, path);
    exit(EXIT_FAILURE);
  } /* end if */
    
  /* get working directory */
  workdir = new_path_w_current_working_directory();
  
  if (workdir == NULL) {
    printf("unable to get current working directory.\n");
    exit(EXIT_FAILURE);
  } /* end if */
  
  /* initialise string repo */
  m2c_init_string_repository(0, NULL);
  
  /* print banner */
  print_identification();
  
  if (m2c_option_parser_debug()) {
    m2c_print_options();
  } /* end if */
  
  printf("processing %s\n", srcpath);
  
  /* run parser on input */
  m2c_parse_file(srctype, srcpath, &ast, &stats, &parser_status);
  
  /* write AST to file */
  if (ast != NULL) {
    /* write AST in S-expression format */
    astpath = new_path_w_components(workdir, basename, ".ast", NULL);
    printf("writing AST to %s\n", astpath);
    m2c_ast_write_tree(astpath, ast);
    
    /* write AST in graphviz DOT format */
    dotpath = new_path_w_components(workdir, basename, ".dot", NULL);
    printf("writing AST graph to %s\n", dotpath);
    m2c_ast_draw_tree(dotpath, ast);
  } /* end if */
  
  /* TO DO: semantic analysis and final code generation */
  
  /* print statistics */
  printf("warnings: %u\n", m2c_stats_warnings(stats));
  printf("errors: %u\n", m2c_stats_errors(stats));
  printf("lines: %u\n", m2c_stats_lines(stats));
  
  /* pass status code to caller */
  if (m2c_stats_errors(stats) == 0) {
    return EXIT_SUCCESS;
  }
  else /* errors occurred */ {
    return EXIT_FAILURE;
  } /* end if */
} /* end main */

/* END OF FILE */