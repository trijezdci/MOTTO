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
 * m2-ast-nodetype.c
 *
 * Implementation of M2C abstract syntax tree node types.
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

#include "m2-ast-nodetype.h"


/* --------------------------------------------------------------------------
 * array m2c_ast_nodetype_name_table
 * --------------------------------------------------------------------------
 * Human readable names for node types.
 * ----------------------------------------------------------------------- */

static const char *m2c_ast_nodetype_name_table[] {
  "EMPTY\0",
  "AST\0",
  "DEFMOD\0",
  "IMPLIST\0",
  "IMPORT\0",
  "UNQIMP\0",
  "DEFLIST\0",
  "CONSTDEF\0",
  "TYPEDEF\0",
  "PROCDEF\0",
  "SUBR\0",
  "ENUM\0",
  "SET\0",
  "ARRAY\0",
  "RECORD\0",
  "POINTER\0",
  "PROCTYPE\0",
  "EXTREC\0",
  "VRNTREC\0",
  "INDEXLIST\0",
  "INDEXTYPE\0",
  "FIELDLISTSEQ\0",
  "FIELDLIST\0",
  "VFLISTSEQ\0",
  "VFLIST\0",
  "VARIANTLIST\0",
  "VARIANT\0",
  "CLABELLIST\0",
  "CLABELS\0",
  "FTYPELIST\0",
  "ARGLIST\0",
  "OPENARRAY\0",
  "CONSTP\0",
  "VARP\0",
  "FPARAMLIST\0",
  "FPARAMS\0",
  "IMPMOD\0",
  "BLOCK\0",
  "DECLLIST\0",
  "TYPEDECL\0",
  "VARDECL\0",
  "PROC\0",
  "MODDECL\0",
  "VSREC\0",
  "VSFIELD\0",
  "EXPORT\0",
  "QUALEXP\0",
  "STMTSEQ\0",
  "ASSIGN\0",
  "PCALL\0",
  "RETURN\0",
  "WITH\0",
  "IF\0",
  "SWITCH\0",
  "LOOP\0",
  "WHILE\0",
  "REPEAT\0",
  "FORTO\0",
  "EXIT\0",
  "ARGS\0",
  "ELSIFSEQ\0",
  "ELSIF\0",
  "CASELIST\0",
  "CASE\0",
  "ELEMLIST\0",
  "RANGE\0",
  "FIELD\0",
  "INDEX\0",
  "DESIG\0",
  "DEREF\0",
  "NEG\0",
  "NOT\0",
  "EQ\0",
  "NEQ\0",
  "<\0",
  "<=\0",
  ">\0",
  ">=\0",
  "IN\0",
  "+\0",
  "-\0",
  "OR\0",
  "*\0",
  "/\0",
  "DIV\0",
  "MOD\0",
  "AND\0",
  "FCALL\0",
  "SETVAL\0",
  "IDENT\0",
  "QUALIDENT\0",
  "INTVAL\0",
  "REALVAL\0",
  "CHRVAL\0",
  "QUOTEDVAL\0",
  "IDENTLIST\0",
  "FILENAME\0",
  "OPTIONS\0",
}; /* end m2c_ast_nodetype_name_table */


/* --------------------------------------------------------------------------
 * array m2c_ast_nodetype_arity_table
 * --------------------------------------------------------------------------
 * Arity values for node types.
 * ----------------------------------------------------------------------- */

