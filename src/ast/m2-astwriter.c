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
 * m2-astwriter.c
 *
 * Implementation of M2C abstract syntax tree output to file.
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

#include "m2-astwriter.h"
#include "cstring.h"

#include <stdio.h>
#include <stdlib.h>


/* --------------------------------------------------------------------------
 * AST target file context
 * ----------------------------------------------------------------------- */

typedef struct astfile_s *astfile_t;

struct astfile_s {
  FILE *fptr;
  uint_t chars_written;
  m2c_fileio_status_t status;
};

typedef struct astfile_s astfile_s;


/* --------------------------------------------------------------------------
 * Forward declarations
 * ----------------------------------------------------------------------- */

static void ast_write_lf (m2c_astfile_t astfile);

static void ast_write_subtree (m2c_astfile_t astfile, m2c_astnode_t node);

static void ast_write_branches (m2c_astfile_t astfile, m2c_astnode_t node);

static void ast_write_leaves
  (m2c_astfile_t astfile, m2c_astnode_t node, m2c_astnode_type_t node_type);

static void ast_write_single_value
  (m2c_astfile_t astfile, m2c_astnode_t node, m2c_astnode_type_t node_type);

static void ast_write_value_list
  (m2c_astfile_t astfile, m2c_astnode_t node, m2c_astnode_type_t node_type);

static int ast_write_unformatted_value (FILE *fptr, m2c_string_t lexeme);

static int ast_write_int_value (FILE *fptr, m2c_string_t lexeme);

static int ast_write_chr_value (FILE *fptr, m2c_string_t lexeme);

static int ast_write_quoted_value (FILE *fptr, m2c_string_t lexeme);


/* --------------------------------------------------------------------------
 * function m2c_ast_write(path, ast, chars_written)
 * --------------------------------------------------------------------------
 * Writes the given abstract syntax tree in S-expression format to the given
 * output file at the given path and returns a status code.  Passes the
 * number of characters written back in out-parameter chars_written.
 * ----------------------------------------------------------------------- */

m2c_fileio_status_t m2c_ast_write
  (const char *path, m2c_astnode_t ast, uint_t *chars_written) {
  
  astfile_s astfile;
  
  if ((file_exists(path)) && (NOT(is_regular_file(path)))) {
    WRITE_OUTPARAM(chars_written, 0);
    return M2C_FILEIO_STATUS_INVALID_FILE;
  } /* end if */
  
  astfile.fptr = fopen(path, "w");
  
  if (astfile.fptr == NULL) {
    WRITE_OUTPARAM(chars_written, 0);
    return M2C_FILEIO_STATUS_FOPEN_FAILED;
  } /* end if */
  
  astfile.chars_written = 0;
  astfile.status = 0;
  
  ast_write_subtree(&astfile, ast);
  
  if (astfile.status == M2C_FILEIO_STATUS_SUCCESS) {
    ast_write_lf(&astfile);
  } /* end if */
      
  fclose(astfile.fptr);
  
  WRITE_OUTPARAM(chars_written, astfile.chars_written);
  
  return astfile.status;
} /* end m2c_ast_write */


/* *********************************************************************** *
 * Private Functions                                                       *
 * *********************************************************************** */

#define ADD_TO(_target,_source) { _target = _target + _source }


/* --------------------------------------------------------------------------
 * private function ast_write_lf(astfile)
 * --------------------------------------------------------------------------
 * Writes linefeed to astfile.  Records chars_written, status.
 * ----------------------------------------------------------------------- */

static void ast_write_lf (m2c_astfile_t astfile) {
  
  int write_count;
  
  write_count = fprintf(astfile->fptr, "\n");
  
  if (write_count >= 0) {
    ADD_TO(astfile->chars_written, write_count);
  }
  else {
    astfile->status = M2C_FILEIO_STATUS_WRITE_FAILED;
  } /* end if */
  
  return;
} /* end ast_write_lf */


/* --------------------------------------------------------------------------
 * private function ast_write_subtree(astfile, node)
 * --------------------------------------------------------------------------
 * Writes node to astfile.  Records chars_written, status.
 * ----------------------------------------------------------------------- */

