/* M2C Modula-2 Compiler & Translator
 * Copyright (c) 2015, 2016 Benjamin Kowarsch
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
 * m2-dotwriter.c
 *
 * Implementation of M2C abstract syntax tree graph export.
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

#include "m2-dotwriter.h"


/* --------------------------------------------------------------------------
 * presentation
 * ----------------------------------------------------------------------- */

#define DEFAULT_GRAPH_TITLE "AST"

#define FONTNAME "helvetica"

#define GRAPH_FONTSIZE 10

#define NODE_FONTSIZE 8


/* --------------------------------------------------------------------------
 * output file context
 * ----------------------------------------------------------------------- */

typedef struct dotfile_s *dotfile_t;

struct dotfile_s {
  FILE *fptr;
  uint_t next_free_id;
  uint_t chars_written;
  m2c_fileio_status_t status;
};

typedef struct dotfile_s dotfile_s;


/* --------------------------------------------------------------------------
 * forward declarations
 * ----------------------------------------------------------------------- */

static m2c_fileio_status_t dot_draw_graph
  (m2c_dotfile_t dotfile, const char *title, m2c_astnode_t node);

static m2c_fileio_status_t dot_draw_subtree
  (m2c_dotfile_t dotfile, m2c_astnode_t node, uint_t node_id);

static m2c_fileio_status_t dot_draw_branches
  (m2c_dotfile_t dotfile, m2c_astnode_t node, uint_t node_id);

static m2c_fileio_status_t dot_draw_leaves
  (m2c_dotfile_t dotfile, m2c_astnode_t node, uint_t node_id);

static m2c_fileio_status_t dot_draw_edges
  (m2c_dotfile_t dotfile,
   uint_t node_id, uint_t first_edge_id, uint_t edge_count);

static int dot_draw_leaf_w_quoted_value
  (FILE *fptr, m2c_string_t value, uint_t id);

static int dot_draw_leaf_w_unquoted_value
  (FILE *fptr, m2c_string_t value, uint_t id);


/* --------------------------------------------------------------------------
 * function m2c_dot_write(path, ast, chars_written)
 * --------------------------------------------------------------------------
 * Writes the given abstract syntax tree in Graphviz DOT format to the given
 * output file at the given path and returns a status code.  Passes the
 * number of characters written back in out-parameter chars_written.
 * ----------------------------------------------------------------------- */

m2c_fileio_status_t m2c_dot_write
  (const char *path, m2c_astnode_t ast, uint_t *chars_written) {  
  
  dotfile_s dotfile;
  m2c_fileio_status_t status;
  
  if ((file_exists(path)) && (NOT(is_regular_file(path)))) {
    WRITE_OUTPARAM(chars_written, 0);
    return M2C_FILEIO_STATUS_INVALID_FILE;
  } /* end if */
  
  dotfile.fptr = fopen(path, "w");
  
  if (dotfile.fptr == NULL) {
    WRITE_OUTPARAM(chars_written, 0);
    return M2C_FILEIO_STATUS_FOPEN_FAILED;
  } /* end if */
  
  dotfile.chars_written = 0;
  
  status = dot_draw_graph(&dotfile, DEFAULT_GRAPH_TITLE, ast);
  
  fclose(dotfile.fptr);
  
  WRITE_OUTPARAM(chars_written, dotfile.chars_written);
  
  return status;
} /* end m2c_dot_write */


/* *********************************************************************** *
 * Private Functions                                                       *
 * *********************************************************************** */

#define ADD_TO(_target,_source) { _target = _target + _source }

#define UPDATE_WRITE_COUNT_OR_BAIL_ON_FAILURE(_dotfile,_write_count) \
  if (_write_count >= 0) \
    { ADD_TO(_dotfile->chars_written, _write_count); } \
  else \
    { return M2C_FILEIO_STATUS_WRITE_FAILED; }


/* --------------------------------------------------------------------------
 * function dot_draw_graph(dotfile, title, node)
 * --------------------------------------------------------------------------
 * Writes AST node as a GraphViz DOT graph to a file. 
 * ----------------------------------------------------------------------- */