static const int8_t m2c_ast_nodetype_arity_table[] {
  /* AST_EMPTY */         0,
  /* AST_ROOT */          3,
  /* AST_DEFMOD */        3,
  /* AST_IMPLIST */      -1, /* variadic, arity >= 1 */
  /* AST_IMPORT */        1,
  /* AST_UNQIMP */        2,
  /* AST_DEFLIST */      -1, /* variadic, arity >= 1 */
  /* AST_CONSTDEF */      2,
  /* AST_TYPEDEF */       2,
  /* AST_PROCDEF */       3,
  /* AST_SUBR */          3,
  /* AST_ENUM */          1,
  /* AST_SET */           1,
  /* AST_ARRAY */         2,
  /* AST_RECORD */        1,
  /* AST_POINTER */       1,
  /* AST_PROCTYPE */      2,
  /* AST_EXTREC */        2,
  /* AST_VRNTREC */       1,
  /* AST_INDEXLIST */    -1, /* variadic, arity >= 1 */
  /* AST_INDEXTYPE */     1,
  /* AST_FIELDLISTSEQ */ -1, /* variadic, arity >= 1 */
  /* AST_FIELDLIST */     2,
  /* AST_VFLISTSEQ */    -1, /* variadic, arity >= 1 */
  /* AST_VFLIST */        4,
  /* AST_VARIANTLIST */  -1, /* variadic, arity >= 1 */
  /* AST_VARIANT */       2,
  /* AST_CLABELLIST */   -1, /* variadic, arity >= 1 */
  /* AST_CLABELS */       2,
  /* AST_FTYPELIST */    -1, /* variadic, arity >= 1 */
  /* AST_ARGLIST */      -1, /* variadic, arity >= 1 */
  /* AST_OPENARRAY */     1,
  /* AST_CONSTP */        1,
  /* AST_VARP */          1,
  /* AST_FPARAMLIST */   -1, /* variadic, arity >= 1 */
  /* AST_FPARAMS */       2,
  /* AST_IMPMOD */        4,
  /* AST_BLOCK */         2,
  /* AST_DECLLIST */     -1, /* variadic, arity >= 1 */
  /* AST_TYPEDECL */      2,
  /* AST_VARDECL */       2,
  /* AST_PROC */          4,
  /* AST_MODDECL */       5,
  /* AST_VSREC */         2,
  /* AST_VSFIELD */       3,
  /* AST_EXPORT */        1,
  /* AST_QUALEXP */       1,
  /* AST_STMTSEQ */      -1, /* variadic, arity >= 1 */
  /* AST_ASSIGN */        2,
  /* AST_PCALL */         2,
  /* AST_RETURN */        1,
  /* AST_WITH */          2,
  /* AST_IF */            4,
  /* AST_SWITCH */        3,
  /* AST_LOOP */          1,
  /* AST_WHILE */         2,
  /* AST_REPEAT */        2,
  /* AST_FORTO */         5,
  /* AST_EXIT */          0,
  /* AST_ARGS */         -1, /* variadic, arity >= 1 */
  /* AST_ELSIFSEQ */     -1, /* variadic, arity >= 1 */
  /* AST_ELSIF */         2,
  /* AST_CASELIST */     -1, /* variadic, arity >= 1 */
  /* AST_CASE */          2,
  /* AST_ELEMLIST */     -1, /* variadic, arity >= 1 */
  /* AST_RANGE */         2,
  /* AST_FIELD */         1,
  /* AST_INDEX */        -1, /* variadic, arity >= 1 */
  /* AST_DESIG */         2,
  /* AST_DEREF */         1,
  /* AST_NEG */           1,
  /* AST_NOT */           1,
  /* AST_EQ */            2,
  /* AST_NEQ */           2,
  /* AST_LT */            2,
  /* AST_LTEQ */          2,
  /* AST_GT */            2,
  /* AST_GTEQ */          2,
  /* AST_IN */            2,
  /* AST_PLUS */          2,
  /* AST_MINUS */         2,
  /* AST_OR */            2,
  /* AST_ASTERISK */      2,
  /* AST_SOLIDUS */       2,
  /* AST_DIV */           2,
  /* AST_MOD */           2,
  /* AST_AND */           2,
  /* AST_FCALL */         2,  
  /* AST_SETVAL */        2,
  /* AST_IDENT */         1,
  /* AST_QUALIDENT */    -2, /* variadic, arity >= 2 */
  /* AST_INTVAL */        1,
  /* AST_REALVAL */       1,
  /* AST_CHRVAL */        1,
  /* AST_QUOTEDVAL */     1
  /* AST_IDENTLIST */    -1, /* variadic, arity >= 1 */
  /* AST_FILENAME */      1,
  /* AST_OPTIONS */      -1, /* variadic, arity >= 1 */
}; /* end m2c_ast_nodetype_arity_table */