static void ast_write_subtree (m2c_astfile_t astfile, m2c_astnode_t node) {
  
  int write_count;
  const char *node_name;
  m2c_ast_nodetype_t node_type;
  
  node_type = m2c_ast_nodetype(node);
  node_name = m2c_name_for_nodetype(node_type);
  
  /* write opening delimiter and stem */
  if (astfile->chars_written == 0) {
    write_count = fprintf(astfile->fptr, "(%s", node_name);
  }
  else {
    write_count = fprintf(astfile->fptr, " (%s", node_name);
  } /* end if */
  
  if (write_count >= 0) {
    ADD_TO(astfile->chars_written, write_count);
  }
  else {
    astfile->status = M2C_FILEIO_STATUS_WRITE_FAILED;
    return;
  } /* end if */
  
  /* write all branches and leaves */
  if (m2c_ast_is_nonterminal(node_type)) {
    ast_write_branches(astfile, node);
  }
  else {
    ast_write_leaves(astfile, node, node_type);
  } /* end if */
  
  if (astfile->status != M2C_FILEIO_STATUS_SUCCESS) {
    return;
  } /* end if */
  
  /* write closing delimiter */
  write_count = fprintf(astfile->fptr, ")");
  
  if (write_count >= 0) {
    ADD_TO(astfile->chars_written, write_count);
  }
  else {
    astfile->status = M2C_FILEIO_STATUS_WRITE_FAILED;
  } /* end if */
  
  return;
} /* end ast_write_subtree */


/* --------------------------------------------------------------------------
 * private function ast_write_branches(astfile, node)
 * --------------------------------------------------------------------------
 * Writes branch nodes of node to astfile.  Records chars_written, status.
 * ----------------------------------------------------------------------- */

static void ast_write_branches (m2c_astfile_t astfile, m2c_astnode_t node) {
  
  m2c_astnode_t branch_node;
  uint_t index, branch_count;
  
  branch_count = m2c_ast_subnode_count(node);
  
  for (index = 0; index < branch_count; index++) {
    branch_node = m2c_ast_subnode_for_index(node, index);
    ast_write_subtree(astfile, branch_node);
    
    /* exit loop on write failure */
    if (astfile->status != M2C_FILEIO_STATUS_SUCCESS)
      break;
    } /* end if */
  } /* end for */
  
  return;
} /* end ast_write_branches */


/* --------------------------------------------------------------------------
 * private function ast_write_leaves(astfile, node, node_type)
 * --------------------------------------------------------------------------
 * Writes leaf values of node to astfile.  Records chars_written, status.
 * ----------------------------------------------------------------------- */

static void ast_write_leaves
  (m2c_astfile_t astfile, m2c_astnode_t node, m2c_astnode_type_t node_type) {
  
  uint_t value_count;
  
  value_count = m2c_ast_subnode_count(node);
  
  if (value_count == 1) {
    ast_write_single_value(astfile, node, node_type);
  }
  else /* multi-leaf node */ {
    ast_write_value_list(astfile, node, node_type);
  } /* end if */
    
  return;
} /* end ast_write_leaves */


/* --------------------------------------------------------------------------
 * private function ast_write_single_value(astfile, node, node_type)
 * --------------------------------------------------------------------------
 * Writes sole leaf value of node to astfile.  Records chars_written, status.
 * ----------------------------------------------------------------------- */

static void ast_write_single_value
  (m2c_astfile_t astfile, m2c_astnode_t node, m2c_astnode_type_t node_type) {
  
  m2c_string_t value;
  int write_count;
    
  switch (node_type) {
    case AST_IDENT :
      value = m2c_ast_value_for_index(node, 0);
      write_count = ast_write_unformatted_value(astfile->fptr, value);
      break;
      
    case AST_INTVAL :
      value = m2c_ast_value_for_index(node, 0);
      write_count = ast_write_int_value(astfile->fptr, value);
      break;

    case AST_REALVAL :
      value = m2c_ast_value_for_index(node, 0);
      write_count = ast_write_unformatted_value(astfile->fptr, value);
      break;

    case AST_CHRVAL :
      value = m2c_ast_value_for_index(node, 0);
      write_count = ast_write_chr_value(astfile->fptr, value);
      break;

    case AST_QUOTEDVAL :
    case AST_FILENAME :
      value = m2c_ast_value_for_index(node, 0);
      write_count = ast_write_quoted_value(astfile->fptr, value);
      break;
  } /* end switch */
  
  if (write_count >= 0) {
    ADD_TO(astfile->chars_written, write_count);
  }
  else {
    astfile->status = M2C_FILEIO_STATUS_WRITE_FAILED;
  } /* end if */
  
  return;
} /* end ast_write_single_value */