static m2c_fileio_status_t dot_draw_graph
  (m2c_dotfile_t dotfile, const char *title, m2c_astnode_t node) {
  
  int write_count;
  m2c_fileio_status_t status;
  
  /* write graph title */
  write_count = fprintf(dotfile->fptr, "digraph %s {\n", title);
  
  UPDATE_WRITE_COUNT_OR_BAIL_ON_FAILURE(dotfile, write_count);
  
  /* write graph defaults */
  write_count =
    fprintf(dotfile->fptr,
      " graph [fontname=%s,fontsize=%u];\n", FONTNAME, GRAPH_FONTSIZE);
  
  UPDATE_WRITE_COUNT_OR_BAIL_ON_FAILURE(dotfile, write_count);
  
  /* write node defaults */
  write_count =
    fprintf(dotfile->fptr,
      " node [style=solid,shape=box,fontsize=%u];\n", NODE_FONTSIZE);
  
  UPDATE_WRITE_COUNT_OR_BAIL_ON_FAILURE(dotfile, write_count);
  
  /* write edge defaults */
  write_count =
    fprintf(dotfile->fptr, " edge [style=solid,arrowsize=0.75];\n\n");
  
  UPDATE_WRITE_COUNT_OR_BAIL_ON_FAILURE(dotfile, write_count);
  
  /* write graph label */
  write_count =
    fprintf(dotfile->fptr,
      " labelloc=\"t\"; labeljust=\"l\";\n label=\"%s\n\";\n\n", title);
  
  UPDATE_WRITE_COUNT_OR_BAIL_ON_FAILURE(dotfile, write_count);
  
  /* draw graph */
  dotfile->next_free_id = 1;
  status = dot_write_subtree(dotfile, node, /* node_id = */ 0);
  
  if (status != M2C_FILEIO_STATUS_SUCCESS) {
    return status;
  } /* end if */
  
  /* write closing delimiter */
  write_count = fprintf(dotfile->fptr, "} /* end %s */\n", title);
  
  UPDATE_WRITE_COUNT_OR_BAIL_ON_FAILURE(dotfile, write_count);
  
  return M2C_FILEIO_STATUS_SUCCESS;
} /* end dot_draw_graph */


/* --------------------------------------------------------------------------
 * private function dot_draw_subtree(dotfile, node, node_id)
 * ----------------------------------------------------------------------- */

static m2c_fileio_status_t dot_draw_subtree
  (m2c_dotfile_t dotfile, m2c_astnode_t node, uint_t node_id) {
  
  int write_count;
  const char *node_name;
  m2c_fileio_status_t status;
  m2c_astnode_type_t node_type;
  
  node_type = m2c_ast_nodetype(node);
  node_name = m2c_name_for_nodetype(node_type);
  
  /* draw root node */
  write_count =
    fprintf(fptr," node%u [label=\"%s\"];\n", node_id, node_name);
  
  UPDATE_WRITE_COUNT_OR_BAIL_ON_FAILURE(dotfile, write_count);
    
  /* draw all branches and leaves */
  if (m2c_ast_is_nonterminal(node_type)) {
    status = dot_draw_branches(dotfile, node, node_id);
  }
  else {
    status = dot_draw_leaves(dotfile, node, node_id);
  } /* end if */
  
  return status;
} /* end dot_draw_subtree */


/* --------------------------------------------------------------------------
 * private function dot_draw_branches(dotfile, node, node_id)
 * ----------------------------------------------------------------------- */

static m2c_fileio_status_t dot_draw_branches
  (m2c_dotfile_t dotfile, m2c_astnode_t node, uint_t node_id) {
  
  int write_count;
  uint_t index, first_branch_id, branch_count, this_branch_id;
  m2c_astnode_t this_branch;
  m2c_fileio_status_t status;
  
  first_branch_id = dotfile->next_free_id;
  branch_count = m2c_ast_subnode_count(node);
  
  /* draw edges, reserve node id's */
  status = dot_draw_edges(dotfile, node_id, first_branch_id, branch_count);
  if (status != M2C_FILEIO_STATUS_SUCCESS) {
    return status;
  } /* end if */
  
  /* draw subtrees for all branch nodes */
  for (index = 0; index < branch_count; index++) {
    this_branch_id = first_branch_id + index;
    this_branch = m2c_ast_subnode_for_index(node, index);
    
    status = dot_draw_subtree(dotfile, this_branch, this_branch_id);
    if (status != M2C_FILEIO_STATUS_SUCCESS) {
      break;
    } /* end if */
  } /* end for */
  
  return status;
} /* end dot_draw_branches */


/* --------------------------------------------------------------------------
 * private function dot_draw_leaves(dotfile, node, node_id)
 * ----------------------------------------------------------------------- */

#define HAS_QUOTABLE_LEAF_VALUES(_nodetype) \
  ((_nodetype == AST_QUOTEDVAL) || \
   (_nodetype == AST_FILENAME) || (_nodetype == AST_OPTIONS))