/* --------------------------------------------------------------------------
 * macros to determine node type group
 * ----------------------------------------------------------------------- */

#define IS_COMPUNIT(_nodetype) \
  ((_nodetype == AST_DEFMOD) || (_nodetype == AST_IMPMOD))

#define IS_IMPLIST_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_IMPLIST) || (_nodetype == AST_EMPTY))

#define IS_DEFLIST_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_DEFLIST) || (_nodetype == AST_EMPTY))

#define IS_DEFINITION(_nodetype) \
  (((_nodetype >= AST_FIRST_DEFINITION_NODETYPE) && \
    (_nodetype <= AST_LAST_DEFINITION_NODETYPE)) || \
   (_nodetype == AST_VARDECL))

#define IS_TYPE(_nodetype) \
  (((IS_IDENT_OR_QUALIDENT(_nodetype)) || \
   ((_nodetype >= AST_FIRST_TYPEDEFN_NODETYPE) && \
    (_nodetype <= AST_LAST_TYPEDEFN_NODETYPE)))

#define IS_TYPE_OR_EMPTY(_nodetype) \
  ((IS_TYPE(_nodetype)) || (_nodetype == AST_EMPTY))

#define IS_FIELDTYPE(_nodetype) \
  (((IS_IDENT_OR_QUALIDENT(_nodetype)) || \
   ((_nodetype >= AST_FIRST_FIELDTYPE_NODETYPE) && \
    (_nodetype <= AST_LAST_FIELDTYPE_NODETYPE)))

#defie IS_QUALIDENT_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_QUALIDENT) || (_nodetype == AST_EMPTY))

#define IS_COUNTABLE_TYPE(_nodetype) \
  ((IS_IDENT_OR_QUALIDENT(_nodetype)) || \
   (_nodetype == AST_SUBR) || (_nodetype == AST_ENUM))

#define IS_FTYPELIST_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_FTYPELIST) || (_nodetype == AST_EMPTY))

#define IS_VFLIST_OR_FIELDLIST(_nodetype) \
  ((_nodetype == AST_VFLIST) || (_nodetype == AST_FIELDLIST))

#define IS_IDENT_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_IDENT) || (_nodetype == AST_EMPTY))

#define IS_IDENT_OR_QUALIDENT(_nodetype) \
  ((_nodetype == AST_IDENT) || (_nodetype == AST_QUALIDENT))

#define IS_IDENT_OR_QUALIDENT_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_IDENT) || \
   (_nodetype == AST_QUALIDENT) || \
   (_nodetype == AST_EMPTY))

#define IS_FIELDLISTSEQ_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_FIELDLIST) || (_nodetype == AST_EMPTY))

#define IS_SIMPLE_FORMAL_TYPE(_nodetype) \
  ((IS_IDENT_OR_QUALIDENT(_nodetype)) || \
   (_nodetype == AST_ARGLIST) || (_nodetype == AST_OPENARRAY))
  
#define IS_FORMAL_TYPE(_nodetype) \
  (IS_SIMPLE_FORMAL_TYPE(_nodetype) ||
   (_nodetype == AST_CONST) || (_nodetype == AST_VAR));

#define IS_FPARAMLIST_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_FPARAMLIST) || (_nodetype == AST_EMPTY))

#define IS_DECLLIST_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_DECLLIST) || (_nodetype == AST_EMPTY))

#define IS_STMTSEQ_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_STMTSEQ) || (_nodetype == AST_EMPTY))

#define IS_DECL(_nodetype) \
  ((_nodetype == AST_CONSTDEF) || \
   ((_nodetype >= AST_FIRST_DECLARATION_NODETYPE) && \
    (_nodetype <= AST_LAST_DECLARATION_NODETYPE)))

#define IS_TYPE_OR_VSRTYPE(_nodetype) \
  (IS_TYPE(_nodetype) || (_nodetype == AST_VSREC))

#define IS_EXPORT_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_EXPORT) || (_nodetype == AST_EMPTY))

#define IS_ARGS_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_ARGS) || (_nodetype == AST_EMPTY))

#define IS_ELSIFSEQ_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_ELSIFSEQ) || (_nodetype == AST_EMPTY))

#define IS_DESIGNATOR(_nodetype) \
  ((IS_IDENT_OR_QUALIDENT(_nodetype)) || \
   (_nodetype == AST_DEREF) || \
   (_nodetype == AST_DESIG))

#define IS_SELECTOR(_nodetype) \
  ((IS_IDENT_OR_QUALIDENT(_nodetype)) || (_nodetype == AST_DESIG))

#define IS_DESIG_HEAD(_nodetype) \
  ((IS_IDENT_OR_QUALIDENT(_nodetype)) || (_nodetype == AST_DEREF))

#define IS_DESIG_TAIL(_nodetype) \
  ((_nodetype == AST_FIELD) || \ (_nodetype == AST_INDEX))

#define IS_STMT(_nodetype) \
  ((_nodetype >= AST_FIRST_STATEMENT_NODETYPE) && \
   (_nodetype <= AST_LAST_STATEMENT_NODETYPE))

#define IS_EXPR(_nodetype) \
  ((_nodetype >= AST_FIRST_EXPRESSION_NODETYPE) && \
   (_nodetype <= AST_LAST_EXPRESSION_NODETYPE))

#define IS_EXPR_OR_EMPTY(_nodetype) \
  ((IS_EXPR(_nodetype)) || (_nodetype == AST_EMPTY))

#define IS_EXPR_OR_RANGE_OR_EMPTY(_nodetype) \
  ((IS_EXPR_OR_EMPTY(_nodetype)) || (_nodetype == AST_RANGE))

#define IS_ELEMLIST_OR_EMPTY(_nodetype) \
  ((_nodetype == AST_ELEMLIST) || (_nodetype == AST_EMPTY))


/* --------------------------------------------------------------------------
 * function m2c_ast_is_list_nodetype(node_type)
 * --------------------------------------------------------------------------
 * Returns true if node_type is a list node type, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_ast_is_list_nodetype (m2c_ast_nodetype_t node_type) {
  int arity;
  
  if (!m2c_ast_is_valid_nodetype(node_type)) {
    return false;
  } /* end if */
  
  arity = m2c_ast_nodetype_arity_table[node_type];
  
  return (arity < 0);
} /* end m2c_ast_is_list_nodetype */


