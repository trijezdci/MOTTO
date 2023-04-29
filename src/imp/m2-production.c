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
 * m2-production.c
 *
 * Implementation of M2C first and follow set lookup.
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

#include "m2-production.h"

#include "m2-compiler-options.h"
#include "m2-first-set-inits.h"
#include "m2-follow-set-inits.h"

#include <stddef.h>


/* --------------------------------------------------------------------------
 * FIRST set data structures
 * ----------------------------------------------------------------------- */

m2c_tokenset_literal_t
  first_of_definition_module = INIT_FIRST_OF_DEFINITION_MODULE,
  first_of_import = INIT_FIRST_OF_IMPORT,
  first_of_qualified_import = INIT_FIRST_OF_QUALIFIED_IMPORT,
  first_of_unqualified_import = INIT_FIRST_OF_UNQUALIFIED_IMPORT,
  first_of_ident_list = INIT_FIRST_OF_IDENT_LIST,
  first_of_definition = INIT_FIRST_OF_DEFINITION,
  first_of_const_definition = INIT_FIRST_OF_CONST_DEFINITION,
  first_of_type_definition = INIT_FIRST_OF_TYPE_DEFINITION,
  first_of_type = INIT_FIRST_OF_TYPE,
  first_of_derived_or_subr_type = INIT_FIRST_OF_DERIVED_OR_SUBRANGE_TYPE,
  first_of_qualident = INIT_FIRST_OF_QUALIDENT,
  first_of_range = INIT_FIRST_OF_RANGE,
  first_of_enum_type = INIT_FIRST_OF_ENUM_TYPE,
  first_of_set_type = INIT_FIRST_OF_SET_TYPE,
  first_of_countable_type = INIT_FIRST_OF_COUNTABLE_TYPE,
  first_of_array_type = INIT_FIRST_OF_ARRAY_TYPE,
  first_of_extensible_record_type = INIT_FIRST_OF_EXTENSIBLE_RECORD_TYPE,
  first_of_field_list_sequence = INIT_FIRST_OF_FIELD_LIST_SEQUENCE,
  first_of_variant_record_type = INIT_FIRST_OF_VARIANT_RECORD_TYPE,
  first_of_variant_field_list_seq = INIT_FIRST_OF_VARIANT_FIELD_LIST_SEQ,
  first_of_variant_field_list = INIT_FIRST_OF_VARIANT_FIELD_LIST,
  first_of_variant_fields = INIT_FIRST_OF_VARIANT_FIELDS,
  first_of_variant = INIT_FIRST_OF_VARIANT,
  first_of_case_label_list = INIT_FIRST_OF_CASE_LABEL_LIST,
  first_of_case_labels = INIT_FIRST_OF_CASE_LABELS,
  first_of_pointer_type = INIT_FIRST_OF_POINTER_TYPE,
  first_of_procedure_type = INIT_FIRST_OF_PROCEDURE_TYPE,
  first_of_simple_formal_type = INIT_FIRST_OF_SIMPLE_FORMAL_TYPE,
  first_of_procedure_header = INIT_FIRST_OF_PROCEDURE_HEADER,
  first_of_procedure_signature = INIT_FIRST_OF_PROCEDURE_SIGNATURE,
  first_of_simple_formal_params = INIT_FIRST_OF_SIMPLE_FORMAL_PARAMS,
  first_of_implementation_module = INIT_FIRST_OF_IMPLEMENTATION_MODULE,
  first_of_program_module = INIT_FIRST_OF_PROGRAM_MODULE,
  first_of_module_priority = INIT_FIRST_OF_MODULE_PRIORITY,
  first_of_block = INIT_FIRST_OF_BLOCK,
  first_of_declaration = INIT_FIRST_OF_DECLARATION,
  first_of_type_declaration = INIT_FIRST_OF_TYPE_DECLARATION,
  first_of_var_size_record_type = INIT_FIRST_OF_VAR_SIZE_RECORD_TYPE,
  first_of_variable_declaration = INIT_FIRST_OF_VARIABLE_DECLARATION,
  first_of_procedure_declaration = INIT_FIRST_OF_PROCEDURE_DECLARATION,
  first_of_module_declaration = INIT_FIRST_OF_MODULE_DECLARATION,
  first_of_export = INIT_FIRST_OF_EXPORT,
  first_of_statement_sequence = INIT_FIRST_OF_STATEMENT_SEQUENCE,
  first_of_statement = INIT_FIRST_OF_STATEMENT,
  first_of_assignment_or_proc_call = INIT_FIRST_OF_ASSIGNMENT_OR_PROC_CALL,
  first_of_actual_parameters = INIT_FIRST_OF_ACTUAL_PARAMETERS,
  first_of_expression_list = INIT_FIRST_OF_EXPRESSION_LIST,
  first_of_return_statement = INIT_FIRST_OF_RETURN_STATEMENT,
  first_of_with_statement = INIT_FIRST_OF_WITH_STATEMENT,
  first_of_if_statement = INIT_FIRST_OF_IF_STATEMENT,
  first_of_case_statement = INIT_FIRST_OF_CASE_STATEMENT,
  first_of_case = INIT_FIRST_OF_CASE,
  first_of_loop_statement = INIT_FIRST_OF_LOOP_STATEMENT,
  first_of_while_statement = INIT_FIRST_OF_WHILE_STATEMENT,
  first_of_repeat_statement = INIT_FIRST_OF_REPEAT_STATEMENT,
  first_of_for_statement = INIT_FIRST_OF_FOR_STATEMENT,
  first_of_designator = INIT_FIRST_OF_DESIGNATOR,
  first_of_selector = INIT_FIRST_OF_SELECTOR,
  first_of_expression = INIT_FIRST_OF_EXPRESSION,
  first_of_simple_expression = INIT_FIRST_OF_SIMPLE_EXPRESSION,
  first_of_term = INIT_FIRST_OF_TERM,
  first_of_simple_term = INIT_FIRST_OF_SIMPLE_TERM,
  first_of_factor = INIT_FIRST_OF_FACTOR,
  first_of_designator_or_func_call = INIT_FIRST_OF_DESIGNATOR_OR_FUNC_CALL,
  first_of_set_value = INIT_FIRST_OF_SET_VALUE,
  first_of_element = INIT_FIRST_OF_ELEMENT,
  
  /* Option dependent FIRST sets */
  
  first_of_formal_type = INIT_FIRST_OF_FORMAL_TYPE,
  first_of_attributed_formal_type = INIT_FIRST_OF_ATTRIBUTED_FORMAL_TYPE,
  first_of_formal_param_list = INIT_FIRST_OF_FORMAL_PARAM_LIST,
  first_of_formal_params = INIT_FIRST_OF_FORMAL_PARAMS,
  first_of_attrib_formal_params = INIT_FIRST_OF_ATTRIB_FORMAL_PARAMS,
  first_of_type_declaration_tail = INIT_FIRST_OF_TYPE_DECLARATION_TAIL,
  
  /* Alternative FIRST sets */
  
  first_of_alt_formal_type = INIT_FIRST_OF_ALT_FORMAL_TYPE,
  first_of_alt_attrib_formal_type = INIT_FIRST_OF_ALT_ATTRIB_FORMAL_TYPE,
  first_of_alt_formal_param_list = INIT_FIRST_OF_ALT_FORMAL_PARAM_LIST,
  first_of_alt_formal_params = INIT_FIRST_OF_ALT_FORMAL_PARAMS,
  first_of_alt_attrib_formal_params = INIT_FIRST_OF_ALT_ATTRIB_FORMAL_PARAMS,
  first_of_alt_type_decl_tail = INIT_FIRST_OF_ALT_TYPE_DECL_TAIL
  ;

