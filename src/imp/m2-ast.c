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
 * m2-ast.c
 *
 * Implementation of the M2C abstract syntax tree.
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

#include "m2-ast.h"

#include <stddef.h>
#include <stdlib.h>


/* --------------------------------------------------------------------------
 * hidden type m2c_astnode_struct_t
 * --------------------------------------------------------------------------
 * record type representing an AST node object.
 * ----------------------------------------------------------------------- */

struct m2c_astnode_struct_t {
  /* node_type */ m2c_ast_nodetype_t node_type;
  /* subnode_count */ uint_t subnode_count;
  /* subnode_table */ m2c_astnode_variant subnode_table[];
};

typedef struct m2c_astnode_struct_t m2c_astnode_struct_t;

union m2c_astnode_variant {
  m2c_string_t terminal;
  m2c_astnode_t non_terminal;
};

typedef union m2c_astnode_variant m2c_astnode_variant;


/* --------------------------------------------------------------------------
 * Empty node singleton
 * ----------------------------------------------------------------------- */

static const m2c_astnode_struct_t m2c_ast_empty_node_struct = {
  AST_EMPTY, 0, NULL
};


/* --------------------------------------------------------------------------
 * function m2c_ast_empty_node()
 * --------------------------------------------------------------------------
 * Returns the empty node singleton.
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_empty_node (void) {
  return (m2c_astnode_t) &m2c_ast_empty_node_struct;
} /* end m2c_ast_empty_node */


/* --------------------------------------------------------------------------
 * function m2c_ast_new_node(node_type, subnode_list)
 * --------------------------------------------------------------------------
 * Allocates a new object of type m2c_astnode_t, populates it with sub-nodes
 * given as a list of astnode objects and returns the newly allocated node.
 *
 * pre-conditions:
 * o  node_type must be a valid node type
 * o  subnode_list may be NULL to indicate an empty node list,
 *    otherwise it must be a NULL terminated list of valid ast nodes,
 *    the number of subnodes must match the required nodes for node_type.
 *
 * post-conditions:
 * o  newly allocated and populated ast node is returned
 *
 * error-conditions:
 * o  if allocation fails, no node is allocated and NULL is returned
 * o  if node_type is invalid, no node is allocated and NULL is returned
 * o  if the number of subnodes is insufficient for node_type,
 *    no node is allocated and NULL is returned
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_new_node
  (m2c_ast_nodetype_t node_type, ...) {
  
  m2c_astnode_t this_subnode, new_node;
  uint_t subnode_count, index;
  va_list subnode_list;
  
  if (!m2c_ast_is_nonterminal_nodetype(node_type)) {
    return NULL;
  } /* end if */
  
  /* count subnodes in arglist */
  subnode_count = 0;
  va_start(subnode_list, node_type);
  this_subnode = va_arg(subnode_list, m2c_astnode_t);
  while (this_subnode != NULL) {
    if (!m2c_ast_is_legal_subnode
          (m2c_ast_nodetype(this_subnode), subnode_count)) {
      return NULL;
    } /* end if */
    subnode_count++;
    this_subnode = va_arg(subnode_list, m2c_astnode_t);
  } /* end while */
  
  /* verify subnode count */
  if (!m2c_ast_is_legal_subnode_count(node_type, subnode_count)) {
    return NULL;
  } /* end if */
  
  if (node_type == AST_EMPTY) {
    return (m2c_astnode_t) &m2c_ast_empty_node_struct;
  } /* end if */
  
  /* allocate node */
  new_node = malloc
    (sizeof(m2c_astnode_struct_t) + subnode_count * sizeof(m2c_astnode_t));
  
  /* initialise fields */
  new_node->node_type = node_type;
  new_node->subnode_count = subnode_count;
  
  /* store subnodes in table */
  va_start(subnode_list, node_type);
  for (index = 0; index < subnode_count; index++) {
    new_node->subnode_table[index].non_terminal =
      va_arg(subnode_list, m2c_astnode_t);
  } /* end for */
  
  va_end(subnode_list);
  
  return new_node;
} /* end m2c_ast_new_node */