/* --------------------------------------------------------------------------
 * function m2c_ast_is_legal_subnode_count(node_type, subnode_count)
 * --------------------------------------------------------------------------
 * Returns true if the given subnode count is a legal value for the given
 * node type, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_ast_is_legal_subnode_count
  (m2c_ast_nodetype_t node_type, uint_t subnode_count) {
  int arity;
  
  if (!m2c_ast_is_valid_nodetype(node_type)) {
    return false;
  } /* end if */
  
  arity = m2c_ast_nodetype_arity_table[node_type];
  
  if (arity >= 0) {
    return (subnode_count == (uint_t) arity);
  }
  else /* variadic */ {
    arity = (-1) * arity;
    return (subnode_count >= (uint_t) arity);
  } /* end if */
  
} /* end m2c_ast_is_legal_subnode_count */


/* --------------------------------------------------------------------------
 * function m2c_ast_is_legal_subnode_type(node_type, subnode_type, index)
 * --------------------------------------------------------------------------
 * Returns true if the given subnode type is a legal node type for the given
 * index in a node of the given subnode type, otherwise false.
 * ----------------------------------------------------------------------- */

bool m2c_ast_is_legal_subnode_type
  (m2c_ast_nodetype_t node_type,
   m2c_ast_nodetype_t subnode_type,
   uint_t subnode_index) {
   
  switch (node_type) {
     
    /* EMPTY */
    case AST_EMPTY :
      return false; /* no subnodes */
      
    /* AST Root */
    case AST_ROOT :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_FILENAME);
        case 1 : return (subnode_type == AST_OPTIONS);
        case 2 : return (IS_COMPUNIT(subnode_type));
        default : return false;
      } /* end switch */
     
    /* Definition Module */
    case AST_DEFMOD :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_IDENT);
        case 1 : return (IS_IMPLIST_OR_EMPTY(subnode_type));
        case 2 : return (IS_DEFLIST_OR_EMPTY(subnode_type));
        default : return false;
      } /* end switch */
     
    /* Import List */
    case AST_IMPLIST :
      return
        ((subnode_type == AST_IMPORT) ||
         (subnode_type == AST_UNQIMP));
     
    /* Qualified Import */
    case AST_IMPORT :
      return
        ((subnode_index == 0) &&
         (subnode_type == AST_IDENTLIST));
    
    /* Unqualified Import */
    case AST_UNQIMP :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_IDENT);
        case 1 : return (subnode_type == AST_IDENTLIST);
        default : return false;
      } /* end switch */
    
    /* Definition List */
    case DEFLIST :
      return (IS_DEFINITION(subnode_type));
    
    /* Const Definition */
    case AST_CONSTDEF :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_IDENT);
        case 1 : return IS_EXPR(subnode_type);
        default : return false;
      } /* end switch */
    
    /* Type Definition */
    case AST_TYPEDEF :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_IDENT);
        case 1 : return IS_TYPE_OR_EMPTY(subnode_type);
        default : return false;
      } /* end switch */
    
    /* Procedure Definition */
    case AST_PROCDEF :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_IDENT);
        case 1 : return (IS_FPARAMLIST_OR_EMPTY(subnode_type));
        case 2 : return (IS_IDENT_OR_QUALIDENT_OR_EMPTY(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Subrange Type */
    case AST_SUBR :
      switch (subnode_index) {
        case 0 :
        case 1 : return (IS_EXPR(subnode_type));
        case 2 : return (IS_IDENT_OR_QUALIDENT_OR_EMPTY(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Enumeration Type */
    case AST_ENUM :
      return
        ((subnode_index == 0) &&
         (subnode_type == AST_IDENTLIST));
    
    /* Set Type */
    case AST_SET :
      return
        ((subnode_index == 0) &&
         (IS_COUNTABLE_TYPE(subnode_type)));
    
    /* Array Type */
    case AST_ARRAY :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_INDEXLIST);
        case 1 : return (IS_FIELDTYPE(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Simple Record Type */
    case AST_RECORD :
      return
        ((subnode_index == 0) &&
         (subnode_type == AST_FIELDLISTSEQ));
    
    /* Pointer Type */
    case AST_POINTER :
      return
        ((subnode_index == 0) &&
         (IS_TYPE(subnode_type)));
    
    /* Procedure Type */
    case AST_PROCTYPE :
      switch (subnode_index) {
        case 0 : return (IS_FTYPELIST_OR_EMPTY(subnode_type));
        case 1 : return (IS_IDENT_OR_QUALIDENT_OR_EMPTY(subnode_type));
        default : return false;
    } /* end switch */
    
    /* Extensible Record Type */
    case AST_EXTREC :
      switch (subnode_index) {
        case 0 : return (IS_IDENT_OR_QUALIDENT(subnode_type));
        case 1 : return (subnode_type == AST_FIELDLISTSEQ);
        default : return false;
      } /* end switch */
    
    /* Variant Record Type */
      case AST_VRNTREC :
        return
          ((subnode_index == 0) &&
           (subnode_type == AST_VFLISTSEQ));
    
    /* Index List */
    case AST_INDEXLIST :
      return (IS_COUNTABLE_TYPE(subnode_type));
    
    /* Simple Fieldlist Sequence */
    case AST_FIELDLISTSEQ :
      return (subnode_type == AST_FIELDLIST);
    
    /* Simple Fieldlist */
    case AST_FIELDLIST :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_IDENTLIST);
        case 1 : return (IS_FIELDTYPE(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Variant Record Fieldlist Sequence */
    case AST_VFLISTSEQ :
      return (IS_VFLIST_OR_FIELDLIST(subnode_type));
    
    /* Variant Fieldlist */
    case AST_VFLIST :
      switch (subnode_index) {
        case 0 : return (IS_IDENT_OR_EMPTY(subnode_type));
        case 1 : return (IS_IDENT_OR_QUALIDENT(subnode_type));
        case 2 : return (subnode_type == AST_VARIANTLIST);
        case 3 : return (IS_FIELDLISTSEQ_OR_EMPTY(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Variant List */
    case AST_VARIANTLIST :
      return (subnode_type == AST_VARIANT);
    
    /* Variant */
    case AST_VARIANT :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_CLABELLIST);
        case 1 : return (subnode_type == AST_FIELDLISTSEQ);
        default : return false;
      } /* end switch */
    
    /* Case Label List */
    case AST_CLABELLIST :
      return (subnode_type == AST_CLABELS);
    
    /* Case Labels */
    case AST_CLABELS :
      switch (subnode_index) {
        case 0 : return (IS_EXPR(subnode_type));
        case 1 : return (IS_EXPR_OR_EMPTY(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Formal Type List */
    case AST_FTYPELIST :
      return (IS_FORMAL_TYPE(subnode_type));
    
    /* Open Array or Variadic Parameter */
    case AST_ARGLIST :
    case AST_OPENARRAY :
      return
        ((subnode_index == 0) &&
         (IS_IDENT_OR_QUALIDENT(subnode_type));
    
    /* CONST and VAR Parameters */
    case AST_CONSTP :
    case AST_VARP :
      return
        ((subnode_index == 0) &&
         (IS_SIMPLE_FORMAL_TYPE(subnode_type)));
    
    /* Formal Parameter List */
    case AST_FPARAMLIST :
      return (subnode_type == AST_FPARAMS);
    
    /* Formal Parameters */
    case AST_FPARAMS :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_IDENTLIST);
        case 1 : return (IS_FORMAL_TYPE(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Program Or Implementation Module */
    case AST_IMPMOD :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_IDENT);
        case 1 : return (IS_EXPR_OR_EMPTY(subnode_type));
        case 2 : return (IS_IMPLIST_OR_EMPTY(subnode_type));
        case 3 : return (subnode_type == AST_BLOCK);
        default : return false;
      } /* end switch */
    
    /* Block */
    case AST_BLOCK :
      switch (subnode_index) {
        case 0 : return (IS_DECLLIST_OR_EMPTY(subnode_type));
        case 1 : return (IS_STMTSEQ_OR_EMPTY(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Declaration List */
    case AST_DECLLIST :
      return IS_DECL(subnode_type);
    
    /* Statement Sequence */
    case AST_STMTSEQ :
      return IS_STMT(subnode_type);
    
    /* Type Declaration */
    case AST_TYPEDECL :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_IDENT);
        case 1 : return (IS_TYPE_OR_VSRTYPE(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Variable Declaration */
    case AST_VARDECL :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_IDENTLIST);
        case 1 : return (IS_FIELDTYPE(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Procedure Declaration */
    case AST_PROC :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_IDENT);
        case 1 : return (IS_FPARAMLIST_OR_EMPTY(subnode_type));
        case 2 : return (IS_IDENT_OR_QUALIDENT_OR_EMPTY(subnode_type));
        case 3 : return (subnode_type == AST_BLOCK);
        default : return false;
      } /* end switch */
    
    /* Module Declaration */
    case AST_MODDECL :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_IDENT);
        case 1 : return (IS_EXPR_OR_EMPTY(subnode_type));
        case 2 : return (IS_IMPLIST_OR_EMPTY(subnode_type));
        case 3 : return (IS_EXPORT_OR_EMPTY(subnode_type));
        case 4 : return (subnode_type == AST_BLOCK);
        default : return false;
      } /* end switch */
    
    /* Variable Size Record Type */
    case AST_VSREC :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_FIELDLISTSEQ);
        case 1 : return (subnode_type == AST_VSFIELD);
        default : return false;
      } /* end switch */
        
    /* Variable Size Field */
    case AST_VSFIELD :
      switch (subnode_index) {
        case 0 :
        case 1 : return (subnode_type == AST_IDENT);
        case 2 : return (IS_IDENT_OR_QUALIDENT(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Export */
    case AST_EXPORT :
    case AST_QUALEXP :
       return
         ((subnode_index == 0) &&
          (subnode_type == AST_IDENTLIST));
    
    /* Statement Sequence */
    case AST_STMTSEQ :
       return
         ((subnode_index == 0) &&
          (IS_STMT(subnode_type)));
    
    /* Assignment */
    case AST_ASSIGN :
      switch (subnode_index) {
        case 0 : return (IS_DESIGNATOR(subnode_type));
        case 1 : return (IS_EXPR(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Procedure Call */
    case AST_PCALL :
      switch (subnode_index) {
        case 0 : return (IS_DESIGNATOR(subnode_type));
        case 1 : return (IS_ARGS_OR_EMPTY(subnode_type));
        default : return false;
      } /* end switch */
    
    /* RETURN Statement */
    case AST_RETURN :
       return
         ((subnode_index == 0) &&
          (IS_EXPR_OR_EMPTY(subnode_type)));
    
    /* WITH Statement */
    case AST_WITH :
      switch (subnode_index) {
        case 0 : return (IS_DESIGNATOR(subnode_type));
        case 1 : return (subnode_type == AST_STMTSEQ);
        default : return false;
      } /* end switch */
    
    /* IF Statement */
    case AST_IF :
      switch (subnode_index) {
        case 0 : return (IS_EXPR(subnode_type));
        case 1 : return (subnode_type == AST_STMTSEQ);
        case 2 : return (IS_ELSIFSEQ_OR_EMPTY(subnode_type));
        case 3 : return (IS_STMTSEQ_OR_EMPTY(subnode_type));
        default : return false;
      } /* end switch */
    
    /* CASE Statement */
    case AST_SWITCH :
      switch (subnode_index) {
        case 0 : return (IS_EXPR(subnode_type));
        case 1 : return (subnode_type == AST_CASELIST);
        case 3 : return (IS_STMTSEQ_OR_EMPTY(subnode_type));
        default : return false;
      } /* end switch */
    
    /* LOOP Statement */
    case AST_LOOP :
       return
         ((subnode_index == 0) &&
          (subnode_type == AST_STMTSEQ));
    
    /* WHILE Statement */
    case AST_WHILE :
      switch (subnode_index) {
        case 0 : return (IS_EXPR(subnode_type));
        case 1 : return (subnode_type == AST_STMTSEQ);
        default : return false;
      } /* end switch */
    
    /* REPEAT Statement */
    case AST_REPEAT :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_STMTSEQ);
        case 1 : return (IS_EXPR(subnode_type));
        default : return false;
      } /* end switch */
    
    /* FOR TO Statement */
    case AST_FORTO :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_IDENT);
        case 1 : 
        case 2 : return (IS_EXPR(subnode_type));
        case 3 : return (IS_EXPR_OR_EMPTY(subnode_type));
        case 4 : return (subnode_type == AST_STMTSEQ);
        default : return false;
      } /* end switch */
    
    /* EXIT */
    case AST_EXIT :
      return false; /* no subnodes */
    
    /* Actual Parameters */
    case AST_ARGS :
       return (IS_EXPR(subnode_type));
    
    /* ELSIF Sequence */
    case AST_ELSIFSEQ :
       return (subnode_type == AST_ELSIF);
    
    /* ELSIF Branch */
    case AST_ELSIF :
      switch (subnode_index) {
        case 0 : return (IS_EXPR(subnode_type));
        case 1 : return (subnode_type == AST_STMTSEQ);
        default : return false;
      } /* end switch */
    
    /* Case List */
    case AST_CASELIST :
       return (subnode_type == AST_CASE);
    
    /* Case Branch */
    case AST_CASE :
      switch (subnode_index) {
        case 0 : return (subnode_type == AST_CLABELLIST);
        case 1 : return (subnode_type == AST_STMTSEQ);
        default : return false;
      } /* end switch */
    
    /* Element List */
    case AST_ELEMLIST :
       return IS_EXPR_OR_RANGE_OR_EMPTY(subnode_type);
    
    /* Expression Range */
    case AST_RANGE :
         (((subnode_index == 0) || (subnode_index == 1)) &&
          (IS_EXPR(subnode_type)));
    
    /* Record Field Selector */
    case AST_FIELD :
       return
         ((subnode_index == 0) &&
          (IS_SELECTOR(subnode_type)));
    
    /* Array Subscript */
    case AST_INDEX :
       return
         ((subnode_index == 0) &&
          (IS_EXPR(subnode_type)));
    
    /* Designator */
    case AST_DESIG :
      switch (subnode_index) {
        case 0 : return (IS_DESIG_HEAD(subnode_type));
        case 1 : return (IS_DESIG_TAIL(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Pointer Dereference */
    case AST_DEREF :
       return
         ((subnode_index == 0) &&
          (IS_DESIGNATOR(subnode_type)));
    
    /* Unary Sub-Expression */
    case AST_NEG :
    case AST_NOT :
      return 
        ((subnode_index == 0) &&
         (IS_EXPR(subnode_type)));
    
    /* Binary Sub-Expression */
    case AST_EQ :
    case AST_NEQ :
    case AST_LT :
    case AST_LTEQ :
    case AST_GT :
    case AST_GTEQ :
    case AST_IN :
    case AST_PLUS :
    case AST_MINUS :
    case AST_OR :
    case AST_MUL :
    case AST_RDIV :
    case AST_EDIV :
    case AST_MOD :
    case AST_AND :
      return
        (((subnode_index == 0) || (subnode_index == 1)) &&
         (IS_EXPR(subnode_type)));
    
    /* Function Call */
    case AST_FCALL :
      switch (subnode_index) {
        case 0 : return (IS_DESIGNATOR(subnode_type));
        case 1 : return (IS_ARGS_OR_EMPTY(subnode_type));
        default : return false;
      } /* end switch */
    
    /* Set Value */
    case AST_SETVAL :
      switch (subnode_index) {
        case 0 : return (IS_IDENT_OR_QUALIDENT_OR_EMPTY(subnode_type));
        case 1 : return (IS_ELEMLIST_OR_EMPTY(subnode_type));
        default : return false;
      } /* end switch */
    
    /* invalid node type */
    default :
      return false;
    
  } /* end switch */
  
} /* end m2c_ast_is_legal_subnode_type */


/* --------------------------------------------------------------------------
 * function m2c_name_for_nodetype(node_type)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to a NUL terminated character string with
 * a human readable name for node_type, or NULL if node_type is invalid.
 * ----------------------------------------------------------------------- */

const char *m2c_name_for_nodetype (m2c_ast_nodetype_t node_type) {
  
  if (!m2c_ast_is_valid_nodetype(node_type)) {
    return NULL;
  } /* end if */
  
  return m2c_ast_nodetype_name_table[node_type];
  
} /* end m2c_name_for_nodetype */


/* END OF FILE */