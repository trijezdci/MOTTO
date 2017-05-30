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
 * m2-ast-nodetype.h
 *
 * Public interface for M2C abstract syntax tree node types.
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

#ifndef M2C_AST_NODETYPE_H
#define M2C_AST_NODETYPE_H

#include "m2-common.h"

#include <stdbool.h>


/* --------------------------------------------------------------------------
 * type m2c_astnode_type_t
 * --------------------------------------------------------------------------
 * Enumerated values representing AST node types.
 * ----------------------------------------------------------------------- */

typedef enum {
  /* Empty Node Type */
  
  AST_EMPTY,
  
  /* Root Node Type */
  
  AST_ROOT,
  
  /* Definition Module Non-Terminal Node Types */

  AST_DEFMOD,             /* definition module node type */
  AST_IMPLIST,            /* qualified import list node type */
  AST_IMPORT,             /* qualified import list node type */
  AST_UNQIMP,             /* unqualified import list node type */
  AST_DEFLIST,            /* definition list node type */
  
  AST_CONSTDEF,           /* constant definition node type */
  AST_TYPEDEF,            /* type definition node type */
  AST_PROCDEF,            /* procedure definition node type */
  
  AST_SUBR,               /* subrange type node type */
  AST_ENUM,               /* enumeration type node type */
  AST_SET,                /* set type node type */
  AST_ARRAY,              /* array type node type */
  AST_RECORD,             /* simple record type node type */
  AST_POINTER,            /* pointer type node type */
  AST_PROCTYPE,           /* procedure type node type */
  
  AST_EXTREC,             /* extensible record type node type */
  AST_VRNTREC,            /* variant record type node type */
  
  AST_INDEXLIST,          /* array index type list node type */
  
  AST_FIELDLISTSEQ,       /* field list sequence node type */
  AST_FIELDLIST,          /* field list node type */
  AST_VFLISTSEQ,          /* variant field list sequence node type */
  AST_VFLIST,             /* variant field list node type */
  AST_VARIANTLIST,        /* variant list node type */
  AST_VARIANT,            /* variant node type */
  AST_CLABELLIST,         /* case label list node type */
  AST_CLABELS,            /* case labels node type */
  
  AST_FTYPELIST,          /* formal type list node type */
  AST_ARGLIST,            /* variadic parameter list formal type node type */
  AST_OPENARRAY,          /* open array formal type node type */
  AST_CONSTP,             /* CONST formal type node type */
  AST_VARP,               /* VAR formal type node type */
  AST_FPARAMLIST,         /* formal parameter list node type */
  AST_FPARAMS,            /* formal parameters node type */

  /* Implementation/Program Module AST Node Types */

  AST_IMPMOD,             /* implementation/program module node type */
  AST_BLOCK,              /* block node type */
  AST_DECLLIST,           /* declaration list node type */
  
  AST_TYPEDECL,           /* type declaration node type */
  AST_VARDECL,            /* variable declaration node type */
  AST_PROC,               /* procedure declaration node type */
  AST_MODDECL,            /* local module declaration node type */
  
  AST_VSREC,              /* variable size record type node type */
  AST_VSFIELD,            /* variable size field node type */
  
  AST_EXPORT,             /* unqualified export list node type */
  AST_QUALEXP,            /* qualified export list node type */
  
  AST_STMTSEQ,            /* statement sequence node type */
  
  AST_ASSIGN,             /* assignment node type */
  AST_PCALL,              /* procedure call node type */
  AST_RETURN,             /* RETURN statement node type */
  AST_WITH,               /* WITH statement node type */
  AST_IF,                 /* IF statement node type */
  AST_SWITCH,             /* CASE statement node type */
  AST_LOOP,               /* LOOP statement node type */
  AST_WHILE,              /* WHILE statement node type */
  AST_REPEAT,             /* REPEAT statement node type */
  AST_FORTO,              /* FOR TO statement node type */
  AST_EXIT,               /* EXIT statement node type */
  
  AST_ARGS,               /* actual parameter list node type */
  
  AST_ELSIFSEQ,           /* ELSIF branch sequence node type */
  AST_ELSIF,              /* ELSIF branch node type */
  AST_CASELIST,           /* case list node type */
  AST_CASE,               /* case branch node type */
  AST_ELEMLIST,           /* element list node type */
  AST_RANGE,              /* expression range node type */
  
  /* Designator Subnode Types */
  
  AST_FIELD,              /* record field selector node type */
  AST_INDEX,              /* array subscript node type */
  
  /* Expression Node Types */
  
  AST_DESIG,              /* designator node type */
  AST_DEREF,              /* pointer dereference node type */
  
  AST_NEG,                /* arithmetic negation sub-expression node */
  AST_NOT,                /* logical negation sub-expression node */
  
  AST_EQ,                 /* equality sub-expression node */
  AST_NEQ,                /* inequality sub-expression node */
  AST_LT,                 /* less-than sub-expression node */
  AST_LTEQ,               /* less-than-or-equal sub-expression node */
  AST_GT,                 /* greater-than sub-expression node */
  AST_GTEQ,               /* greater-than-or-equal sub-expression node */
  AST_IN,                 /* set membership sub-expression node */
  AST_PLUS,               /* plus sub-expression node */
  AST_MINUS,              /* minus sub-expression node */
  AST_OR,                 /* logical disjunction sub-expression node */
  AST_ASTERISK,           /* asterisk sub-expression node */
  AST_SOLIDUS,            /* solidus sub-expression node */
  AST_DIV,                /* euclidean division sub-expression node */
  AST_MOD,                /* modulus sub-expression node */
  AST_AND,                /* logical conjunction expression node */
  
  AST_FCALL,              /* function call node */
  AST_SETVAL,             /* set value node */
  
  /* Identifier Node Types */
  
  AST_IDENT,              /* identifier node type */
  AST_QUALIDENT,          /* qualified identifier node type */
  
  /* Literal Value Node Types */
  
  AST_INTVAL,             /* whole number value node */
  AST_REALVAL,            /* real number value node */
  AST_CHRVAL,             /* character code value node */
  AST_QUOTEDVAL,          /* quoted literal value node */
  
  AST_IDENTLIST,          /* identifier list node type */
  
  /* Compilation Parameter Node Types */
  
  AST_FILENAME,           /* filename node type */
  AST_OPTIONS,            /* compiler option list node type */
  
  /* Invalid Node Type */
  
  AST_INVALID, /* for use as failure indicator */
  
  /* Enumeration Terminator */
  
  AST_END_MARK /* marks the end of this enumeration */
} m2c_astnode_type_t;