/* --------------------------------------------------------------------------
 * FOLLOW set data structures
 * ----------------------------------------------------------------------- */

m2c_tokenset_literal_t
  follow_of_definition_module = INIT_FOLLOW_OF_DEFINITION_MODULE,
  follow_of_import = INIT_FOLLOW_OF_IMPORT,
  follow_of_qualified_import = INIT_FOLLOW_OF_QUALIFIED_IMPORT,
  follow_of_unqualified_import = INIT_FOLLOW_OF_UNQUALIFIED_IMPORT,
  follow_of_ident_list = INIT_FOLLOW_OF_IDENT_LIST,
  follow_of_definition = INIT_FOLLOW_OF_DEFINITION,
  follow_of_const_definition = INIT_FOLLOW_OF_CONST_DEFINITION,
  follow_of_type_definition = INIT_FOLLOW_OF_TYPE_DEFINITION,
  follow_of_type = INIT_FOLLOW_OF_TYPE,
  follow_of_derived_or_subr_type = INIT_FOLLOW_OF_DERIVED_OR_SUBRANGE_TYPE,
  follow_of_qualident = INIT_FOLLOW_OF_QUALIDENT,
  follow_of_range = INIT_FOLLOW_OF_RANGE,
  follow_of_enum_type = INIT_FOLLOW_OF_ENUM_TYPE,
  follow_of_set_type = INIT_FOLLOW_OF_SET_TYPE,
  follow_of_countable_type = INIT_FOLLOW_OF_COUNTABLE_TYPE,
  follow_of_array_type = INIT_FOLLOW_OF_ARRAY_TYPE,
  follow_of_extensible_record_type = INIT_FOLLOW_OF_EXTENSIBLE_RECORD_TYPE,
  follow_of_field_list_sequence = INIT_FOLLOW_OF_FIELD_LIST_SEQUENCE,
  follow_of_variant_record_type = INIT_FOLLOW_OF_VARIANT_RECORD_TYPE,
  follow_of_variant_field_list_seq = INIT_FOLLOW_OF_VARIANT_FIELD_LIST_SEQ,
  follow_of_variant_field_list = INIT_FOLLOW_OF_VARIANT_FIELD_LIST,
  follow_of_variant_fields = INIT_FOLLOW_OF_VARIANT_FIELDS,
  follow_of_variant = INIT_FOLLOW_OF_VARIANT,
  follow_of_case_label_list = INIT_FOLLOW_OF_CASE_LABEL_LIST,
  follow_of_case_labels = INIT_FOLLOW_OF_CASE_LABELS,
  follow_of_pointer_type = INIT_FOLLOW_OF_POINTER_TYPE,
  follow_of_procedure_type = INIT_FOLLOW_OF_PROCEDURE_TYPE,
  follow_of_simple_formal_type = INIT_FOLLOW_OF_SIMPLE_FORMAL_TYPE,
  follow_of_procedure_header = INIT_FOLLOW_OF_PROCEDURE_HEADER,
  follow_of_procedure_signature = INIT_FOLLOW_OF_PROCEDURE_SIGNATURE,
  follow_of_simple_formal_params = INIT_FOLLOW_OF_SIMPLE_FORMAL_PARAMS,
  follow_of_implementation_module = INIT_FOLLOW_OF_IMPLEMENTATION_MODULE,
  follow_of_program_module = INIT_FOLLOW_OF_PROGRAM_MODULE,
  follow_of_module_priority = INIT_FOLLOW_OF_MODULE_PRIORITY,
  follow_of_block = INIT_FOLLOW_OF_BLOCK,
  follow_of_declaration = INIT_FOLLOW_OF_DECLARATION,
  follow_of_type_declaration = INIT_FOLLOW_OF_TYPE_DECLARATION,
  follow_of_var_size_record_type = INIT_FOLLOW_OF_VAR_SIZE_RECORD_TYPE,
  follow_of_variable_declaration = INIT_FOLLOW_OF_VARIABLE_DECLARATION,
  follow_of_procedure_declaration = INIT_FOLLOW_OF_PROCEDURE_DECLARATION,
  follow_of_module_declaration = INIT_FOLLOW_OF_MODULE_DECLARATION,
  follow_of_export = INIT_FOLLOW_OF_EXPORT,
  follow_of_statement_sequence = INIT_FOLLOW_OF_STATEMENT_SEQUENCE,
  follow_of_statement = INIT_FOLLOW_OF_STATEMENT,
  follow_of_assignment_or_proc_call = INIT_FOLLOW_OF_ASSIGNMENT_OR_PROC_CALL,
  follow_of_actual_parameters = INIT_FOLLOW_OF_ACTUAL_PARAMETERS,
  follow_of_expression_list = INIT_FOLLOW_OF_EXPRESSION_LIST,
  follow_of_return_statement = INIT_FOLLOW_OF_RETURN_STATEMENT,
  follow_of_with_statement = INIT_FOLLOW_OF_WITH_STATEMENT,
  follow_of_if_statement = INIT_FOLLOW_OF_IF_STATEMENT,
  follow_of_case_statement = INIT_FOLLOW_OF_CASE_STATEMENT,
  follow_of_case = INIT_FOLLOW_OF_CASE,
  follow_of_loop_statement = INIT_FOLLOW_OF_LOOP_STATEMENT,
  follow_of_while_statement = INIT_FOLLOW_OF_WHILE_STATEMENT,
  follow_of_repeat_statement = INIT_FOLLOW_OF_REPEAT_STATEMENT,
  follow_of_for_statement = INIT_FOLLOW_OF_FOR_STATEMENT,
  follow_of_designator = INIT_FOLLOW_OF_DESIGNATOR,
  follow_of_selector = INIT_FOLLOW_OF_SELECTOR,
  follow_of_expression = INIT_FOLLOW_OF_EXPRESSION,
  follow_of_simple_expression = INIT_FOLLOW_OF_SIMPLE_EXPRESSION,
  follow_of_term = INIT_FOLLOW_OF_TERM,
  follow_of_simple_term = INIT_FOLLOW_OF_SIMPLE_TERM,
  follow_of_factor = INIT_FOLLOW_OF_FACTOR,
  follow_of_designator_or_func_call = INIT_FOLLOW_OF_DESIGNATOR_OR_FUNC_CALL,
  follow_of_set_value = INIT_FOLLOW_OF_SET_VALUE,
  follow_of_element = INIT_FOLLOW_OF_ELEMENT,
  
  /* Option dependent FOLLOW sets */
  
  follow_of_formal_type = INIT_FOLLOW_OF_FORMAL_TYPE,
  follow_of_attributed_formal_type = INIT_FOLLOW_OF_ATTRIBUTED_FORMAL_TYPE,
  follow_of_formal_param_list = INIT_FOLLOW_OF_FORMAL_PARAM_LIST,
  follow_of_formal_params = INIT_FOLLOW_OF_FORMAL_PARAMS,
  follow_of_attrib_formal_params = INIT_FOLLOW_OF_ATTRIB_FORMAL_PARAMS,
  follow_of_type_declaration_tail = INIT_FOLLOW_OF_TYPE_DECLARATION_TAIL,
  
  /* Alternative FOLLOW sets */
  
  follow_of_alt_formal_type = INIT_FOLLOW_OF_ALT_FORMAL_TYPE,
  follow_of_alt_attrib_formal_type = INIT_FOLLOW_OF_ALT_ATTRIB_FORMAL_TYPE,
  follow_of_alt_formal_param_list = INIT_FOLLOW_OF_ALT_FORMAL_PARAM_LIST,
  follow_of_alt_formal_params = INIT_FOLLOW_OF_ALT_FORMAL_PARAMS,
  follow_of_alt_attrib_formal_params =
    INIT_FOLLOW_OF_ALT_ATTRIB_FORMAL_PARAMS,
  follow_of_alt_type_decl_tail = INIT_FOLLOW_OF_ALT_TYPE_DECL_TAIL
  ;