/* --------------------------------------------------------------------------
 * private function ast_write_value_list(astfile, node, node_type)
 * --------------------------------------------------------------------------
 * Writes all leaf values of node to astfile.  Records chars_written, status.
 * ----------------------------------------------------------------------- */
 
static void ast_write_value_list
  (m2c_astfile_t astfile, m2c_astnode_t node, m2c_astnode_type_t node_type) {
  
  m2c_string_t value;
  int write_count;
  uint_t index, value_count;
  
  value_count = m2c_ast_subnode_count(node);
  
  for (index = 0; index < value_count; index++) {
    value = m2c_ast_value_for_index(node, index);
    
    if ((node_type == AST_QUALIDENT) || (node_type == AST_IDENTLIST)) {
        write_count = ast_write_unformatted_value(astfile->fptr, value);
    }
    else if (node_type == AST_OPTIONS) {
        write_count = ast_write_quoted_value(astfile->fptr, value);
    } /* end switch */
      
    if (write_count >= 0) {
      ADD_TO(astfile->chars_written, write_count);
    }
    else {
      astfile->status = M2C_FILEIO_STATUS_WRITE_FAILED;
      break;
    } /* end if */
  } /* end for */
  
  return;
} /* end ast_write_value_list */


/* --------------------------------------------------------------------------
 * private function ast_write_unformatted_value(fptr, lexeme)
 * --------------------------------------------------------------------------
 * Writes an unformatted leaf value to astfile.  Returns write count.
 * ----------------------------------------------------------------------- */

static int ast_write_unformatted_value (FILE *fptr, m2c_string_t lexeme) {
  
  const char *lexstr;
  
  lexstr = m2c_string_char_ptr(lexeme);
  
  return = fprintf(fptr, " %s", lexstr);
} /* end ast_write_unformatted_value */


/* --------------------------------------------------------------------------
 * private function ast_write_int_value(fptr, lexeme)
 * --------------------------------------------------------------------------
 * Writes the value of an INTVAL leaf node to astfile.  Returns write count.
 * ----------------------------------------------------------------------- */

static int ast_write_int_value (FILE *fptr, m2c_string_t lexeme) {
  
  uint_t length;
  const char *lexstr;
  
  length = m2c_string_length(lexeme);
  lexstr = m2c_string_char_ptr(lexeme);
  
  if (lexstr[1] == 'x') {
    return fprintf(fptr, " #%s", lexstr);
  }
  else if ((lexstr[length-1] == 'H') || (lexstr[length-1] == 'B')) {
    return fprintf(fptr, " ?%s", lexstr);
  }
  else {
    return fprintf(fptr, " %s", lexstr);
  } /* end if */
} /* end ast_write_int_value */


/* --------------------------------------------------------------------------
 * private function ast_write_chr_value(fptr, lexeme)
 * --------------------------------------------------------------------------
 * Writes the value of a CHRVAL leaf node to astfile.  Returns write count.
 * ----------------------------------------------------------------------- */

static int ast_write_chr_value (FILE *fptr, m2c_string_t lexeme) {
  
  uint_t length;
  const char *lexstr;
  
  length = m2c_string_length(lexeme);
  lexstr = m2c_string_char_ptr(lexeme);
  
  if (lexstr[1] == 'u') {
    return fprintf(fptr, " #%s", lexstr);
  }
  else if (lexstr[length-1] == 'C') {
    return fprintf(fptr, " ?%s", lexstr);
  }
  else {
    return fprintf(fptr, " %s", lexstr);
  } /* end if */
} /* end ast_write_chr_value */


/* --------------------------------------------------------------------------
 * private function ast_write_quoted_value(fptr, lexeme)
 * --------------------------------------------------------------------------
 * Writes the value of a QUOTEDVAL leaf node to astfile. Returns write count.
 * ----------------------------------------------------------------------- */

static int ast_write_quoted_value (FILE *fptr, m2c_string_t lexeme) {
  
  const char *lexstr;
  
  lexstr = m2c_string_char_ptr(lexeme);
  
  if (cstr_contains_char(lexstr, '"')) {
    return fprintf(fptr, " '%s'", lexstr);
  }
  else {
    return fprintf(fptr, " \"%s\"", lexstr);
  } /* end if */
} /* end ast_write_quoted_value */


/* END OF FILE */