/* --------------------------------------------------------------------------
 * AST node type groupings.
 * ----------------------------------------------------------------------- */

#define AST_FIRST_VALID_NODETYPE AST_EMPTY
#define AST_LAST_VALID_NODETYPE AST_QUOTEDVAL

#define AST_FIRST_NONTERMINAL_NODETYPE AST_EMPTY
#define AST_LAST_NONTERMINAL_NODETYPE AST_SETVAL

#define AST_FIRST_TERMINAL_NODETYPE AST_INTVAL
#define AST_LAST_TERMINAL_NODETYPE AST_OPTIONS

#define AST_FIRST_DEFINITION_NODETYPE AST_CONSTDEF
#define AST_LAST_DEFINITION_NODETYPE AST_PROCDEF

#define AST_FIRST_TYPEDEFN_NODETYPE AST_SUBR
#define AST_LAST_TYPEDEFN_NODETYPE AST_VRNTREC

#define AST_FIRST_FIELDTYPE_NODETYPE AST_SUBR
#define AST_LAST_FIELDTYPE_NODETYPE AST_PROCTYPE

#define AST_FIRST_DECLARATION_NODETYPE AST_TYPEDECL
#define AST_LAST_DECLARATION_NODETYPE AST_MODDECL

#define AST_FIRST_STATEMENT_NODETYPE AST_ASSIGN
#define AST_LAST_STATEMENT_NODETYPE AST_EXIT

