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
 * m2-ast.h
 *
 * Public interface for the M2C abstract syntax tree.
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

#ifndef M2C_AST_H
#define M2C_AST_H

#include "m2-common.h"
#include "m2-ast-nodetype.h"

/* --------------------------------------------------------------------------
 * opaque type m2c_astnode_t
 * --------------------------------------------------------------------------
 * Opaque pointer type representing an AST node object.
 * ----------------------------------------------------------------------- */

typedef struct m2c_astnode_struct_t *m2c_astnode_t;


/* --------------------------------------------------------------------------
 * function m2c_ast_empty_node()
 * --------------------------------------------------------------------------
 * Returns the empty node singleton.
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_empty_node (void);


/* --------------------------------------------------------------------------
 * function m2c_ast_new_node(node_type, subnode0, subnode1, subnode2, ...)
 * --------------------------------------------------------------------------
 * Allocates a new branch node of the given node type, stores the subnodes of
 * the argument list in the node and returns the node, or NULL on failure.
 *
 * pre-conditions:
 * o  node_type must be a valid node type
 * o  a NULL terminated argument list of valid ast nodes must be passed
 *    and type and number of subnodes must match the given node type.
 *
 * post-conditions:
 * o  newly allocated and populated ast node is returned
 *
 * error-conditions:
 * o  if allocation fails, no node is allocated and NULL is returned
 * o  if node_type is invalid, no node is allocated and NULL is returned
 * o  if type and number of subnodes does not match the given node type,
 *    no node is allocated and NULL is returned
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_new_node
  (m2c_ast_nodetype_t node_type, ...);


/* --------------------------------------------------------------------------
 * function m2c_ast_new_list_node(node_type, list)
 * --------------------------------------------------------------------------
 * Allocates a new branch node of the given node type, stores the subnodes of
 * the given node list in the node and returns the node, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_new_list_node
  (m2c_ast_nodetype_t node_type, m2c_fifo_t list);


/* --------------------------------------------------------------------------
 * function m2c_ast_new_terminal_node(node_type, value)
 * --------------------------------------------------------------------------
 * Allocates a new terminal node of the given node type, stores the given
 * value in the node and returns the node, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_new_terminal_node
  (m2c_ast_nodetype_t type, m2c_string_t value);


/* --------------------------------------------------------------------------
 * function m2c_ast_new_terminal_list_node(node_type, value_list)
 * --------------------------------------------------------------------------
 * Allocates a new terminal node of the given node type, stores the values of
 * the given value list in the node and returns the node, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_new_terminal_list_node
  (m2c_ast_nodetype_t node_type, m2c_fifo_t list);


/* --------------------------------------------------------------------------
 * function m2c_ast_nodetype(node)
 * --------------------------------------------------------------------------
 * Returns the node type of node, or AST_INVALID if node is NULL.
 * ----------------------------------------------------------------------- */

m2c_ast_nodetype_t m2c_ast_nodetype (m2c_astnode_t node);


/* --------------------------------------------------------------------------
 * function m2c_ast_subnode_count(node)
 * --------------------------------------------------------------------------
 * Returns the number of subnodes or values of node. 
 * ----------------------------------------------------------------------- */

uint_t m2c_ast_subnode_count (m2c_astnode_t node);


/* --------------------------------------------------------------------------
 * function m2c_ast_subnode_for_index(node, index)
 * --------------------------------------------------------------------------
 * Returns the subnode of node with the given index or NULL if no subnode of
 * the given index is stored in node.
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_subnode_for_index (m2c_astnode_t node, uint_t index);


/* --------------------------------------------------------------------------
 * function m2c_ast_value_for_index(terminal_node, index)
 * --------------------------------------------------------------------------
 * Returns the value stored at the given index in a terminal node,
 * or NULL if the node does not store any value at the given index.
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_ast_value_for_index (m2c_astnode_t node, uint_t index);


/* --------------------------------------------------------------------------
 * convenience function m2c_ast_value(node)
 * --------------------------------------------------------------------------
 * Calls function m2c_ast_value_for_index with an index of zero. 
 * ----------------------------------------------------------------------- */

inline m2c_string_t m2c_ast_value (m2c_astnode_t node) {
  return m2c_ast_value_for_index(node, 0);
} /* end m2c_ast_terminal */


/* --------------------------------------------------------------------------
 * function m2c_ast_replace_subnode(in_node, at_index, with_subnode)
 * --------------------------------------------------------------------------
 * Replaces a subnode and returns the replaced node, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_astnode_t m2c_ast_replace_subnode
  (m2c_astnode_t in_node, uint_t at_index, m2c_astnode_t with_subnode);


/* --------------------------------------------------------------------------
 * function m2c_ast_replace_value(in_node, at_index, with_value)
 * --------------------------------------------------------------------------
 * Replaces a value and returns the replaced value, or NULL on failure.
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_ast_replace_value
  (m2c_astnode_t in_node, uint_t at_index, m2c_string_t with_value);


/* --------------------------------------------------------------------------
 * function m2c_ast_release_node(node)
 * --------------------------------------------------------------------------
 * Deallocates node. 
 * ----------------------------------------------------------------------- */

void m2c_ast_release_node (m2c_astnode_t node);


#endif /* M2C_AST_H */

/* END OF FILE */