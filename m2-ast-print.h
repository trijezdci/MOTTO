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
 * m2-ast-print.h
 *
 * Public interface for M2C abstract syntax tree output.
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

#ifndef M2C_AST_PRINT_H
#define M2C_AST_PRINT_H

#include "m2-common.h"
#include "m2-unique-string.h"
#include "m2-ast-nodetype.h"
#include "m2-ast.h"


/* --------------------------------------------------------------------------
 * function m2c_ast_print_int_value(lexeme)
 * --------------------------------------------------------------------------
 * Prints the value of an INTVAL node to the console. 
 * ----------------------------------------------------------------------- */

void m2c_ast_print_int_value (m2c_string_t lexeme);


/* --------------------------------------------------------------------------
 * function m2c_ast_print_chr_value(lexeme)
 * --------------------------------------------------------------------------
 * Prints the value of a CHRVAL node to the console. 
 * ----------------------------------------------------------------------- */

void m2c_ast_print_chr_value (m2c_string_t lexeme);


/* --------------------------------------------------------------------------
 * function m2c_ast_print_quoted_value(lexeme)
 * --------------------------------------------------------------------------
 * Prints the value of a QUOTEDVAL node to the console. 
 * ----------------------------------------------------------------------- */

void m2c_ast_print_quoted_value (m2c_string_t lexeme);


/* --------------------------------------------------------------------------
 * function m2c_ast_print_node(node)
 * --------------------------------------------------------------------------
 * Prints node as an S-expression to the console. 
 * ----------------------------------------------------------------------- */

void m2c_ast_print_node (m2c_astnode_t node);


#endif /* M2C_AST_PRINT_H */

/* END OF FILE */