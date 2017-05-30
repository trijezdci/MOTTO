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
 * m2-ast-print.c
 *
 * Implementation of M2C abstract syntax tree output.
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

#include "m2-ast-print.h"


/* --------------------------------------------------------------------------
 * function m2c_ast_print_unformatted_value(lexeme)
 * --------------------------------------------------------------------------
 * Prints the unformatted value of a terminal node to the console. 
 * ----------------------------------------------------------------------- */

void m2c_ast_print_unformatted_value (m2c_string_t lexeme) {
  const char *lexstr = m2c_string_char_ptr(lexeme);
  printf("%s", lexstr);
} /* end m2c_ast_print_unformatted_value */


/* --------------------------------------------------------------------------
 * function m2c_ast_print_int_value(lexeme)
 * --------------------------------------------------------------------------
 * Prints the value of an INTVAL node to the console. 
 * ----------------------------------------------------------------------- */

void m2c_ast_print_int_value (m2c_string_t lexeme) {
  uint_t length;
  const char *lexstr;
  
  length = m2c_string_length(lexeme);
  lexstr = m2c_string_char_ptr(lexeme);
  
  if (lexstr[1] == 'x') {
    printf("#%s", lexstr);
  }
  else if ((lexstr[length-1] == 'H') || (lexstr[length-1] == 'B')) {
    printf("?%s", lexstr);
  }
  else {
    printf("%s", lexstr);
  } /* end if */
} /* end m2c_ast_print_int_value */


/* --------------------------------------------------------------------------
 * function m2c_ast_print_chr_value(lexeme)
 * --------------------------------------------------------------------------
 * Prints the value of a CHRVAL node to the console. 
 * ----------------------------------------------------------------------- */

void m2c_ast_print_chr_value (m2c_string_t lexeme) {
  uint_t length;
  const char *lexstr;
  
  length = m2c_string_length(lexeme);
  lexstr = m2c_string_char_ptr(lexeme);
  
  if (lexstr[1] == 'u') {
    printf("#%s", lexstr);
  }
  else if (lexstr[length-1] == 'C') {
    printf("?%s", lexstr);
  }
  else {
    printf("%s", lexstr);
  } /* end if */
} /* end m2c_ast_print_chr_value */


/* --------------------------------------------------------------------------
 * function m2c_ast_print_quoted_value(lexeme)
 * --------------------------------------------------------------------------
 * Prints the value of a QUOTEDVAL node to the console. 
 * ----------------------------------------------------------------------- */

void m2c_ast_print_quoted_value (m2c_string_t lexeme) {
  uint_t length;
  const char *lexstr;
  bool contains_double_quote;
  
  length = m2c_string_length(lexeme);
  lexstr = m2c_string_char_ptr(lexeme);
  
  for (index = 0; index < length; index++) {
    contains_double_quote = (lexstr[index] == '"');
    if (contains_double_quote) {
      break;
    } /* end if */
  } /* end for */
  
  if (contains_double_quote) {
    printf("'%s'", lexstr);
  }
  else {
    printf("\"%s\"", lexstr);
  } /* end if */
} /* end m2c_ast_print_quoted_value */


/* --------------------------------------------------------------------------
 * function m2c_ast_print_node(node)
 * --------------------------------------------------------------------------
 * Prints node as an S-expression to the console. 
 * ----------------------------------------------------------------------- */

void m2c_ast_print_node (m2c_astnode_t node) {
  m2c_astnode_type_t node_type;
  uint_t index, subnode_count;
  const char *node_name;
  m2c_string_t value;
  
  node_type = m2c_ast_nodetype(node);
  node_name = m2c_name_for_nodetype(node_type);
  subnode_count = m2c_ast_subnode_count(node);
  
  printf("(%s", node_name);
  
  if (m2c_ast_is_nonterminal(node_type)) {
    for (index = 0; index < subnode_count; index++) {
      subnode = m2c_ast_subnode_for_index(node, index);
      printf(" "); m2c_ast_print_node(subnode);
    } /* end for */
  }
  else /* terminal node */ {
    value = m2c_ast_value_for_index(node, 0);
    
    switch (node_type) {
      case AST_IDENT :
        printf(" "); m2c_ast_print_unformatted_value(value);
        break;
      
      case AST_QUALIDENT :
      case AST_IDENTLIST :
        printf(" "); m2c_ast_print_unformatted_value(value);
        for (index = 1; index < subnode_count; index++) {
          value = m2c_ast_value_for_index(node, index);
          printf(" "); m2c_ast_print_unformatted_value(value);
        } /* end for */
        break;
      
      case AST_INTVAL :
        printf(" "); m2c_ast_print_int_value(value);
        break;
      
      case AST_REALVAL :
        printf(" "); m2c_ast_print_unformatted_value(value);
        break;
      
      case AST_CHRVAL :
        printf(" "); m2c_ast_print_chr_value(value);
        break;
      
      case AST_QUOTEDVAL :
        printf(" "); m2c_ast_print_quoted_value(value);
        break;
      
      case AST_FILENAME :
        printf(" "); m2c_ast_print_quoted_value(value);
        break;
    
      case AST_OPTIONS :
        printf(" "); m2c_ast_print_quoted_value(value);
        for (index = 1; index < subnode_count; index++) {
          value = m2c_ast_value_for_index(node, index);
          printf(" "); m2c_ast_print_quoted_value(value);
        } /* end for */
        break;
    } /* end switch */
  } /* end if */
  
  printf(")");
} /* end m2c_ast_print_node */

/* END OF FILE */