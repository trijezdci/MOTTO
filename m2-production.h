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
 * m2-production.h
 *
 * Public interface for M2C first and follow set lookup.
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

#ifndef M2C_PRODUCTION_H
#define M2C_PRODUCTION_H

#include "m2-common.h"
#include "m2-tokenset.h"

#include <stdbool.h>


/* --------------------------------------------------------------------------
 * type m2c_production_t
 * --------------------------------------------------------------------------
 * Enumerated production values representing Modula-2 non-terminal symbols.
 * ----------------------------------------------------------------------- */

typedef enum {
  /* Productions with unique FIRST and FOLLOW sets */

  DEFINITION_MODULE,        /* definitionModule */
  IMPORT,                   /* import */
  QUALIFIED_IMPORT,         /* qualifiedImport */
  UNQUALIFIED_IMPORT,       /* unqualifiedImport */
  IDENT_LIST,               /* identList */
  DEFINITION,               /* definition */
  CONST_DEFINITION,         /* constDefinition */
  TYPE_DEFINITION,          /* typeDefinition */
  TYPE,                     /* type */
  DERIVED_OR_SUBRANGE_TYPE, /* derivedOrSubrangeType */
  QUALIDENT,                /* qualident */
  RANGE,                    /* range */
  ENUM_TYPE,                /* enumType */
  SET_TYPE,                 /* setType */
  COUNTABLE_TYPE,           /* countableType */
  ARRAY_TYPE,               /* arrayType */
  EXTENSIBLE_RECORD_TYPE,   /* extensibleRecordType */
  FIELD_LIST_SEQUENCE,      /* fieldListSequence */
  VARIANT_RECORD_TYPE,      /* variantRecordType */
  VARIANT_FIELD_LIST_SEQ,   /* variantFieldListSeq */
  VARIANT_FIELD_LIST,       /* variantFieldList */
  VARIANT_FIELDS,           /* variantFields */
  VARIANT,                  /* variant */
  CASE_LABEL_LIST,          /* caseLabelList */
  CASE_LABELS,              /* caseLabels */
  POINTER_TYPE,             /* pointerType */
  PROCEDURE_TYPE,           /* procedureType */
  SIMPLE_FORMAL_TYPE,       /* simpleFormalType */
  PROCEDURE_HEADER,         /* procedureHeader */
  PROCEDURE_SIGNATURE,      /* procedureSignature */
  SIMPLE_FORMAL_PARAMS,     /* simpleFormalParams */
  IMPLEMENTATION_MODULE,    /* implementationModule */
  PROGRAM_MODULE,           /* programModule */
  MODULE_PRIORITY,          /* modulePriority */
  BLOCK,                    /* block */
  DECLARATION,              /* declaration */
  TYPE_DECLARATION,         /* typeDeclaration */
  VAR_SIZE_RECORD_TYPE,     /* varSizeRecordType */
  VARIABLE_DECLARATION,     /* variableDeclaration */
  PROCEDURE_DECLARATION,    /* procedureDeclaration */
  MODULE_DECLARATION,       /* moduleDeclaration */
  EXPORT,                   /* export */
  STATEMENT_SEQUENCE,       /* statementSequence */
  STATEMENT,                /* statement */
  ASSIGNMENT_OR_PROC_CALL,  /* assignmentOrProcCall */
  ACTUAL_PARAMETERS,        /* actualParameters */
  EXPRESSION_LIST,          /* expressionList */
  RETURN_STATEMENT,         /* returnStatement */
  WITH_STATEMENT,           /* withStatement */
  IF_STATEMENT,             /* ifStatement */
  CASE_STATEMENT,           /* caseStatement */
  CASE,                     /* case */
  LOOP_STATEMENT,           /* loopStatement */
  WHILE_STATEMENT,          /* whileStatement */
  REPEAT_STATEMENT,         /* repeatStatement */
  FOR_STATEMENT,            /* forStatement */
  DESIGNATOR,               /* designator */
  SELECTOR,                 /* selector */
  EXPRESSION,               /* expression */
  SIMPLE_EXPRESSION,        /* simpleExpression */
  TERM,                     /* term */
  SIMPLE_TERM,              /* simpleTerm */
  FACTOR,                   /* factor */
  DESIGNATOR_OR_FUNC_CALL,  /* designatorOrFuncCall */
  SET_VALUE,                /* setValue */
  ELEMENT,                  /* element */
  
  /* Productions with alternative FIRST or FOLLOW sets */
  
  /* Dependent on option --const-parameters */
  FORMAL_TYPE,              /* formalType */
  ATTRIBUTED_FORMAL_TYPE,   /* attributedFormalType */
  FORMAL_PARAM_LIST,        /* formalParamList */
  FORMAL_PARAMS,            /* formalParams */
  ATTRIB_FORMAL_PARAMS,     /* attribFormalParams */
  
  /* Dependent on option --no-variant-records */
  TYPE_DECLARATION_TAIL,    /* typeDeclarationTail */
  
  /* Enumeration Terminator */
  
  PRODUCTION_END_MARK /* marks the end of this enumeration */
} m2c_production_t;

#define M2C_FIRST_OPTION_DEPENDENT FORMAL_TYPE
#define M2C_LAST_OPTION_DEPENDENT TYPE_DECLARATION_TAIL

#define M2C_FIRST_CONST_PARAM_DEPENDENT FORMAL_TYPE
#define M2C_LAST_CONST_PARAM_DEPENDENT ATTRIB_FORMAL_PARAMS

#define M2C_FIRST_NO_VARIANT_REC_DEPENDENT TYPE_DECLARATION_TAIL
#define M2C_LAST_NO_VARIANT_REC_DEPENDENT TYPE_DECLARATION_TAIL


/* --------------------------------------------------------------------------
 * Constant M2C_PRODUCTION_COUNT -- number of productions
 * ----------------------------------------------------------------------- */

#define M2C_PRODUCTION_COUNT PRODUCTION_END_MARK

/* --------------------------------------------------------------------------
 * function macro M2C_IS_VALID_PRODUCTION(p)
 * --------------------------------------------------------------------------
 * Returns TRUE if p represents a non-terminal symbol, otherwise FALSE.
 * ----------------------------------------------------------------------- */

#define M2C_IS_VALID_PRODUCTION(_p) \
  (((_p) >= 0) && ((_p) < PRODUCTION_END_MARK))


/* --------------------------------------------------------------------------
 * function macro IS_INVALID_PRODUCTION(p)
 * --------------------------------------------------------------------------
 * Returns TRUE if p does not represents a non-terminal symbol, else FALSE.
 * ----------------------------------------------------------------------- */

#define M2C_IS_INVALID_PRODUCTION(_p) \
  (!M2C_IS_VALID_PRODUCTION(_p))


/* --------------------------------------------------------------------------
 * function FIRST(p)
 * --------------------------------------------------------------------------
 * Returns a tokenset with the FIRST set of production p.
 * ----------------------------------------------------------------------- */

m2c_tokenset_t FIRST (m2c_production_t p);


/* --------------------------------------------------------------------------
 * function FOLLOW(p)
 * --------------------------------------------------------------------------
 * Returns a tokenset with the FOLLOW set of production p.
 * ----------------------------------------------------------------------- */

m2c_tokenset_t FOLLOW (m2c_production_t p);


/* --------------------------------------------------------------------------
 * function m2c_name_for_production(p)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to a NUL terminated character string with
 * a human readable name for p.  Returns NULL if p is not valid.
 * ----------------------------------------------------------------------- */

const char *m2c_name_for_production (m2c_production_t p);

#endif /* M2C_PRODUCTION_H */

/* END OF FILE */