/* --------------------------------------------------------------------------
 * function m2c_ast_new_list_node(node_type, list)
 * --------------------------------------------------------------------------
 * Allocates a new branch node of the given node type, stores the subnodes of
 * the given node list in the node and returns the node, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_new_list_node
  (m2c_ast_nodetype_t node_type, m2c_fifo_t list) {
  
  m2c_astnode_t new_node;
  uint_t subnode_count, index;
  
  if (!m2c_ast_is_nonterminal_nodetype(node_type)) {
    return NULL;
  } /* end if */
  
  if (!m2c_ast_is_list_nodetype(node_type)) {
    return NULL;
  } /* end if */
  
  if (list == NULL) {
    return NULL;
  } /* end if */
  
  subnode_count = m2c_fifo_entry_count(list);
  
  /* allocate node */
  new_node = malloc
    (sizeof(m2c_astnode_struct_t) + subnode_count * sizeof(m2c_astnode_t));
  
  /* initialise fields */
  new_node->node_type = node_type;
  new_node->subnode_count = subnode_count;
  
  /* store subnodes in table */
  for (index = 0; index < subnode_count; index++) {
    new_node->subnode_table[index].non_terminal = m2c_fifo_dequeue(list);
  } /* end for */
  
  return new_node;
} /* end m2c_ast_new_list_node */


/* --------------------------------------------------------------------------
 * function m2c_ast_new_terminal_node(node_type, value)
 * --------------------------------------------------------------------------
 * Allocates a new terminal node of the given node type, stores the given
 * value in the node and returns the node, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_new_terminal_node
  (m2c_ast_nodetype_t type, m2c_string_t value) {
  
  m2c_astnode_t new_node;
  
  if (!m2c_ast_is_terminal_nodetype(node_type)) {
    return NULL;
  } /* end if */
  
  /* verify subnode count */
  if (!m2c_ast_is_legal_subnode_count(node_type, 1)) {
    return NULL;
  } /* end if */
  
  /* allocate node */
  new_node = malloc
    (sizeof(m2c_astnode_struct_t) + sizeof(m2c_astnode_t));
  
  /* initialise fields */
  new_node->node_type = node_type;
  new_node->subnode_count = 1;
  
  /* store value */
  new_node->subnode_table[0].terminal = value;
  
  return new_node;
} /* end m2c_ast_new_terminal_node */


/* --------------------------------------------------------------------------
 * function m2c_ast_new_terminal_list_node(node_type, value_list)
 * --------------------------------------------------------------------------
 * Allocates a new terminal node of the given node type, stores the values of
 * the given value list in the node and returns the node, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_new_terminal_list_node
  (m2c_ast_nodetype_t node_type, m2c_fifo_t list)
  
  m2c_astnode_t new_node;
  uint_t subnode_count, index;
  
  if (!m2c_ast_is_terminal_nodetype(node_type)) {
    return NULL;
  } /* end if */
  
  if (!m2c_ast_is_list_nodetype(node_type)) {
    return NULL;
  } /* end if */
  
  if (list == NULL) {
    return NULL;
  } /* end if */
  
  subnode_count = m2c_fifo_entry_count(list);
  
  /* allocate node */
  new_node = malloc
    (sizeof(m2c_astnode_struct_t) + subnode_count * sizeof(m2c_astnode_t));
  
  /* initialise fields */
  new_node->node_type = node_type;
  new_node->subnode_count = subnode_count;
  
  /* store values in table */
  for (index = 0; index < subnode_count; index++) {
    new_node->subnode_table[index].terminal = m2c_fifo_dequeue(list);
  } /* end for */
  
  return new_node;
} /* end m2c_ast_new_terminal_list_node */


/* --------------------------------------------------------------------------
 * function m2c_ast_nodetype(node)
 * --------------------------------------------------------------------------
 * Returns the node type of node or AST_INVALID if node is NULL or invalid.
 * ----------------------------------------------------------------------- */