#define AST_FIRST_EXPRESSION_NODETYPE AST_DESIG
#define AST_LAST_EXPRESSION_NODETYPE AST_QUOTEDVAL

#define AST_FIRST_LITERAL_NODETYPE AST_INTVAL
#define AST_LAST_LITERAL_NODETYPE AST_QUOTEDVAL


/* --------------------------------------------------------------------------
 * function m2c_ast_is_valid_nodetype(node_type)
 * --------------------------------------------------------------------------
 * Returns true if node_type is a valid node type, otherwise false.
 * Values AST_INVALID and AST_END_MARK are not valid node types.
 * ----------------------------------------------------------------------- */

inline bool m2c_ast_is_valid_nodetype (m2c_ast_nodetype_t node_type) {
  return
    ((node_type >= AST_FIRST_VALID_NODETYPE) &&
     (node_type <= AST_LAST_VALID_NODETYPE));
} /* end m2c_ast_is_valid_nodetype */


/* --------------------------------------------------------------------------
 * function m2c_ast_is_nonterminal_nodetype(node_type)
 * --------------------------------------------------------------------------
 * Returns true if node_type is a nonterminal node type, otherwise false.
 * ----------------------------------------------------------------------- */

inline bool m2c_ast_is_nonterminal_nodetype (m2c_ast_nodetype_t node_type) {
  return
    ((node_type >= AST_FIRST_NONTERMINAL_NODETYPE) &&
     (node_type <= AST_LAST_NONTERMINAL_NODETYPE));
} /* end m2c_ast_is_nonterminal_nodetype */


/* --------------------------------------------------------------------------
 * function m2c_ast_is_terminal_nodetype(node_type)
 * --------------------------------------------------------------------------
 * Returns true if node_type is a terminal node type, otherwise false.
 * ----------------------------------------------------------------------- */

inline bool m2c_ast_is_terminal_nodetype (m2c_ast_nodetype_t node_type) {
  return
    ((node_type >= AST_FIRST_TERMINAL_NODETYPE) &&
     (node_type <= AST_LAST_TERMINAL_NODETYPE));
} /* end m2c_ast_is_terminal_nodetype */


/* --------------------------------------------------------------------------
 * function m2c_ast_is_list_nodetype(node_type)
 * --------------------------------------------------------------------------
 * Returns true if node_type is a list node type, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_ast_is_list_nodetype (m2c_ast_nodetype_t node_type);


/* --------------------------------------------------------------------------
 * function m2c_ast_is_legal_subnode_count(node_type, subnode_count)
 * --------------------------------------------------------------------------
 * Returns true if the given subnode count is a legal value for the given
 * node type, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_ast_is_legal_subnode_count
  (m2c_ast_nodetype_t node_type, uint_t subnode_count);


/* --------------------------------------------------------------------------
 * function m2c_ast_is_legal_subnode_type(node_type, subnode_type, index)
 * --------------------------------------------------------------------------
 * Returns true if the given subnode type is a legal node type for the given
 * index in a node of the given subnode type, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_ast_is_legal_subnode_type
  (m2c_ast_nodetype_t node_type,
   m2c_ast_nodetype_t subnode_type,
   uint_t subnode_index);


/* --------------------------------------------------------------------------
 * function m2c_name_for_nodetype(node_type)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to a NUL terminated character string with
 * a human readable name for node_type, or NULL if node_type is invalid.
 * ----------------------------------------------------------------------- */

const char *m2c_name_for_nodetype (m2c_ast_nodetype_t node_type);

#endif /* M2C_AST_NODETYPE_H */

/* END OF FILE */