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
 * m2-ast-draw.c
 *
 * Implementation of M2C abstract syntax tree graph exporter.
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

#include "m2-ast-draw.h"


/* --------------------------------------------------------------------------
 * macros and forward declarations
 * ----------------------------------------------------------------------- */

#define HAS_QUOTABLE_LEAF_VALUES(_nodetype) \
  ((_nodetype == AST_QUOTEDVAL) || \
   (_nodetype == AST_FILENAME) || (_nodetype == AST_OPTIONS))

static void draw_leaf_with_quoted_value (m2c_string_t value, uint_t id);

static void draw_leaf_with_unquoted_value (m2c_string_t value, uint_t id);

static void draw_subtree
  (m2c_astnode_t node, uint_t parent_id, uint_t *node_id);


/* --------------------------------------------------------------------------
 * function m2c_ast_draw_node(node, title)
 * --------------------------------------------------------------------------
 * Prints node as a GraphViz DOT graph to the console. 
 * ----------------------------------------------------------------------- */

void m2c_ast_draw_node (m2c_astnode_t node, const char *title) {
  uint_t node_id, subnode_id;
    
  printf("digraph %s {\n", title);
  
  printf(" graph [fontname=helvetica,fontsize=10];\n");
  printf(" node [style=solid,shape=box,fontsize=8];\n");
  printf(" edge [style=solid,arrowsize=0.75];\n\n");
  
  if ((title != NULL) && (title[0] != 0)) {
    printf(" labelloc=\"t\"; labeljust=\"l\";\n");
    printf(" label=\"%s\n\";\n\n");
  } /* end if */
  
  node_id = 0; subnode_id = node_id + 1;
  draw_subtree(node, node_id, &subnode_id);
  
  printf("} /* end %s */\n", title);
} /* end m2c_ast_draw_node */


/* --------------------------------------------------------------------------
 * private function draw_subtree(node, node_id, first_subnode_id)
 * ----------------------------------------------------------------------- */

static void draw_subtree
  (m2c_astnode_t node, uint_t node_id, uint_t first_subnode_id) {
  
  uint_t index, subnode_id, subnode_count, next_free_id, subsubnode_count;
  m2c_astnode_type_t node_type;
  const char *node_name;
  m2c_string_t value;
    
  node_type = m2c_ast_nodetype(node);
  node_name = m2c_name_for_nodetype(node_type);
  subnode_count = m2c_ast_subnode_count(node);
  
  printf(" node%u [label=\"%s\"];\n", node_id, node_name);
  
  if (m2c_ast_is_nonterminal(node_type)) {
  
    /* connections to all subnodes */
    printf(" node%u -> {", node_id);
    for (index = 0; index < subnode_count; index++) {
      subnode_id = first_subnode_id + index;
      printf(" node%u", subnode_id);
    } /* end for */
    printf(" };\n\n");
    
    /* subtrees of all subnodes */
    next_free_id = first_subnode_id + subnode_count + 1;
    for (index = 0; index < subnode_count; index++) {
      subnode = m2c_ast_subnode_for_index(node, index);
      subnode_id = first_subnode_id + index;
      draw_subtree(subnode, subnode_id, next_free_id);
      subsubnode_count = m2c_ast_subnode_count(subnode);
      next_free_id = next_free_id + subsubnode_count + 1;
    } /* end for */
  }
  else /* terminal node */ {
    
    /* connections to all leafs */
    printf(" node%u -> {", node_id);
    for (index = 0; index < subnode_count; index++) {
      subnode_id = first_subnode_id + index + 1;
      printf(" node%u", subnode_id);
    } /* end for */
    printf(" };\n\n");
    
    /* draw leafs */
    for (index = 0; index < subnode_count; index++) {
      subnode = m2c_ast_subnode_for_index(node, index);
      subnode_id = first_subnode_id + index + 1;
      value = m2c_ast_value_for_index(node, index);
      if (HAS_QUOTABLE_LEAF_VALUES(node_type)) {
        draw_leaf_with_quoted_value(value, subnode_id);
      }
      else {
        draw_leaf_with_unquoted_value(value, subnode_id);
      } /* end if */
    } /* end for */
    printf("\n");
  } /* end if */
} /* end draw_subtree */


/* --------------------------------------------------------------------------
 * private function draw_leaf_with_quoted_value(node, id)
 * ----------------------------------------------------------------------- */

static void draw_leaf_with_quoted_value (m2c_string_t value, uint_t id) {
  const char *lexstr;
  uint_t length, index;
  bool contains_double_quote;
  
  length = m2c_string_length(value);
  lexstr = m2c_string_char_ptr(value);
  
  for (index = 0; index < length; index++) {
    contains_double_quote = (lexstr[index] == '"');
    if (contains_double_quote) {
      break;
    } /* end if */
  } /* end for */
  
  if (contains_double_quote) {
    printf(" node%u [label=\"'%s'\",style=filled];\n", id, lexstr);
  }
  else {
    printf(" node%u [label=\"\"%s\"\",style=filled];\n", id, lexstr);
  } /* end if */
} /* end draw_leaf_with_quoted_value */


/* --------------------------------------------------------------------------
 * private function draw_leaf_with_unquoted_value(node, id)
 * ----------------------------------------------------------------------- */

static void draw_leaf_with_unquoted_value (m2c_string_t value, uint_t id) {
  const char *lexstr = m2c_string_char_ptr(value);
  printf(" node%u [label=\"%s\",style=filled];\n", id, lexstr);
} /* end draw_leaf_with_unquoted_value */

/* END OF FILE */