/* --------------------------------------------------------------------------
 * private variable m2c_first_set
 * --------------------------------------------------------------------------
 * Table of pointers to FIRST set data structures
 * ----------------------------------------------------------------------- */

m2c_tokenset_literal_t *m2c_first_set[] = {
  &first_of_definition_module,        /* FIRST(definitionModule) */
  &first_of_import,                   /* FIRST(import) */
  &first_of_qualified_import,         /* FIRST(qualifiedImport) */
  &first_of_unqualified_import,       /* FIRST(unqualifiedImport) */
  &first_of_ident_list,               /* FIRST(identList) */
  &first_of_definition,               /* FIRST(definition) */
  &first_of_const_definition,         /* FIRST(constDefinition) */
  &first_of_type_definition,          /* FIRST(typeDefinition) */
  &first_of_type,                     /* FIRST(type) */
  &first_of_derived_or_subr_type,     /* FIRST(derivedOrSubrangeType) */
  &first_of_qualident,                /* FIRST(qualident) */
  &first_of_range,                    /* FIRST(range) */
  &first_of_enum_type,                /* FIRST(enumType) */
  &first_of_set_type,                 /* FIRST(setType) */
  &first_of_countable_type,           /* FIRST(countableType) */
  &first_of_array_type,               /* FIRST(arrayType) */
  &first_of_extensible_record_type,   /* FIRST(extensibleRecordType) */
  &first_of_field_list_sequence,      /* FIRST(fieldListSequence) */
  &first_of_variant_record_type,      /* FIRST(variantRecordType) */
  &first_of_variant_field_list_seq,   /* FIRST(variantFieldListSeq) */
  &first_of_variant_field_list,       /* FIRST(variantFieldList) */
  &first_of_variant_fields,           /* FIRST(variantFields) */
  &first_of_variant,                  /* FIRST(variant) */
  &first_of_case_label_list,          /* FIRST(caseLabelList) */
  &first_of_case_labels,              /* FIRST(caseLabels) */
  &first_of_pointer_type,             /* FIRST(pointerType) */
  &first_of_procedure_type,           /* FIRST(procedureType) */
  &first_of_simple_formal_type,       /* FIRST(simpleFormalType) */
  &first_of_procedure_header,         /* FIRST(procedureHeader) */
  &first_of_procedure_signature,      /* FIRST(procedureSignature) */
  &first_of_simple_formal_params,     /* FIRST(simpleFormalParams) */
  &first_of_implementation_module,    /* FIRST(implementationModule) */
  &first_of_program_module,           /* FIRST(programModule) */
  &first_of_module_priority,          /* FIRST(modulePriority) */
  &first_of_block,                    /* FIRST(block) */
  &first_of_declaration,              /* FIRST(declaration) */
  &first_of_type_declaration,         /* FIRST(typeDeclaration) */
  &first_of_var_size_record_type,     /* FIRST(varSizeRecordType) */
  &first_of_variable_declaration,     /* FIRST(variableDeclaration) */
  &first_of_procedure_declaration,    /* FIRST(procedureDeclaration) */
  &first_of_module_declaration,       /* FIRST(moduleDeclaration) */
  &first_of_export,                   /* FIRST(export) */
  &first_of_statement_sequence,       /* FIRST(statementSequence) */
  &first_of_statement,                /* FIRST(statement) */
  &first_of_assignment_or_proc_call,  /* FIRST(assignmentOrProcCall) */
  &first_of_actual_parameters,        /* FIRST(actualParameters) */
  &first_of_expression_list,          /* FIRST(expressionList) */
  &first_of_return_statement,         /* FIRST(returnStatement) */
  &first_of_with_statement,           /* FIRST(withStatement) */
  &first_of_if_statement,             /* FIRST(ifStatement) */
  &first_of_case_statement,           /* FIRST(caseStatement) */
  &first_of_case,                     /* FIRST(case) */
  &first_of_loop_statement,           /* FIRST(loopStatement) */
  &first_of_while_statement,          /* FIRST(whileStatement) */
  &first_of_repeat_statement,         /* FIRST(repeatStatement) */
  &first_of_for_statement,            /* FIRST(forStatement) */
  &first_of_designator,               /* FIRST(designator) */
  &first_of_selector,                 /* FIRST(selector) */
  &first_of_expression,               /* FIRST(expression) */
  &first_of_simple_expression,        /* FIRST(simpleExpression) */
  &first_of_term,                     /* FIRST(term) */
  &first_of_simple_term,              /* FIRST(simpleTerm) */
  &first_of_factor,                   /* FIRST(factor) */
  &first_of_designator_or_func_call,  /* FIRST(designatorOrFuncCall) */
  &first_of_set_value,                /* FIRST(setValue) */
  &first_of_element,                  /* FIRST(element) */
  
  /* Option dependent FIRST sets */
  
  &first_of_formal_type,              /* FIRST(formalType) */
  &first_of_attributed_formal_type,   /* FIRST(attributedFormalType) */
  &first_of_formal_param_list,        /* FIRST(formalParamList) */
  &first_of_formal_params,            /* FIRST(formalParams) */
  &first_of_attrib_formal_params,     /* FIRST(attribFormalParams) */
  &first_of_type_declaration_tail,    /* FIRST(typeDeclarationTail) */
  
  /* Alternative FIRST sets */
  
  &first_of_alt_formal_type,          /* FIRST(formalType) */
  &first_of_alt_attrib_formal_type,   /* FIRST(attributedFormalType) */
  &first_of_alt_formal_param_list,    /* FIRST(formalParamList) */
  &first_of_alt_formal_params,        /* FIRST(formalParams) */
  &first_of_alt_attrib_formal_params, /* FIRST(attribFormalParams) */
  &first_of_alt_type_decl_tail        /* FIRST(typeDeclarationTail) */
}; /* end m2c_first_set */