m2c_ast_nodetype_t m2c_ast_nodetype (m2c_astnode_t node) {

  if ((node == NULL) || (!m2c_ast_is_valid_nodetype(node->node_type))) {
    return AST_INVALID;
  } /* end if */
  
  return node->node_type;
} /* end m2c_ast_nodetype */


/* --------------------------------------------------------------------------
 * function m2c_ast_subnode_count(node)
 * --------------------------------------------------------------------------
 * Returns the number of subnodes or values of node. 
 * ----------------------------------------------------------------------- */

uint_t m2c_ast_subnode_count (m2c_astnode_t node) {

  if (node == NULL) {
    return 0;
  } /* end if */
  
  return node->subnode_count;
} /* end m2c_ast_subnode_count */


/* --------------------------------------------------------------------------
 * function m2c_ast_subnode_for_index(node, index)
 * --------------------------------------------------------------------------
 * Returns the subnode of node with the given index or NULL if no subnode of
 * the given index is stored in node.
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_subnode_for_index (m2c_astnode_t node, uint_t index) {
  
  if ((node == NULL) || (index >= node->subnode_count)) {
    return NULL;
  } /* end if */
  
  return node->subnode_table[index].non_terminal;
} /* end m2c_ast_subnode_for_index */


/* --------------------------------------------------------------------------
 * function m2c_ast_value_for_index(terminal_node, index)
 * --------------------------------------------------------------------------
 * Returns the value stored at the given index in a terminal node,
 * or NULL if the node does not store any value at the given index.
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_ast_value_for_index (m2c_astnode_t node, uint_t index) {
  
  if ((node == NULL) || (!m2c_ast_is_terminal_nodetype(node->node_type))) {
    return NULL;
  } /* end if */
  
  return node->subnode_table[index].terminal;
} /* end m2c_ast_value_for_index */


/* --------------------------------------------------------------------------
 * function m2c_ast_replace_subnode(in_node, at_index, with_subnode)
 * --------------------------------------------------------------------------
 * Replaces a subnode and returns the replaced node, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_replace_subnode
  (m2c_astnode_t in_node, uint_t at_index, m2c_astnode_t with_subnode) {
  
  m2c_astnode_t replaced_node;

  if ((in_node == NULL) ||
      (!m2c_ast_is_nonterminal_nodetype(in_node->node_type))) {
    return NULL;
  } /* end if */
  
  if (!m2c_ast_is_legal_subnode(m2c_ast_nodetype(with_subnode), at_index)) {
    return NULL;
  } /* end if */
  
  replaced_node = in_node->subnode_table[at_index].non_terminal;
  in_node->subnode_table[at_index].non_terminal = with_subnode;
  
  return replaced_node;
} /* end m2c_ast_replace_subnode */


/* --------------------------------------------------------------------------
 * function m2c_ast_replace_value(in_node, at_index, with_value)
 * --------------------------------------------------------------------------
 * Replaces a value and returns the replaced value, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_ast_replace_value
  (m2c_astnode_t in_node, uint_t at_index, m2c_string_t with_value) {
  
  m2c_string_t replaced_value;

  if ((in_node == NULL) || (at_index >= in_node->subnode_count) ||
      (!m2c_ast_is_terminal_nodetype(in_node->node_type))) {
    return NULL;
  } /* end if */
  
  replaced_value = in_node->subnode_table[at_index].terminal;
  in_node->subnode_table[at_index].terminal = with_value;
  
  return replaced_value;
} /* end m2c_ast_replace_value */


/* --------------------------------------------------------------------------
 * function m2c_ast_release_node(node)
 * --------------------------------------------------------------------------
 * Deallocates node. 
 * ----------------------------------------------------------------------- */

void m2c_ast_release_node (m2c_astnode_t node) {
  
  if (node == NULL) {
    return;
  } /* end if */
  
  free(node);
} /* end m2c_ast_release_node */


/* END OF FILE */