static m2c_fileio_status_t dot_draw_leaves
  (m2c_dotfile_t dotfile, m2c_astnode_t node, uint_t node_id) {
  
  int write_count;
  uint_t index, first_leaf_id, leaf_count, this_leaf_id;
  bool is_quotable_value;
  m2c_ast_nodetype_t node_type;
  m2c_string_t this_value;
  m2c_fileio_status_t status;
  
  first_leaf_id = dotfile->next_free_id;
  leaf_count = m2c_ast_subnode_count(node);
  
  /* draw edges, reserve node id's */
  status = dot_draw_edges(dotfile, node_id, first_leaf_id, leaf_count);
  if (status != M2C_FILEIO_STATUS_SUCCESS) {
    return status;
  } /* end if */
  
  /* determine quoting style */
  node_type = m2c_ast_nodetype(node);
  is_quotable_value = (HAS_QUOTABLE_LEAF_VALUES(node_type));
  
  /* draw all leaf nodes */
  for (index = 0; index < leaf_count; index++) {
    this_leaf_id = first_leaf_id + index;
    this_value = m2c_ast_value_for_index(node, index);
    
    /* draw leaf, adjust quoting style */
    if (is_quotable_value) {
      write_count =
        draw_leaf_w_quoted_value(dotfile->fptr, value, this_leaf_id);
    }
    else {
      write_count =
        draw_leaf_w_unquoted_value(dotfile->fptr, value, this_leaf_id);
    } /* end if */
    
    UPDATE_WRITE_COUNT_OR_BAIL_ON_FAILURE(dotfile, write_count);
  } /* end for */
    
  write_count = fprintf(fptr, "\n");
  
  UPDATE_WRITE_COUNT_OR_BAIL_ON_FAILURE(dotfile, write_count);
  
  return M2C_FILEIO_STATUS_SUCCESS;
} /* end dot_draw_leaves */


/* --------------------------------------------------------------------------
 * private function dot_draw_edges(dotfile, node_id, first_edge_id, count)
 * ----------------------------------------------------------------------- */

static m2c_fileio_status_t dot_draw_edges
  (m2c_dotfile_t dotfile,
   uint_t node_id, uint_t first_edge_id, uint_t edge_count) {
  
  int write_count;
  uint_t index, this_edge_id;

  /* write header with originating node */
  write_count = fprintf(dotfile->fptr, " node%u -> {", node_id);
  
  UPDATE_WRITE_COUNT_OR_BAIL_ON_FAILURE(dotfile, write_count);
    
  /* write body with list of connected nodes */
  for (index = 0; index < edge_count; index++) {
    this_edge_id = first_edge_id + index;
    
    write_count = fprintf(fptr, " node%u", this_edge_id);
    
    UPDATE_WRITE_COUNT_OR_BAIL_ON_FAILURE(dotfile, write_count);
  } /* end for */
    
  /* write list's closing delimiter */
  write_count = fprintf(fptr, " };\n\n");
    
  UPDATE_WRITE_COUNT_OR_BAIL_ON_FAILURE(dotfile, write_count);
  
  /* reserve the node id's of the connected nodes */
  ADD_TO(dotfile->next_free_id, edge_count);
  
  return M2C_FILEIO_STATUS_SUCCESS;
} /* end dot_draw_edges */


/* --------------------------------------------------------------------------
 * private function dot_draw_leaf_w_quoted_value(fptr, value, id)
 * ----------------------------------------------------------------------- */

static int dot_draw_leaf_w_quoted_value
  (FILE *fptr, m2c_string_t value, uint_t id) {
  
  int write_count;
  const char *lexstr;
  
  lexstr = m2c_string_char_ptr(value);
    
  if (cstr_contains_char(lexstr, '"')) {
    write_count = /* DOT output: nodeN [label="'...'",style=filled]; */
      fprintf(fptr, " node%u [label=\"'%s'\",style=filled];\n", id, lexstr);
  }
  else {
    write_count = /* DOT output: nodeN [label="\"...\"",style=filled]; */
      fprintf(fptr,
        " node%u [label=\"\\\"%s\\\"\",style=filled];\n", id, lexstr);
  } /* end if */
  
  return write_count;
} /* end dot_draw_leaf_w_quoted_value */


/* --------------------------------------------------------------------------
 * private function dot_draw_leaf_w_unquoted_value(fptr, value, id)
 * ----------------------------------------------------------------------- */

static int dot_draw_leaf_w_unquoted_value
  (FILE *fptr, m2c_string_t value, uint_t id) {
  
  int write_count;
  const char *lexstr;
  
  lexstr = m2c_string_char_ptr(value);
  
  write_count = /* DOT output: nodeN [label="...",style=filled]; */
    fprintf(fptr, " node%u [label=\"%s\",style=filled];\n", id, lexstr);
  
  return write_count;
} /* end dot_draw_leaf_w_unquoted_value */


/* END OF FILE */