/* --------------------------------------------------------------------------
 * private variable m2c_follow_set
 * --------------------------------------------------------------------------
 * Table of pointers to FOLLOW set data structures
 * ----------------------------------------------------------------------- */

m2c_tokenset_literal_t *m2c_follow_set[] = {
  &follow_of_definition_module,        /* FOLLOW(definitionModule) */
  &follow_of_import,                   /* FOLLOW(import) */
  &follow_of_qualified_import,         /* FOLLOW(qualifiedImport) */
  &follow_of_unqualified_import,       /* FOLLOW(unqualifiedImport) */
  &follow_of_ident_list,               /* FOLLOW(identList) */
  &follow_of_definition,               /* FOLLOW(definition) */
  &follow_of_const_definition,         /* FOLLOW(constDefinition) */
  &follow_of_type_definition,          /* FOLLOW(typeDefinition) */
  &follow_of_type,                     /* FOLLOW(type) */
  &follow_of_qualident,                /* FOLLOW(qualident) */
  &follow_of_derived_or_subr_type,     /* FOLLOW(derivedOrSubrangeType) */
  &follow_of_range,                    /* FOLLOW(range) */
  &follow_of_enum_type,                /* FOLLOW(enumType) */
  &follow_of_set_type,                 /* FOLLOW(setType) */
  &follow_of_countable_type,           /* FOLLOW(countableType) */
  &follow_of_array_type,               /* FOLLOW(arrayType) */
  &follow_of_extensible_record_type,   /* FOLLOW(extensibleRecordType) */
  &follow_of_field_list_sequence,      /* FOLLOW(fieldListSequence) */
  &follow_of_variant_record_type,      /* FOLLOW(variantRecordType) */
  &follow_of_variant_field_list_seq,   /* FOLLOW(variantFieldListSeq) */
  &follow_of_variant_field_list,       /* FOLLOW(variantFieldList) */
  &follow_of_variant_fields,           /* FOLLOW(variantFields) */
  &follow_of_variant,                  /* FOLLOW(variant) */
  &follow_of_case_label_list,          /* FOLLOW(caseLabelList) */
  &follow_of_case_labels,              /* FOLLOW(caseLabels) */
  &follow_of_pointer_type,             /* FOLLOW(pointerType) */
  &follow_of_procedure_type,           /* FOLLOW(procedureType) */
  &follow_of_simple_formal_type,       /* FOLLOW(simpleFormalType) */
  &follow_of_procedure_header,         /* FOLLOW(procedureHeader) */
  &follow_of_procedure_signature,      /* FOLLOW(procedureSignature) */
  &follow_of_simple_formal_params,     /* FOLLOW(simpleFormalParams) */
  &follow_of_implementation_module,    /* FOLLOW(implementationModule) */
  &follow_of_program_module,           /* FOLLOW(programModule) */
  &follow_of_module_priority,          /* FOLLOW(modulePriority) */
  &follow_of_block,                    /* FOLLOW(block) */
  &follow_of_declaration,              /* FOLLOW(declaration) */
  &follow_of_type_declaration,         /* FOLLOW(typeDeclaration) */
  &follow_of_var_size_record_type,     /* FOLLOW(varSizeRecordType) */
  &follow_of_variable_declaration,     /* FOLLOW(variableDeclaration) */
  &follow_of_procedure_declaration,    /* FOLLOW(procedureDeclaration) */
  &follow_of_module_declaration,       /* FOLLOW(moduleDeclaration) */
  &follow_of_export,                   /* FOLLOW(export) */
  &follow_of_statement_sequence,       /* FOLLOW(statementSequence) */
  &follow_of_statement,                /* FOLLOW(statement) */
  &follow_of_assignment_or_proc_call,  /* FOLLOW(assignmentOrProcCall) */
  &follow_of_actual_parameters,        /* FOLLOW(actualParameters) */
  &follow_of_expression_list,          /* FOLLOW(expressionList) */
  &follow_of_return_statement,         /* FOLLOW(returnStatement) */
  &follow_of_with_statement,           /* FOLLOW(withStatement) */
  &follow_of_if_statement,             /* FOLLOW(ifStatement) */
  &follow_of_case_statement,           /* FOLLOW(caseStatement) */
  &follow_of_case,                     /* FOLLOW(case) */
  &follow_of_loop_statement,           /* FOLLOW(loopStatement) */
  &follow_of_while_statement,          /* FOLLOW(whileStatement) */
  &follow_of_repeat_statement,         /* FOLLOW(repeatStatement) */
  &follow_of_for_statement,            /* FOLLOW(forStatement) */
  &follow_of_designator,               /* FOLLOW(designator) */
  &follow_of_selector,                 /* FOLLOW(selector) */
  &follow_of_expression,               /* FOLLOW(expression) */
  &follow_of_simple_expression,        /* FOLLOW(simpleExpression) */
  &follow_of_term,                     /* FOLLOW(term) */
  &follow_of_simple_term,              /* FOLLOW(simpleTerm) */
  &follow_of_factor,                   /* FOLLOW(factor) */
  &follow_of_designator_or_func_call,  /* FOLLOW(designatorOrFuncCall) */
  &follow_of_set_value,                /* FOLLOW(setValue) */
  &follow_of_element,                  /* FOLLOW(element) */
  
  /* Option dependent FOLLOW sets */
  
  &follow_of_formal_type,              /* FOLLOW(formalType) */
  &follow_of_attributed_formal_type,   /* FOLLOW(attributedFormalType) */
  &follow_of_formal_param_list,        /* FOLLOW(formalParamList) */
  &follow_of_formal_params,            /* FOLLOW(formalParams) */
  &follow_of_attrib_formal_params,     /* FOLLOW(attribFormalParams) */
  &follow_of_type_declaration_tail,    /* FOLLOW(typeDeclarationTail) */
  
  /* Alternative FOLLOW sets */
  
  &follow_of_alt_formal_type,          /* FOLLOW(formalType) */
  &follow_of_alt_attrib_formal_type,   /* FOLLOW(attributedFormalType) */
  &follow_of_alt_formal_param_list,    /* FOLLOW(formalParamList) */
  &follow_of_alt_formal_params,        /* FOLLOW(formalParams) */
  &follow_of_alt_attrib_formal_params, /* FOLLOW(attribFormalParams) */
  &follow_of_alt_type_decl_tail        /* FOLLOW(typeDeclarationTail) */
}; /* end m2c_follow_set */


/* --------------------------------------------------------------------------
 * private variable m2c_production_name_table
 * --------------------------------------------------------------------------
 * Table of pointers to human readable production names
 * ----------------------------------------------------------------------- */

const char *m2c_production_name_table[] = {
  "definitionModule\0",
  "import\0",
  "qualifiedImport\0",
  "unqualifiedImport\0",
  "identList\0",
  "definition\0",
  "constDefinition\0",
  "typeDefinition\0",
  "type\0",
  "derivedOrSubrangeType\0",
  "qualident\0",
  "range\0",
  "enumType\0",
  "setType\0",
  "countableType\0",
  "arrayType\0",
  "extensibleRecordType\0",
  "fieldListSequence\0",
  "variantRecordType\0",
  "variantFieldListSeq\0",
  "variantFieldList\0",
  "variantFields\0",
  "variant\0",
  "caseLabelList\0",
  "caseLabels\0",
  "pointerType\0",
  "procedureType\0",
  "simpleFormalType\0",
  "procedureHeader\0",
  "procedureSignature\0",
  "simpleFormalParams\0",
  "implementationModule\0",
  "programModule\0",
  "modulePriority\0",
  "block\0",
  "declaration\0",
  "typeDeclaration\0",
  "varSizeRecordType\0",
  "variableDeclaration\0",
  "procedureDeclaration\0",
  "moduleDeclaration\0",
  "export\0",
  "statementSequence\0",
  "statement\0",
  "assignmentOrProcCall\0",
  "actualParameters\0",
  "expressionList\0",
  "returnStatement\0",
  "withStatement\0",
  "ifStatement\0",
  "caseStatement\0",
  "case\0",
  "loopStatement\0",
  "whileStatement\0",
  "repeatStatement\0",
  "forStatement\0",
  "designator\0",
  "selector\0",
  "expression\0",
  "simpleExpression\0",
  "term\0",
  "simpleTerm\0",
  "factor\0",
  "designatorOrFuncCall\0",
  "setValue\0",
  "element\0"
  "formalType\0",
  "attributedFormalType\0",
  "formalParamList\0",
  "formalParams\0",
  "attribFormalParams\0",
  "typeDeclarationTail\0",
}; /* end m2c_production_name_table */


/* --------------------------------------------------------------------------
 * function FIRST(p)
 * --------------------------------------------------------------------------
 * Returns a tokenset with the FIRST set of production p.
 * ----------------------------------------------------------------------- */

#define ALTERNATE_SET_OFFSET \
  (M2C_LAST_OPTION_DEPENDENT - M2C_FIRST_OPTION_DEPENDENT + 1)

#define IS_CONST_PARAM_DEPENDENT(_p) \
  (((_p) >= M2C_FIRST_CONST_PARAM_DEPENDENT) && \
   ((_p) <= M2C_LAST_CONST_PARAM_DEPENDENT))

#define IS_NO_VARIANT_REC_DEPENDENT(_p) \
  (((_p) >= M2C_FIRST_NO_VARIANT_REC_DEPENDENT) && \
   ((_p) <= M2C_LAST_NO_VARIANT_REC_DEPENDENT))

m2c_tokenset_t FIRST (m2c_production_t p) {
  if (M2C_IS_INVALID_PRODUCTION(p)) {
    return NULL;
  } /* end if */
  
  if ((IS_CONST_PARAM_DEPENDENT(p)) && (!m2c_option_const_parameters())) {
    p = p + ALTERNATE_SET_OFFSET;
  } /* end if */
  
  if ((IS_NO_VARIANT_REC_DEPENDENT(p)) && (m2c_option_variant_records())) {
    p = p + ALTERNATE_SET_OFFSET;
  } /* end if */
  
  return (m2c_tokenset_t) m2c_first_set[p];
} /* end FIRST */


/* --------------------------------------------------------------------------
 * function FOLLOW(p)
 * --------------------------------------------------------------------------
 * Returns a tokenset with the FOLLOW set of production p.
 * ----------------------------------------------------------------------- */

m2c_tokenset_t FOLLOW (m2c_production_t p) {
  if (M2C_IS_INVALID_PRODUCTION(p)) {
    return NULL;
  } /* end if */
  
  if ((IS_CONST_PARAM_DEPENDENT(p)) && (!m2c_option_const_parameters())) {
    p = p + ALTERNATE_SET_OFFSET;
  } /* end if */
  
  if ((IS_NO_VARIANT_REC_DEPENDENT(p)) && (!m2c_option_variant_records())) {
    p = p + ALTERNATE_SET_OFFSET;
  } /* end if */
  
  return (m2c_tokenset_t) m2c_follow_set[p];
} /* end FOLLOW */


/* --------------------------------------------------------------------------
 * function m2c_name_for_production(p)
 * --------------------------------------------------------------------------
 * Returns an immutable pointer to a NUL terminated character string with
 * a human readable name for p.  Returns NULL if p is not valid.
 * ----------------------------------------------------------------------- */

const char *m2c_name_for_production (m2c_production_t p) {
  if (M2C_IS_INVALID_PRODUCTION(p)) {
    return NULL;
  } /* end if */
  
  return m2c_production_name_table[p];
} /* end m2c_name_for_production */

/* END OF FILE */