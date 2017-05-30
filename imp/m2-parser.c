/* M2C Modula-2 Compiler & Translator
 * Copyright (c) 2015-2016 Benjamin Kowarsch
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
 * m2-parser.c
 *
 * Implementation of M2C parser module.
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

#include "m2-parser.h"

#include "m2-lexer.h"
#include "m2-error.h"
#include "m2-tokenset.h"
#include "m2-fileutils.h"
#include "m2-production.h"
#include "m2-resync-sets.h"
#include "m2-compiler-options.h"

#include <stdio.h>
#include <stdlib.h>


#define PARSER_DEBUG_INFO(_str) \
  { if (m2c_option_parser_debug()) \
      printf("*** %s ***\n  @ line: %u, column: %u, lookahead: %s\n", _str, \
        m2c_lexer_lookahead_line(p->lexer), \
        m2c_lexer_lookahead_column(p->lexer), \
        m2c_string_char_ptr(m2c_lexer_lookahead_lexeme(p->lexer))); }


/* --------------------------------------------------------------------------
 * private type m2c_parser_context_t
 * --------------------------------------------------------------------------
 * Pointer type to represent parser context.
 * ----------------------------------------------------------------------- */

typedef struct m2c_parser_context_s *m2c_parser_context_t;


/* --------------------------------------------------------------------------
 * private type m2c_nonterminal_f
 * --------------------------------------------------------------------------
 * function pointer type for function to parse a non-terminal symbol.
 * ----------------------------------------------------------------------- */

typedef m2c_token_t (m2c_nonterminal_f) (m2c_parser_context_t);


/* --------------------------------------------------------------------------
 * forward declarations of alternative parsing functions.
 * ----------------------------------------------------------------------- */

/* for use with compiler option --variant-records */
m2c_token_t variant_record_type (m2c_parser_context_t p);

/* for use with compiler option --no-variant-records */
m2c_token_t extensible_record_type (m2c_parser_context_t p);


/* --------------------------------------------------------------------------
 * private type m2c_parser_context_s
 * --------------------------------------------------------------------------
 * Record type to implement parser context.
 * ----------------------------------------------------------------------- */

struct m2c_parser_context_s {
  /* filename */      const char *filename;
  /* lexer */         m2c_lexer_t lexer;
  /* ast */           m2c_astnode_t ast;
  /* warning_count */ uint_t warning_count;
  /* error_count */   uint_t error_count;
  /* status */        m2c_parser_status_t status;
  /* record_type */   m2c_nonterminal_f *record_type;
};

typedef struct m2c_parser_context_s m2c_parser_context_s;


/* --------------------------------------------------------------------------
 * function m2c_parse_file(srctype, srcpath, ast, stats, status)
 * --------------------------------------------------------------------------
 * Parses a Modula-2 source file represented by srcpath and returns status.
 * Builds an abstract syntax tree and passes it back in ast, or NULL upon
 * failure.  Collects simple statistics and passes them back in stats.
 * ----------------------------------------------------------------------- */

static void parse_start_symbol
  (m2c_sourcetype_t srctype, m2c_parser_context_t p);
 
void m2c_parse_file
  (m2c_sourcetype_t srctype,
   const char *srcpath,
   m2c_ast_t *ast,
   m2c_stats_t *stats,
   m2c_parser_status_t *status) {
  
  const char *filename;
  m2c_parser_context_t p;
  uint_t line_count;
  
  if ((srctype < M2C_FIRST_SOURCETYPE) || (srctype > M2C_LAST_SOURCETYPE)) {
    SET_STATUS(status, M2C_PARSER_STATUS_INVALID_SOURCETYPE);
    return;
  } /* end if */
  
  if ((srcpath == NULL) || (srcpath[0] == ASCII_NUL)) {
    SET_STATUS(status, M2C_PARSER_STATUS_INVALID_REFERENCE);
    return;
  } /* end if */
  
  /* set up parser context */
  p = malloc(sizeof(m2c_parser_context_s));
  
  if (p == NULL) {
    SET_STATUS(status, M2C_PARSER_STATUS_ALLOCATION_FAILED);
    return;
  } /* end if */
  
  /* create lexer object */
  m2c_new_lexer(&(p->lexer), srcpath, NULL);
  
  if (p->lexer == NULL) {
    SET_STATUS(status, M2C_PARSER_STATUS_ALLOCATION_FAILED);
    free(p);
    return;
  } /* end if */
  
  /* init context */
  p->filename = filename;
  p->ast = NULL;
  p->warning_count = 0;
  p->error_count = 0;
  p->status = 0;
  
  if (m2c_option_variant_records()) {
    /* install function to parse variant records */
    p->record_type = variant_record_type;
  }
  else /* extensible records */ {
    /* install function to parse extensible records */
    p->record_type = extensible_record_type;
  } /* end if */
  
  /* parse and build AST */
  parse_start_symbol(srctype, p);
  line_count = m2c_lexer_lookahead_line(p->lexer);
  
  /* pass back AST, statistics and return status */
  *ast = p->ast;
  *stats = m2c_stats_new(p->warning_count, p->error_count, line_count);
  SET_STATUS(status, p->status);
  
  /* clean up and return */
  m2c_release_lexer(p->lexer, NULL);
  free(p);
  return;
} /* end m2c_parse_file */


/* --------------------------------------------------------------------------
 * private function match_token(p, expected_token, resync_set)
 * --------------------------------------------------------------------------
 * Matches the lookahead symbol to expected_token and returns true if they
 * match.  If they don't match, a syntax error is reported, the error count
 * is incremented, symbols are consumed until the lookahead symbol matches
 * one of the symbols in resync_set and false is returned.
 * ----------------------------------------------------------------------- */

bool match_token
  (m2c_parser_context_t p,
   m2c_token_t expected_token,
   m2c_tokenset_t resync_set) {
  
  m2c_string_t lexeme;
  const char *lexstr;
  uint_t line, column;
  m2c_token_t lookahead;
  
  lookahead = m2c_next_sym(p->lexer);
  
  if (lookahead == expected_token) {
    return true;
  }
  else /* no match */ {
    /* get details for offending lookahead symbol */
    line = m2c_lexer_lookahead_line(p->lexer);
    column = m2c_lexer_lookahead_column(p->lexer);
    lexeme = m2c_lexer_lookahead_lexeme(p->lexer);
    lexstr = m2c_string_char_ptr(lexeme);
    
    /* report error */
    m2c_emit_syntax_error_w_token
      (line, column, lookahead, lexstr, expected_token);
    
    /* print source line */
    if (m2c_option_verbose()) {
      m2c_print_line_and_mark_column(p->lexer, line, column);
    } /* end if */
    
    /* update error count */
    p->error_count++;
    
    /* skip symbols until lookahead matches resync_set */
    while (!m2c_tokenset_element(resync_set, lookahead)) {
      lookahead = m2c_consume_sym(p->lexer);
    } /* end while */
    return false;
  } /* end if */
} /* end match_token */


/* --------------------------------------------------------------------------
 * private function match_set(p, expected_set, resync_set)
 * --------------------------------------------------------------------------
 * Matches the lookahead symbol to set expected_set and returns true if it
 * matches any of the tokens in the set.  If there is no match, a syntax
 * error is reported, the error count is incremented, symbols are consumed
 * until the lookahead symbol matches one of the symbols in resync_set and
 * false is returned.
 * ----------------------------------------------------------------------- */

bool match_set
  (m2c_parser_context_t p,
   m2c_tokenset_t expected_set,
   m2c_tokenset_t resync_set) {
  
  m2c_string_t lexeme;
  const char *lexstr;
  uint_t line, column;
  m2c_token_t lookahead;
  
  lookahead = m2c_next_sym(p->lexer);
  
  /* check if lookahead matches any token in expected_set */
  if (m2c_tokenset_element(expected_set, lookahead)) {
    return true;
  }
  else /* no match */ {
    /* get details for offending lookahead symbol */
    line = m2c_lexer_lookahead_line(p->lexer);
    column = m2c_lexer_lookahead_column(p->lexer);
    lexeme = m2c_lexer_lookahead_lexeme(p->lexer);
    lexstr = m2c_string_char_ptr(lexeme);
    
    /* report error */
    m2c_emit_syntax_error_w_set
      (line, column, lookahead, lexstr, expected_set);
    
    /* print source line */
    if (m2c_option_verbose()) {
      m2c_print_line_and_mark_column(p->lexer, line, column);
    } /* end if */
        
    /* update error count */
    p->error_count++;
    
    /* skip symbols until lookahead matches resync_set */
    while (!m2c_tokenset_element(resync_set, lookahead)) {
      lookahead = m2c_consume_sym(p->lexer);
    } /* end while */
    return false;
  } /* end if */
} /* end match_set */


/* --------------------------------------------------------------------------
 * procedure report_error_w_offending_lexeme(error, lexeme, line, column)
 * ----------------------------------------------------------------------- */

static void report_error_w_offending_lexeme
  (m2c_error_t error,
   m2c_parser_context_t p,
   m2c_string_t lexeme, uint_t line, uint_t column) {
    
  m2c_emit_error_w_lex(error, line, column, m2c_string_char_ptr(lexeme));
  
  if (m2c_option_verbose()) {
    m2c_print_line_and_mark_column(p->lexer, line, column);
  } /* end if */
  
  p->error_count++;
  return;
} /* end report_error_w_offending_lexeme */


/* ************************************************************************ *
 * Syntax Analysis                                                          *
 * ************************************************************************ */

/* --------------------------------------------------------------------------
 * private procedure parse_start_symbol(srctype, p)
 * ----------------------------------------------------------------------- */

static void parse_start_symbol
  (m2c_sourcetype_t srctype, m2c_parser_context_t p) {
  
  m2c_astnode_t id, opt;
  m2c_string_t ident;
  m2c_token_t lookahead;
  
  lookahead = m2c_next_sym(p->lexer);
  
  switch (srctype) {
    M2C_ANY_SOURCE :
      if ((lookahead == TOKEN_DEFINITION) ||
          (lookahead == TOKEN_IMPLEMENTATION) ||
          (lookahead == TOKEN_MODULE)) {
        lookahead = compilation_unit(p);
      }
      else /* invalid start symbol */ {
        /* TO DO: report error */
        p->status = M2C_PARSER_STATUS_INVALID_START_SYMBOL;
      } /* end if */
      break;
    
    M2C_DEF_SOURCE :
      if (lookahead == TOKEN_DEFINITION) {
        lookahead = definition_module(p);
      }
      else /* invalid start symbol */ {
        /* TO DO: report error */
        p->status = M2C_PARSER_STATUS_INVALID_START_SYMBOL;
      } /* end if */
      break;
    
    M2C_MOD_SOURCE :
      if (lookahead == TOKEN_IMPLEMENTATION) {
        lookahead = implementation_module(p);
      }
      else if (lookahead == TOKEN_MODULE) {
        lookahead = program_module(p);
      }
      else /* invalid start symbol */ {
        /* TO DO: report error */
        p->status = M2C_PARSER_STATUS_INVALID_START_SYMBOL;
      } /* end if */
      break;
  } /* end switch */
  
  /* build AST node and pass it back in p->ast */
  ident = m2c_get_string(p->filename);
  id = m2c_ast_new_terminal_node(AST_IDENT, ident);
  opt = m2c_ast_empty_node(); /* TO DO : encode options */
  p->ast = m2c_ast_new_node(AST_ROOT, id, opt, p->ast, NULL);
  
  if (lookahead != TOKEN_EOF) {
    /* TO DO: report error -- extra symbols after end of compilation unit */
  } /* end if */
} /* end parse_start_symbol */


/* --------------------------------------------------------------------------
 * private function compilation_unit()
 * --------------------------------------------------------------------------
 * compilationUnit :=
 *   definitionModule | implementationModule | programModule
 *   ;
 *
 * astnode: defModuleNode | impModuleNode
 * ----------------------------------------------------------------------- */

m2c_token_t compilation_unit (m2c_parser_context_t p) {
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("compilationUnit");
  
  m2c_token_t lookahead = m2c_next_sym(p->lexer);
  
  switch (lookahead) {
    case TOKEN_DEFINITION :
      lookahead = definition_module(p);
      break;
      
    case TOKEN_IMPLEMENTATION :
      lookahead = implementation_module(p);
      break;
      
    case TOKEN_MODULE:
      lookahead = program_module(p);
      break;
  } /* end switch */
  
  return lookahead;
} /* end compilation_unit */


/* ************************************************************************ *
 * Definition Module Syntax                                                 *
 * ************************************************************************ */

/* --------------------------------------------------------------------------
 * alias const_expression()
 * ----------------------------------------------------------------------- */

#define const_expression(_p) expression(_p)


/* --------------------------------------------------------------------------
 * private function definition_module()
 * --------------------------------------------------------------------------
 * definitionModule :=
 *   DEFINITION MODULE moduleIdent ';'
 *   import* definition* END moduleIdent '.'
 *   ;
 *
 * moduleIdent := Ident ;
 *
 * astnode: (DEFMOD identNode implist deflist)
 * ----------------------------------------------------------------------- */

m2c_token_t import (m2c_parser_context_t p);

m2c_token_t definition (m2c_parser_context_t p);

m2c_token_t definition_module (m2c_parser_context_t p) {
  m2c_astnode_t id, implist, deflist;
  m2c_string_t ident1, ident2;
  m2c_fifo_t tmplist;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("definitionModule");
  
  /* DEFINITION */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* MODULE */
  if (match_token(p, TOKEN_MODULE, RESYNC(IMPORT_OR_DEFINITON_OR_END))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* moduleIdent */
    if (match_token(p, TOKEN_IDENTIFIER,
        RESYNC(IMPORT_OR_DEFINITON_OR_END))) {
      lookahead = m2c_consume_sym(p->lexer);
      ident1 = m2c_lexer_current_lexeme(p->lexer);
      
      /* ';' */
      if (match_token(p, TOKEN_SEMICOLON,
          RESYNC(IMPORT_OR_DEFINITON_OR_END))) {
        lookahead = m2c_consume_sym(p->lexer);
      }
      else /* resync */ {
        lookahead = m2c_next_sym(p->lexer);
      } /* end if */
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  }
  else /* resync */ {
    lookahead = m2c_next_sym(p->lexer);
  } /* end if */
  
  tmplist = m2c_fifo_new_queue(NULL);

  /* import* */
  while ((lookahead == TOKEN_IMPORT) ||
         (lookahead == TOKEN_FROM)) {
    lookahead = import(p);
    m2c_fifo_enqueue(tmplist, p->ast);
  } /* end while */
  
  implist = m2c_ast_new_list_node(AST_IMPLIST, tmplist);
  m2c_fifo_reset_queue(tmplist);
  
  /* definition* */
  while ((lookahead == TOKEN_CONST) ||
         (lookahead == TOKEN_TYPE) ||
         (lookahead == TOKEN_VAR) ||
         (lookahead == TOKEN_PROCEDURE)) {
    lookahead = definition(p);
    m2c_fifo_enqueue(tmplist, p->ast);
  } /* end while */
  
  deflist = m2c_ast_new_list_node(AST_DEFLIST, tmplist);
  m2c_fifo_release_queue(tmplist);
  
  /* END */
  if (match_token(p, TOKEN_END, FOLLOW(DEFINITION_MODULE))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* moduleIdent */
    if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(DEFINITION_MODULE))) {
      lookahead = m2c_consume_sym(p->lexer);
      ident2 = m2c_lexer_current_lexeme(p->lexer);
    
      if (ident1 != ident2) {
        /* TO DO: report error -- module identifiers don't match */ 
      } /* end if */
    
      /* '.' */
      if (match_token(p, TOKEN_PERIOD, FOLLOW(DEFINITION_MODULE))) {
        lookahead = m2c_consume_sym(p->lexer);
      } /* end if */
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  id = m2c_ast_new_terminal_node(AST_IDENT, ident1);
  p->ast = m2c_ast_new_node(AST_DEFMOD, id, implist, deflist, NULL);
  
  return lookahead;
} /* end definition_module */


/* --------------------------------------------------------------------------
 * private function import()
 * --------------------------------------------------------------------------
 * import :=
 *   ( qualifiedImport | unqualifiedImport ) ';'
 *   ;
 * ----------------------------------------------------------------------- */

m2c_token_t qualified_import (m2c_parser_context_t p);

m2c_token_t unqualified_import (m2c_parser_context_t p);

m2c_token_t import (m2c_parser_context_t p) {
  
  m2c_token_t lookahead = m2c_next_sym(p->lexer);
  
  PARSER_DEBUG_INFO("import");
  
  lookahead = m2c_next_sym(p->lexer);
  
  /* qualifiedImport */
  if (lookahead == TOKEN_IMPORT) {
    lookahead = qualified_import(p); /* p->ast holds ast-node */
  }
  /* | unqualifiedImport */
  else if (lookahead == TOKEN_FROM) {
    lookahead = unqualified_import(p); /* p->ast holds ast-node */
  }
  else /* unreachable code */ {
    /* fatal error -- abort */
  } /* end if */
  
  /* ';' */
  if (match_token(p, TOKEN_SEMICOLON, RESYNC(IMPORT_OR_DEFINITON_OR_END))) {
    lookahead = m2c_consume_sym(p->lexer);
  } /* end if */
  
  /* AST node is passed through in p->ast */
  
  return lookahead;
} /* end import */


/* --------------------------------------------------------------------------
 * private function qualified_import()
 * --------------------------------------------------------------------------
 * qualifiedImport :=
 *   IMPORT moduleList
 *   ;
 *
 * moduleList := identList ;
 *
 * astnode: (IMPORT identListNode)
 * ----------------------------------------------------------------------- */

m2c_token_t ident_list (m2c_parser_context_t p);

m2c_token_t qualified_import (m2c_parser_context_t p) {
  m2c_astnode_t idlist;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("qualifiedImport");
  
  /* IMPORT */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* moduleList */
  if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(QUALIFIED_IMPORT))) {
    lookahead = ident_list(p);
    idlist = p->ast;
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_IMPORT, idlist, NULL);
  
  return lookahead;
} /* end qualified_import */


/* --------------------------------------------------------------------------
 * private function unqualified_import()
 * --------------------------------------------------------------------------
 * unqualifiedImport :=
 *   FROM moduleIdent IMPORT identList
 *   ;
 *
 * moduleIdent := Ident ;
 *
 * astnode: (UNQIMP identNode identListNode)
 * ----------------------------------------------------------------------- */

m2c_token_t unqualified_import (m2c_parser_context_t p) {
  m2c_astnode_t id, idlist;
  m2c_string_t ident;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("unqualifiedImport");
  
  /* FROM */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* moduleIdent */
  if (match_token(p, TOKEN_IDENTIFIER,
      RESYNC(IMPORT_OR_IDENT_OR_SEMICOLON))) {
    lookahead = m2c_consume_sym(p->lexer);
    ident = m2c_lexer_current_lexeme(p->lexer);

    /* IMPORT */
    if (match_token(p, TOKEN_IMPORT, RESYNC(IDENT_OR_SEMICOLON))) {
      lookahead = m2c_consume_sym(p->lexer);
      
      /* moduleList */
      if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(UNQUALIFIED_IMPORT))) {
        lookahead = ident_list(p);
        idlist = p->ast;
      } /* end if */
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  id = m2c_ast_new_terminal_node(AST_IDENT, ident);
  p->ast = m2c_ast_new_node(AST_UNQIMP, id, idlist, NULL);
  
  return lookahead;
} /* end unqualified_import */


/* --------------------------------------------------------------------------
 * private function ident_list()
 * --------------------------------------------------------------------------
 * identList :=
 *   Ident ( ',' Ident )*
 *   ;
 *
 * astnode: (IDENTLIST ident0 ident1 ident2 ... identN)
 * ----------------------------------------------------------------------- */

m2c_token_t ident_list (m2c_parser_context_t p) {
  m2c_string_t ident;
  m2c_fifo_t tmplist;
  uint_t line, column;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("identList");
  
  /* Ident */
  ident = m2c_lexer_lookahead_lexeme(p->lexer);
  lookahead = m2c_consume_sym(p->lexer);
  
  /* add ident to temporary list */
  tmplist = m2c_fifo_new_queue(ident);
  
  /* ( ',' Ident )* */
  while (lookahead == TOKEN_COMMA) {
    /* ',' */
    lookahead = m2c_consume_sym(p->lexer);
    
    /* Ident */
    if (match_token(p, TOKEN_IDENTIFIER, RESYNC(COMMA_OR_SEMICOLON))) {
      lookahead = m2c_consume_sym(p->lexer);
      ident = m2c_lexer_current_lexeme(p->lexer);
      
      /* check for duplicate identifier */
      if (m2c_fifo_entry_exists(id_list, ident)) {
        line = m2c_lexer_current_line(p->lexer);
        column = m2c_lexer_current_column(p->lexer);
        report_error_w_offending_lexeme
          (M2C_ERROR_DUPLICATE_IDENT_IN_IDENT_LIST, p,
           m2c_lexer_current_lexeme(p->lexer), line, column);
      }
      else /* not a duplicate */ {
        /* add ident to temporary list */
        m2c_fifo_enqueue(tmplist, ident);
      } /* end if */
    } /* end if */
  } /* end while */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_IDENTLIST, tmplist);
  m2c_fifo_release_queue(tmplist);
    
  return lookahead;
} /* end ident_list */


/* --------------------------------------------------------------------------
 * private function definition()
 * --------------------------------------------------------------------------
 * definition :=
 *   CONST ( constDefinition ';' )* |
 *   TYPE ( typeDefinition ';' )* |
 *   VAR ( varDefinition ';' )* |
 *   procedureHeader ';'
 *   ;
 *
 * varDefinition := variableDeclaration ;
 * ----------------------------------------------------------------------- */

m2c_token_t const_definition (m2c_parser_context_t p);

m2c_token_t type_definition (m2c_parser_context_t p);

m2c_token_t variable_declaration (m2c_parser_context_t p);

m2c_token_t procedure_header (m2c_parser_context_t p);

m2c_token_t definition (m2c_parser_context_t p) {
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("definition");
  
  lookahead = m2c_next_sym(p->lexer);
  
  switch (lookahead) {
    
    /* CONST */
    case TOKEN_CONST :
      lookahead = m2c_consume_sym(p->lexer);
      
      /* ( constDefinition ';' )* */
      while (lookahead == TOKEN_IDENTIFIER) {
        lookahead = const_definition(p); /* p->ast holds ast-node */
        
        /* ';' */
        if (match_token(p, TOKEN_SEMICOLON,
            RESYNC(DEFINITION_OR_IDENT_OR_SEMICOLON))) {
          lookahead = m2c_consume_sym(p->lexer);
        } /* end if */
      } /* end while */
      break;
      
    /* | TYPE */
    case TOKEN_TYPE :
      lookahead = m2c_consume_sym(p->lexer);
      
      /* ( typeDefinition ';' )* */
      while (lookahead == TOKEN_IDENTIFIER) {
        lookahead = type_definition(p); /* p->ast holds ast-node */
        
        /* ';' */
        if (match_token(p, TOKEN_SEMICOLON,
            RESYNC(DEFINITION_OR_IDENT_OR_SEMICOLON))) {
          lookahead = m2c_consume_sym(p->lexer);
        } /* end if */
      } /* end while */
      break;
      
    /* | VAR */
    case TOKEN_VAR :
      lookahead = m2c_consume_sym(p->lexer);
      
      /* ( varDefinition ';' )* */
      while (lookahead == TOKEN_IDENTIFIER) {
        lookahead = variable_declaration(p); /* p->ast holds ast-node */
        
        /* ';' */
        if (match_token(p, TOKEN_SEMICOLON,
            RESYNC(DEFINITION_OR_IDENT_OR_SEMICOLON))) {
          lookahead = m2c_consume_sym(p->lexer);
        } /* end if */
      } /* end while */
      break;
      
    /* | procedureHeader */
    case TOKEN_PROCEDURE :
      lookahead = procedure_header(p); /* p->ast holds ast-node */
      
      /* ';' */
      if (match_token(p, TOKEN_SEMICOLON,
          RESYNC(DEFINITION_OR_SEMICOLON))) {
        lookahead = m2c_consume_sym(p->lexer);
      } /* end if */
      break;
      
    default : /* unreachable code */
      /* fatal error -- abort */
      exit(-1);
  } /* end switch */
  
  /* AST node is passed through in p->ast */
  
  return lookahead;
} /* end definition */


/* --------------------------------------------------------------------------
 * private function const_definition()
 * --------------------------------------------------------------------------
 * constDefinition :=
 *   Ident '=' constExpression
 *   ;
 *
 * astnode: (CONSTDEF identNode exprNode)
 * ----------------------------------------------------------------------- */

m2c_token_t const_expression (m2c_parser_context_t p);

m2c_token_t const_definition (m2c_parser_context_t p) {
  m2c_astnode_t id, expr;
  m2c_string_t ident;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("constDefinition");
  
  /* Ident */
  lookahead = m2c_consume_sym(p->lexer);
  ident = m2c_lexer_current_lexeme(p->lexer);
  
  /* '=' */
  if (match_token(p, TOKEN_EQUAL, FOLLOW(CONST_DEFINITION))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* constExpression */
    if (match_set(p, FIRST(EXPRESSION), FOLLOW(CONST_DEFINITION))) {
      lookahead = const_expression(p);
      expr = p->ast;
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  id = m2c_ast_new_terminal_node(AST_IDENT, ident);
  p->ast = m2c_ast_new_node(AST_CONSTDEF, id, expr, NULL);
  
  return lookahead;
} /* end const_definition */


/* --------------------------------------------------------------------------
 * private function type_definition()
 * --------------------------------------------------------------------------
 * typeDefinition :=
 *   Ident ( '=' type )?
 *   ;
 *
 * astnode: (TYPEDEF identNode typeConstructorNode)
 * ----------------------------------------------------------------------- */

m2c_token_t type (m2c_parser_context_t p);

m2c_token_t type_definition (m2c_parser_context_t p) {
  m2c_astnode_t id, tc;
  m2c_string_t ident;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("typeDefinition");
  
  /* Ident */
  lookahead = m2c_consume_sym(p->lexer);
  ident = m2c_lexer_current_lexeme(p->lexer);
  
  /* ( '=' type )? */
  if (lookahead == TOKEN_EQUAL) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* type */
    if (match_set(p, FIRST(TYPE), FOLLOW(TYPE_DEFINITION))) {
      lookahead = type(p);
      tc = p->ast;
    } /* end if */
  }
  else {
    tc = m2c_ast_empty_node();
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  id = m2c_ast_new_terminal_node(AST_IDENT, ident);
  p->ast = m2c_ast_new_node(AST_TYPEDEF, id, tc, NULL);
  
  return lookahead;
} /* end type_definition */


/* --------------------------------------------------------------------------
 * private function type()
 * --------------------------------------------------------------------------
 * type :=
 *   derivedOrSubrangeType | enumType | setType | arrayType |
 *   recordType | pointerType | procedureType
 *   ;
 * ----------------------------------------------------------------------- */

m2c_token_t derived_or_subrange_type (m2c_parser_context_t p);

m2c_token_t enum_type (m2c_parser_context_t p);

m2c_token_t set_type (m2c_parser_context_t p);

m2c_token_t array_type (m2c_parser_context_t p);

m2c_token_t pointer_type (m2c_parser_context_t p);

m2c_token_t procedure_type (m2c_parser_context_t p);

m2c_token_t type (m2c_parser_context_t p) {
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("type");
  
  lookahead = m2c_next_sym(p->lexer);
  
  switch (lookahead) {
  
    /* derivedOrSubrangeType */
    case TOKEN_IDENTIFIER :
    case TOKEN_LEFT_BRACKET :
      lookahead = derived_or_subrange_type(p); /* p->ast holds ast-node */
      break;
      
    /* | enumType */
    case TOKEN_LEFT_PAREN :
      lookahead = enum_type(p); /* p->ast holds ast-node */
      break;
      
    /* | setType */
    case TOKEN_SET :
      lookahead = set_type(p); /* p->ast holds ast-node */
      break;
      
    /* | arrayType */
    case TOKEN_ARRAY :
      lookahead = array_type(p); /* p->ast holds ast-node */
      break;
      
    /* | recordType */
    case TOKEN_RECORD :
      lookahead = p->record_type(p); /* p->ast holds ast-node */
      break;
      
    /* | pointerType */
    case TOKEN_POINTER :
      lookahead = pointer_type(p); /* p->ast holds ast-node */
      break;
      
    /* | procedureType */
    case TOKEN_PROCEDURE :
      lookahead = procedure_type(p); /* p->ast holds ast-node */
      break;
      
    default : /* unreachable code */
      /* fatal error -- abort */
      exit(-1);
    } /* end switch */
  
  /* AST node is passed through in p->ast */
  
  return lookahead;
} /* end type */


/* --------------------------------------------------------------------------
 * private function derived_or_subrange_type()
 * --------------------------------------------------------------------------
 * derivedOrSubrangeType :=
 *   typeIdent range? | range
 *   ;
 *
 * typeIdent := qualident ;
 * ----------------------------------------------------------------------- */

m2c_token_t qualident (m2c_parser_context_t p);

m2c_token_t range (m2c_parser_context_t p);

m2c_token_t derived_or_subrange_type (m2c_parser_context_t p) {
  m2c_astnode_t id;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("derivedOrSubrangeType");
  
  lookahead = m2c_next_sym(p->lexer);
  
  if (match_set(p, FIRST(DERIVED_OR_SUBRANGE_TYPE),
      FOLLOW(DERIVED_OR_SUBRANGE_TYPE))) {
    
    /* typeIdent range? */
    if (lookahead == TOKEN_IDENTIFIER) {
      
      /* typeIdent */
      lookahead = qualident(p);
      /* astnode: (IDENT ident) | (QUALIDENT q0 q1 q2 ... qN ident) */
      id = p->ast;
      
      /* range? */
      if (lookahead == TOKEN_LEFT_BRACKET) {
        lookahead = range(p);
        m2c_ast_replace_subnode(p->ast, 2, id);
        /* astnode: (SUBR lower upper identNode) */
      } /* end if */
    }
    /* | range */
    else if (lookahead == TOKEN_LEFT_BRACKET) {
      lookahead = range(p);
      /* astnode: (SUBR lower upper (EMPTY)) */
    }
    else /* unreachable code */ {
      /* fatal error -- abort */
      exit(-1);
    } /* end if */
  } /* end if */
  
  /* AST node is passed through in p->ast */
  
  return lookahead;
} /* end derived_or_subrange_type */


/* --------------------------------------------------------------------------
 * private function qualident()
 * --------------------------------------------------------------------------
 * qualident :=
 *   Ident ( '.' Ident )*
 *   ;
 *
 * astnode: (IDENT ident) | (QUALIDENT q0 q1 q2 ... qN ident)
 * ----------------------------------------------------------------------- */

m2c_token_t qualident (m2c_parser_context_t p) {
  m2c_string_t ident, qident;
  m2c_fifo_t tmplist;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("qualident");
  
  /* Ident */
  lookahead = m2c_consume_sym(p->lexer);
  ident = m2c_lexer_current_lexeme(p->lexer);
  
  /* add ident to temporary list */
  tmplist = m2c_fifo_new_queue(ident);
  
  /* ( '.' Ident )* */
  while (lookahead == TOKEN_PERIOD) {
    /* '.' */
    lookahead = m2c_consume_sym(p->lexer);
    
    /* Ident */
    if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(QUALIDENT))) {
      lookahead = m2c_consume_sym(p->lexer);
      qident = m2c_lexer_current_lexeme(p->lexer);
      m2c_fifo_enqueue(tmplist, qident);
    } /* end if */
  } /* end while */
  
  /* build AST node and pass it back in p->ast */
  if (m2c_fifo_entry_count(tmplist) == 1) {
    p->ast = m2c_ast_new_terminal_node(AST_IDENT, ident, NULL);
  }
  else {
    p->ast = m2c_ast_new_terminal_list_node(AST_QUALIDENT, tmplist);
  } /* end if */
  
  m2c_fifo_release(tmplist);
  
  return lookahead;
} /* end qualident */


/* --------------------------------------------------------------------------
 * private function range()
 * --------------------------------------------------------------------------
 * range :=
 *   '[' constExpression '..' constExpression ']'
 *   ;
 *
 * astnode: (SUBR exprNode exprNode (EMPTY))
 * ----------------------------------------------------------------------- */

m2c_token_t range (m2c_parser_context_t p) {
  m2c_astnode_t lower, upper, empty;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("range");
  
  /* '[' */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* constExpression */
  if (match_set(p, FIRST(EXPRESSION), FOLLOW(RANGE))) {
    lookahead = const_expression(p);
    lower = p->ast;
    
    /* '..' */
    if (match_token(p, TOKEN_RANGE, FOLLOW(RANGE))) {
      lookahead = m2c_consume_sym(p->lexer);
      
      /* constExpression */
      if (match_set(p, FIRST(EXPRESSION), FOLLOW(RANGE))) {
        lookahead = const_expression(p);
        upper = p->ast;
        
        /* ']' */
        if (match_token(p, TOKEN_RIGHT_BRACKET, FOLLOW(RANGE))) {
          lookahead = m2c_consume_sym(p->lexer);
        } /* end if */
      } /* end if */
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  empty = m2c_ast_empty_node();
  p->ast = m2c_ast_new_node(AST_SUBR, lower, upper, empty, NULL);
  
  return lookahead;
} /* end range */


/* --------------------------------------------------------------------------
 * private function enum_type()
 * --------------------------------------------------------------------------
 * enumType :=
 *   '(' identList ')'
 *   ;
 *
 * astnode: (ENUM identListNode)
 * ----------------------------------------------------------------------- */

m2c_token_t enum_type (m2c_parser_context_t p) {
  m2c_astnode idlist;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("enumType");
  
  /* '(' */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* identList */
  if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(ENUM_TYPE))) {
    lookahead = ident_list(p);
    idlist = p->ast;
    
    /* ')' */
    if (match_token(p, TOKEN_RIGHT_PAREN, FOLLOW(ENUM_TYPE))) {
      lookahead = m2c_consume_sym(p->lexer);
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_ENUM, idlist, NULL);
  
  return lookahead;
} /* end enum_type */


/* --------------------------------------------------------------------------
 * private function set_type()
 * --------------------------------------------------------------------------
 * setType :=
 *   SET OF countableType
 *   ;
 *
 * astnode: (SET typeConstructorNode)
 * ----------------------------------------------------------------------- */

m2c_token_t countable_type (m2c_parser_context_t p);

m2c_token_t set_type (m2c_parser_context_t p) {
  m2c_astnode_t tc;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("setType");
  
  /* SET */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* OF */
  if (match_token(p, TOKEN_OF, FOLLOW(SET_TYPE))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* countableType */
    if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(SET_TYPE))) {
      lookahead = countable_type(p);
      tc = p->ast;
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_SET, tc, NULL);
  
  return lookahead;
} /* end set_type */


/* --------------------------------------------------------------------------
 * private function countable_type()
 * --------------------------------------------------------------------------
 * countableType :=
 *   range | enumType | countableTypeIdent range?
 *   ;
 *
 * countableTypeIdent := typeIdent ;
 *
 * astnode:
 *  (IDENT ident) | (QUALIDENT q0 q1 q2 ... qN ident) |
 *  (SUBR expr expr (EMPTY)) | (SUBR expr expr identNode) |
 *  (ENUM identListNode)
 * ----------------------------------------------------------------------- */

m2c_token_t countable_type (m2c_parser_context_t p) {
  m2c_string_t ident;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("countableType");
  
  lookahead = m2c_next_sym(p->lexer);
  
  switch (lookahead) {
  
    /* range */
    case TOKEN_LEFT_BRACKET :
      lookahead = range(p);
      (* astnode: (SUBR expr expr (EMPTY)) *)
      break;
      
    /* | enumType */
    case TOKEN_LEFT_PAREN :
      lookahead = enum_type(p);
      (* astnode: (ENUM identListNode) *)
      break;
      
    /* | countableTypeIdent range? */
    case TOKEN_IDENTIFIER :
      lookahead = qualident(p);
      /* astnode: (IDENT ident) | (QUALIDENT q0 q1 q2 ... qN ident) */
      id = p->ast;
      
      /* range? */
      if (lookahead == TOKEN_LEFT_BRACKET) {
        lookahead = range(p);
        m2c_ast_replace_subnode(p->ast, 2, id);
      /* astnode: (SUBR expr expr identNode) */
      } /* end if */
      break;
      
    default : /* unreachable code */
      /* fatal error -- abort */
      exit(-1);
    } /* end switch */
  
  /* AST node is passed through in p->ast */
  
  return lookahead;
} /* end countable_type */


/* --------------------------------------------------------------------------
 * private function array_type()
 * --------------------------------------------------------------------------
 * arrayType :=
 *   ARRAY countableType ( ',' countableType )* OF type
 *   ;
 *
 * astnode: (ARRAY indexTypeListNode arrayBaseTypeNode)
 * ----------------------------------------------------------------------- */

m2c_token_t array_type (m2c_parser_context_t p) {
  m2c_astnode_t idxlist, basetype;
  m2c_fifo_t tmplist;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("arrayType");
  
  /* ARRAY */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* countableType */
  if (match_set(p, FIRST(COUNTABLE_TYPE), FOLLOW(ARRAY_TYPE))) {
    lookahead = countable_type(p);
    tmplist = m2c_fifo_new_queue(p->ast);
    
    /* ( ',' countableType )* */
    while (lookahead == TOKEN_COMMA) {
      /* ',' */
      lookahead = m2c_consume_sym(p->lexer);
      
      if (match_set(p, FIRST(COUNTABLE_TYPE), RESYNC(TYPE_OR_COMMA_OR_OF))) {
        lookahead = countable_type(p);
        m2c_fifo_enqueue(tmplist, p->ast);
      }
      else /* resync */ {
        lookahead = m2c_next_sym(p->lexer);
      } /* end if */
    } /* end while */

    /* OF */
    if (match_token(p, TOKEN_OF, FOLLOW(ARRAY_TYPE))) {
      lookahead = m2c_consume_sym(p->lexer);
  
      /* type */
      if (match_set(p, FIRST(TYPE), FOLLOW(ARRAY_TYPE))) {
        lookahead = type(p);
        basetype = p->ast;
      } /* end if */
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  idxlist = m2c_ast_new_list_node(AST_INDEXLIST, tmplist);
  p->ast = m2c_ast_new_node(AST_ARRAY, idxlist, basetype, NULL);
  m2c_fifo_release_queue(tmplist);
  
  return lookahead;
} /* end array_type */


/* --------------------------------------------------------------------------
 * private function extensible_record_type()
 * --------------------------------------------------------------------------
 * For use with compiler option --no-variant-records.
 *
 * recordType := extensibleRecordType ;
 *
 * extensibleRecordType :=
 *   RECORD ( '(' baseType ')' )? fieldListSequence END
 *   ;
 *
 * baseType := typeIdent ;
 *
 * astnode:
 *  (RECORD fieldListSeqNode) | (EXTREC baseTypeNode fieldListSeqNode)
 * ----------------------------------------------------------------------- */

m2c_token_t field_list_sequence (m2c_parser_context_t p);

m2c_token_t extensible_record_type (m2c_parser_context_t p) {
  m2c_astnode_t basetype, flseq;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("recordType");
  
  /* RECORD */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* ( '(' baseType ')' )? */
  if (lookahead == TOKEN_LEFT_PAREN) {
    /* '(' */
    lookahead = m2c_consume_sym(p->lexer);
    
    /* baseType */
    if (match_token(p, TOKEN_IDENTIFIER,
        FIRST(FIELD_LIST_SEQUENCE))) {
      lookahead = qualident(p);
      basetype = p->ast;
      
      /* ')' */
      if (match_token(p, TOKEN_RIGHT_PAREN,
          FIRST(FIELD_LIST_SEQUENCE))) {
        lookahead = m2c_consume_sym(p->lexer);
      }
      else /* resync */ {
        lookahead = m2c_next_sym(p->lexer);
      } /* end if */
    }
    else  /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  }
  else {
    basetype = NULL;
  } /* end if */
  
  /* check for empty field list sequence */
  if (lookahead == TOKEN_END) {

      /* empty field list sequence warning */
      m2c_emit_warning_w_pos
        (M2C_EMPTY_FIELD_LIST_SEQ,
         m2c_lexer_lookahead_line(p->lexer),
         m2c_lexer_lookahead_column(p->lexer));
      p->warning_count++;
      
      /* END */
      lookahead = m2c_consume_sym(p->lexer);
  }
  
  /* fieldListSequence */
  else if (match_set(p, FIRST(FIELD_LIST_SEQUENCE),
           FOLLOW(EXTENSIBLE_RECORD_TYPE))) {
    lookahead = field_list_sequence(p);
    flseq = p->ast;
    
    /* END */
    if (match_token(p, TOKEN_END, FOLLOW(EXTENSIBLE_RECORD_TYPE))) {
      lookahead = m2c_consume_sym(p->lexer);
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  if (basetype == NULL) {
    p->ast = m2c_ast_new_node(AST_RECORD, flseq, NULL);
  }
  else {
    p->ast = m2c_ast_new_node(AST_EXTREC, basetype, flseq, NULL);
  } /* end if */
  
  return lookahead;
} /* end extensible_record_type */


/* --------------------------------------------------------------------------
 * private function field_list_sequence()
 * --------------------------------------------------------------------------
 * For use with compiler option --no-variant-records.
 *
 * fieldListSequence :=
 *   fieldList ( ';' fieldList )*
 *   ;
 *
 * astnode: (FIELDLISTSEQ fieldListNode+)
 * ----------------------------------------------------------------------- */

m2c_token_t field_list (m2c_parser_context_t p);

m2c_token_t field_list_sequence (m2c_parser_context_t p) {
  m2c_fifo_t tmplist;
  m2c_token_t lookahead;
  uint_t line_of_semicolon, column_of_semicolon;
  
  PARSER_DEBUG_INFO("fieldListSequence");
  
  /* fieldList */
  lookahead = field_list(p);
  tmplist = m2c_fifo_new_queue(p->ast);
  
  /* ( ';' fieldList )* */
  while (lookahead == TOKEN_SEMICOLON) {
    /* ';' */
    line_of_semicolon = m2c_lexer_lookahead_line(p->lexer);
    column_of_semicolon = m2c_lexer_lookahead_column(p->lexer);
    lookahead = m2c_consume_sym(p->lexer);
    
    /* check if semicolon occurred at the end of a field list sequence */
    if (m2c_tokenset_element(FOLLOW(FIELD_LIST_SEQUENCE), lookahead)) {
    
      if (m2c_option_errant_semicolon()) {
        /* treat as warning */
        m2c_emit_warning_w_pos
          (M2C_SEMICOLON_AFTER_FIELD_LIST_SEQ,
           line_of_semicolon, column_of_semicolon);
        p->warning_count++;
      }
      else /* treat as error */ {
        m2c_emit_error_w_pos
          (M2C_SEMICOLON_AFTER_FIELD_LIST_SEQ,
           line_of_semicolon, column_of_semicolon);
        p->error_count++;
      } /* end if */
      
      /* print source line */
      if (m2c_option_verbose()) {
        m2c_print_line_and_mark_column(p->lexer,
          line_of_semicolon, column_of_semicolon);
      } /* end if */
    
      /* leave field list sequence loop to continue */
      break;
    } /* end if */
    
    /* fieldList */
    if (match_set(p, FIRST(VARIABLE_DECLARATION),
        RESYNC(SEMICOLON_OR_END))) {
      lookahead = field_list(p);
      m2c_fifo_enqueue(tmplist, p->ast);
    } /* end if */
  } /* end while */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_list_node(AST_FIELDLISTSEQ, tmplist);
  m2c_fifo_release(tmplist);
  
  return lookahead;
} /* end field_list_sequence */


/* --------------------------------------------------------------------------
 * private function field_list()
 * --------------------------------------------------------------------------
 * fieldList :=
 *   identList ':' type
 *   ;
 *
 * astnode: (FIELDLIST identListNode typeConstructorNode)
 * ----------------------------------------------------------------------- */

/* TO DO: add discrete first and follow set for fieldList */

m2c_token_t field_list (m2c_parser_context_t p) {
  m2c_astnode_t idlist, tc;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("fieldList");
  
  /* IdentList */
  lookahead = ident_list(p);
  idlist = p->ast;
  
  /* ':' */
  if (match_token(p, TOKEN_COLON, FOLLOW(VARIABLE_DECLARATION))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* type */
    if (match_set(p, FIRST(TYPE), FOLLOW(VARIABLE_DECLARATION))) {
      lookahead = type(p);
      tc = p->ast;
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_FIELDLIST, idlist, tc, NULL);
  
  return lookahead;
} /* end field_list */


/* --------------------------------------------------------------------------
 * private function variant_record_type()
 * --------------------------------------------------------------------------
 * For use with compiler option --variant-records.
 *
 * recordType := variantRecordType ;
 *
 * variantRecordType :=
 *   RECORD variantFieldListSeq END
 *   ;
 *
 * astnode: (RECORD fieldListSeqNode) | (VRNTREC variantFieldListSeqNode)
 * ----------------------------------------------------------------------- */

m2c_token_t variant_field_list_seq (m2c_parser_context_t p);

m2c_token_t variant_record_type (m2c_parser_context_t p) {
  m2c_astnode_t flseq;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("recordType");
  
  /* RECORD */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* check for empty field list sequence */
  if (lookahead == TOKEN_END) {

      /* empty field list sequence warning */
      m2c_emit_warning_w_pos
        (M2C_EMPTY_FIELD_LIST_SEQ,
         m2c_lexer_lookahead_line(p->lexer),
         m2c_lexer_lookahead_column(p->lexer));
      p->warning_count++;
      
      /* END */
      lookahead = m2c_consume_sym(p->lexer);
  }
  /* variantFieldListSeq */
  else if(match_set(p, FIRST(VARIANT_FIELD_LIST_SEQ),
          FOLLOW(VARIANT_RECORD_TYPE))) {
    lookahead = variant_field_list_seq(p);
    flseq = p->ast;
    
    /* END */
    if (match_token(p, TOKEN_END, FOLLOW(VARIANT_RECORD_TYPE))) {
      lookahead = m2c_consume_sym(p->lexer);
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  if (m2c_ast_nodetype(flseq) == AST_VFLISTSEQ) {
    p->ast = m2c_ast_new_node(AST_VRNTREC, flseq, NULL);
  }
  else /* not variant field list sequence */ {
    p->ast = m2c_ast_new_node(AST_RECORD, flseq, NULL);
  } /* end if */
  
  return lookahead;
} /* end variant_record_type */


/* --------------------------------------------------------------------------
 * private function variant_field_list_seq()
 * --------------------------------------------------------------------------
 * For use with compiler option --variant-records.
 *
 * variantFieldListSeq :=
 *   variantFieldList ( ';' variantFieldList )*
 *   ;
 *
 * astnode:
 *  (FIELDLISTSEQ fieldListNode+) | (VFLISTSEQ anyFieldListNode+)
 * ----------------------------------------------------------------------- */

m2c_token_t variant_field_list (m2c_parser_context_t p);

m2c_token_t variant_field_list_seq (m2c_parser_context_t p) {
  m2c_fifo_t tmplist;
  m2c_token_t lookahead;
  uint_t line_of_semicolon, column_of_semicolon;
  bool variant_fieldlist_found = false;
  
  PARSER_DEBUG_INFO("variantFieldListSeq");
  
  /* variantFieldList */
  lookahead = variant_field_list(p);
  tmplist = m2c_fifo_new_queue(p->ast);
  
  if (m2c_ast_nodetype(p->ast) == AST_VFLIST) {
    variant_fieldlist_found = true;
  } /* end if */
  
  /* ( ';' variantFieldList )* */
  while (lookahead == TOKEN_SEMICOLON) {
    /* ';' */
    line_of_semicolon = m2c_lexer_lookahead_line(p->lexer);
    column_of_semicolon = m2c_lexer_lookahead_column(p->lexer);
    lookahead = m2c_consume_sym(p->lexer);
    
    /* check if semicolon occurred at the end of a field list sequence */
    if (m2c_tokenset_element(FOLLOW(VARIANT_FIELD_LIST_SEQ), lookahead)) {
    
      if (m2c_option_errant_semicolon()) {
        /* treat as warning */
        m2c_emit_warning_w_pos
          (M2C_SEMICOLON_AFTER_FIELD_LIST_SEQ,
           line_of_semicolon, column_of_semicolon);
        p->warning_count++;
      }
      else /* treat as error */ {
        m2c_emit_error_w_pos
          (M2C_SEMICOLON_AFTER_FIELD_LIST_SEQ,
           line_of_semicolon, column_of_semicolon);
        p->error_count++;
      } /* end if */
      
      /* print source line */
      if (m2c_option_verbose()) {
        m2c_print_line_and_mark_column(p->lexer,
          line_of_semicolon, column_of_semicolon);
      } /* end if */
    
      /* leave field list sequence loop to continue */
      break;
    } /* end if */
    
    /* variantFieldList */
    if (match_set(p, FIRST(VARIANT_FIELD_LIST),
        FOLLOW(VARIANT_FIELD_LIST))) {
      lookahead = variant_field_list(p);
      m2c_fifo_enqueue(tmplist, p->ast);
      
      if (m2c_ast_nodetype(p->ast) == AST_VFLIST) {
        variant_fieldlist_found = true;
      } /* end if */
    } /* end if */
  } /* end while */
  
  /* build AST node and pass it back in p->ast */
  if (variant_fieldlist_found) {
    p->ast = m2c_ast_new_list_node(AST_VFLISTSEQ, tmplist);
  }
  else /* not variant field list */ {
    p->ast = m2c_ast_new_list_node(AST_FIELDLISTSEQ, tmplist);
  } /* end if */
  
  m2c_fifo_release(tmplist);
  
  return lookahead;
} /* end variant_field_list_seq */


/* --------------------------------------------------------------------------
 * private function variant_field_list()
 * --------------------------------------------------------------------------
 * For use with compiler option --variant-records.
 *
 * variantFieldList :=
 *   fieldList | variantFields
 *   ;
 * ----------------------------------------------------------------------- */

m2c_token_t variant_fields (m2c_parser_context_t p);

m2c_token_t variant_field_list (m2c_parser_context_t p) {
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("variantFieldList");
  
  lookahead = m2c_next_sym(p->lexer);
  
  /* fieldList */
  if (lookahead == TOKEN_IDENTIFIER) {
    lookahead = field_list(p);
  }
  /* | variantFields */
  else if (lookahead == TOKEN_CASE) {
    lookahead = variant_fields(p);
  }
  else /* unreachable code */ {
    /* fatal error -- abort */
      exit(-1);
  } /* end if */
  
  /* AST node is passed through in p->ast */
  
  return lookahead;
} /* end variant_field_list */


/* --------------------------------------------------------------------------
 * private function variant_fields()
 * --------------------------------------------------------------------------
 * For use with compiler option --variant-records.
 *
 * variantFields :=
 *   CASE Ident? ':' typeIdent OF
 *   variant ( '|' variant )*
 *   ( ELSE fieldListSequence )?
 *   END
 *   ;
 *
 * astnode:
 *  (VFLIST caseIdentNode caseTypeNode variantListNode fieldListSeqNode)
 * ----------------------------------------------------------------------- */

m2c_token_t variant (m2c_parser_context_t p);

m2c_token_t variant_fields (m2c_parser_context_t p) {
  m2c_astnode_t caseid, typeid, vlist, flseq;
  m2c_fifo_t tmplist;
  m2c_string_t ident;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("variantFields");
  
  /* CASE */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* Ident? */
  if (lookahead == TOKEN_IDENTIFIER) {
    lookahead = m2c_consume_sym(p->lexer);
    ident = m2c_lexer_current_lexeme(p->lexer);
    caseid = m2c_ast_new_terminal_node(AST_IDENT, ident);
  }
  else {
    caseid = m2c_ast_empty_node();
  } /* end if */
  
  /* ':' */
  if (match_token(p, TOKEN_COLON, RESYNC(ELSE_OR_END))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* typeIdent */
    if (match_token(p, TOKEN_IDENTIFIER, RESYNC(ELSE_OR_END))) {
      lookahead = m2c_consume_sym(p->lexer);
      ident = m2c_lexer_current_lexeme(p->lexer);
      typeid = m2c_ast_new_terminal_node(AST_IDENT, ident);
    
      /* OF */
      if (match_token(p, TOKEN_OF, RESYNC(ELSE_OR_END))) {
        lookahead = m2c_consume_sym(p->lexer);
      
        /* variant */
        if (match_set(p, FIRST(VARIANT), RESYNC(ELSE_OR_END))) {
          lookahead = variant(p);
          tmplist = m2c_fifo_new_queue(p->ast);
        
          /* ( '|' variant )* */
          while (lookahead == TOKEN_BAR) {
          
            /* '|' */
            lookahead = m2c_consume_sym(p->lexer);
          
            /* variant */
            if (match_set(p, FIRST(VARIANT), RESYNC(ELSE_OR_END))) {
              lookahead = variant(p);
              m2c_fifo_enqueue(tmplist, p->ast);
            } /* end if */
          } /* end while */
        } /* end if */
      } /* end if */
    } /* end if */
  } /* end if */
  
  /* resync */
  lookahead = m2c_next_sym(p->lexer);
    
  /* ( ELSE fieldListSequence )? */
  if (lookahead == TOKEN_ELSE) {
  
    /* ELSE */
    lookahead = m2c_consume_sym(p->lexer);
  
    /* check for empty field list sequence */
    if (lookahead == TOKEN_END) {

        /* empty field list sequence warning */
        m2c_emit_warning_w_pos
          (M2C_EMPTY_FIELD_LIST_SEQ,
           m2c_lexer_lookahead_line(p->lexer),
           m2c_lexer_lookahead_column(p->lexer));
        p->warning_count++;
    }
    /* fieldListSequence */
    else if (match_set(p,
             FIRST(FIELD_LIST_SEQUENCE), FOLLOW(VARIANT_FIELDS))) {
      lookahead = field_list_sequence(p);
      flseq = p->ast;
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  }
  else {
    flseq = m2c_ast_empty_node();
  } /* end if */
  
  /* END */
  if (match_token(p, TOKEN_END, FOLLOW(VARIANT_FIELDS))) {
    lookahead = m2c_consume_sym(p->lexer);
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  vlist = m2c_ast_new_list_node(AST_VARIANTLIST, tmplist);
  p->ast = m2c_ast_new_node(AST_VFLIST, caseid, typeid, vlist, flseq, NULL);
  m2c_fifo_release(tmplist);
  
  return lookahead;
} /* end variant_fields */


/* --------------------------------------------------------------------------
 * private function variant()
 * --------------------------------------------------------------------------
 * For use with compiler option --variant-records.
 *
 * variant :=
 *   caseLabelList ':' variantFieldListSeq
 *   ;
 *
 * astnode: (VARIANT caseLabelListNode fieldListSeqNode)
 * ----------------------------------------------------------------------- */

m2c_token_t case_label_list (m2c_parser_context_t p);

m2c_token_t variant (m2c_parser_context_t p) {
  m2c_astnode_t cllist, flseq;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("variant");
  
  /* caseLabelList */
  lookahead = case_label_list(p);
  cllist = p->ast;
  
  /* ':' */
  if (match_token(p, TOKEN_COLON, FOLLOW(VARIANT))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* check for empty field list sequence */
    if (m2c_tokenset_element(FOLLOW(VARIANT), lookahead)) {

        /* empty field list sequence warning */
        m2c_emit_warning_w_pos
          (M2C_EMPTY_FIELD_LIST_SEQ,
           m2c_lexer_lookahead_line(p->lexer),
           m2c_lexer_lookahead_column(p->lexer));
           p->warning_count++;
    }
    /* variantFieldListSeq */
    else if (match_set(p, FIRST(VARIANT_FIELD_LIST_SEQ), FOLLOW(VARIANT))) {
      lookahead = variant_field_list_seq(p);
      flseq = p->ast;
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_VARIANT, cllist, flseq, NULL);
  
  return lookahead;
} /* end variant */


/* --------------------------------------------------------------------------
 * private function case_label_list()
 * --------------------------------------------------------------------------
 * caseLabelList :=
 *   caseLabels ( ',' caseLabels )*
 *   ;
 *
 * astnode : (CLABELLIST caseLabelsNode+)
 * ----------------------------------------------------------------------- */

m2c_token_t case_labels (m2c_parser_context_t p);

m2c_token_t case_label_list (m2c_parser_context_t p) {
  m2c_fifo_t tmplist;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("caseLabelList");
  
  /* caseLabels */
  lookahead = case_labels(p);
  tmplist = m2c_fifo_new_queue(p->ast);
  
  /* ( ',' caseLabels )* */
  while (lookahead == TOKEN_COMMA) {
    /* ',' */
    lookahead = m2c_consume_sym(p->lexer);
    
    /* caseLabels */
    if (match_set(p, FIRST(CASE_LABELS), FOLLOW(CASE_LABEL_LIST))) {
      lookahead = case_labels(p);
      m2c_fifo_enqueue(tmplist, p->ast);
    } /* end if */
  } /* end while */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_list_node(AST_CLABELLIST, tmplist);
  m2c_fifo_release(tmplist);
  
  return lookahead;
} /* end case_label_list */


/* --------------------------------------------------------------------------
 * private function case_labels()
 * --------------------------------------------------------------------------
 * caseLabels :=
 *   constExpression ( '..' constExpression )?
 *   ;
 *
 * astnode: (CLABELS exprNode exprNode)
 * ----------------------------------------------------------------------- */

m2c_token_t case_labels (m2c_parser_context_t p) {
  m2c_astnode_t lower, upper;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("caseLabels");
  
  /* constExpression */
  lookahead = const_expression(p);
  lower = p->ast;
  
  /* ( '..' constExpression )? */
  if (lookahead == TOKEN_RANGE) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* constExpression */
    if (match_set(p, FIRST(EXPRESSION), FOLLOW(CASE_LABELS))) {
      lookahead = const_expression(p);
      upper = p->ast;
    } /* end if */
  }
  else {
    upper = m2c_ast_empty_node();
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  m2c_ast_new_node(AST_CLABELS, lower, upper, NULL);
  
  return lookahead;
} /* end case_labels */


/* --------------------------------------------------------------------------
 * private function pointer_type()
 * --------------------------------------------------------------------------
 * pointerType :=
 *   POINTER TO type
 *   ;
 *
 * astnode: (POINTER typeConstructorNode)
 * ----------------------------------------------------------------------- */

m2c_token_t pointer_type (m2c_parser_context_t p) {
  m2c_astnode_t tc;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("pointerType");
  
  /* POINTER */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* TO */
  if (match_token(p, TOKEN_TO, FOLLOW(POINTER_TYPE))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* type */
    if (match_set(p, FIRST(TYPE), FOLLOW(POINTER_TYPE))) {
      lookahead = type(p);
      tc = p->ast;
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_POINTER, tc, NULL);
  
  return lookahead;
} /* end pointer_type */


/* --------------------------------------------------------------------------
 * private function procedure_type()
 * --------------------------------------------------------------------------
 * procedureType :=
 *   PROCEDURE ( '(' ( formalType ( ',' formalType )* )? ')' )?
 *   ( ':' returnedType )?
 *   ;
 *
 * returnedType := typeIdent ;
 *
 * astnode: (PROCTYPE formalTypeListNode returnedTypeNode)
 * ----------------------------------------------------------------------- */

m2c_token_t formal_type (m2c_parser_context_t p);

m2c_token_t procedure_type (m2c_parser_context_t p) {
  m2c_astnode_t ftlist, rtype;
  m2c_fifo_t tmplist;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("procedureType");
  
  /* PROCEDURE */
  lookahead = m2c_consume_sym(p->lexer);
  
  tmplist = m2c_fifo_new_queue(NULL);
  
  /* ( '(' ( formalType ( ',' formalType )* )? ')' )? */
  if (lookahead == TOKEN_LEFT_PAREN) {
    /* '(' */
    lookahead = m2c_consume_sym(p->lexer);
    
    /* formalType */
    if (lookahead != TOKEN_RIGHT_PAREN) {
      if (match_set(p, FIRST(FORMAL_TYPE), RESYNC(COMMA_OR_RIGHT_PAREN))) {
        lookahead = formal_type(p);
        m2c_fifo_enqueue(tmplist, p->ast);
      }
      else /* resync */ {
        lookahead = m2c_next_sym(p->lexer);
      } /* end if */
      
      /* ( ',' formalType )* */
      while (lookahead == TOKEN_COMMA) {
        /* ',' */
        lookahead = m2c_consume_sym(p->lexer);
      
        /* formalType */
        if (match_set(p, FIRST(FORMAL_TYPE), RESYNC(COMMA_OR_RIGHT_PAREN))) {
          lookahead = formal_type(p);
          m2c_fifo_enqueue(tmplist, p->ast);
        }
        else /* resync */ {
          lookahead = m2c_next_sym(p->lexer);
        } /* end if */
      } /* end while */
    } /* end if */
    
    /* ')' */
    if (match_token(p, TOKEN_RIGHT_PAREN, RESYNC(COLON_OR_SEMICOLON))) {
      lookahead = m2c_consume_sym(p->lexer);
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  } /* end if */
  
  /* ( ':' returnedType )? */
  if (lookahead == TOKEN_COLON) {
    /* ':' */
    lookahead = m2c_consume_sym(p->lexer);
    
    /* returnedType */
    if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(PROCEDURE_TYPE))) {
      lookahead = qualident(p);
      rtype = p->ast;
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  }
  else {
    rtype = m2c_ast_empty_node();
  } /* end if */
  
  /* build formal type list node */
  if (m2c_fifo_entry_count(tmplist) > 0) {
    ftlist = m2c_ast_new_list_node(AST_FTYPELIST, tmplist);
  }
  else /* no formal type list */ {
    ftlist = m2c_ast_empty_node();
  } /* end if */
  
  m2c_fifo_release(tmplist);
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_PROCTYPE, ftlist, rtype, NULL);
  
  return lookahead;
} /* end procedure_type */


/* --------------------------------------------------------------------------
 * private function formal_type()
 * --------------------------------------------------------------------------
 * formalType :=
 *   simpleFormalType | attributedFormalType
 *   ;
 * ----------------------------------------------------------------------- */

m2c_token_t simple_formal_type (m2c_parser_context_t p);

m2c_token_t attributed_formal_type (m2c_parser_context_t p);

m2c_token_t formal_type (m2c_parser_context_t p) {
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("formalType");
  
  lookahead = m2c_next_sym(p->lexer);
  
  /* simpleFormalType */
  if ((lookahead == TOKEN_ARRAY) || (lookahead == TOKEN_IDENTIFIER)) {
    lookahead = simple_formal_type(p);
  }
  /* | attributedFormalType */
  else if ((lookahead == TOKEN_CONST) || (lookahead == TOKEN_VAR)) {
    lookahead = attributed_formal_type(p);
  }
  else /* unreachable code */ {
    /* fatal error -- abort */
    exit(-1);
  } /* end if */
  
  /* AST node is passed through in p->ast */
  
  return lookahead;
} /* end formal_type */


/* --------------------------------------------------------------------------
 * private function simple_formal_type()
 * --------------------------------------------------------------------------
 * simpleFormalType :=
 *   ( ARRAY OF )? typeIdent
 *   ;
 *
 * astnode:
 *  (IDENT ident) | (QUALIDENT q0 q1 q2 ... qN ident) | (OPENARRAY identNode)
 * ----------------------------------------------------------------------- */

m2c_token_t simple_formal_type (m2c_parser_context_t p) {
  m2c_astnode_t id;
  m2c_token_t lookahead;
  bool open_array = false;
  
  PARSER_DEBUG_INFO("simpleFormalType");
  
  lookahead = m2c_next_sym(p->lexer);
  
  /* ( ARRAY OF )? */
  if (lookahead == TOKEN_ARRAY) {
    lookahead = m2c_consume_sym(p->lexer);
    open_array = true;
    
    /* OF */
    if (match_token(p, TOKEN_OF, FOLLOW(SIMPLE_FORMAL_TYPE))) {
      lookahead = m2c_consume_sym(p->lexer);
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  
  /* typeIdent */
  if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(SIMPLE_FORMAL_TYPE))) {
    lookahead = qualident(p);
    id = p->ast;
    /* astnode: (IDENT ident) | (QUALIDENT q0 q1 q2 ... qN ident) */
  } /* end if */
  
  if (open_array) {
    p->ast = m2c_ast_new_node(AST_OPENARRAY, id, NULL);
    /* astnode: (OPENARRAY identNode) */
  } /* end if */
  
  return lookahead;
} /* end simple_formal_type */


/* --------------------------------------------------------------------------
 * private function attributed_formal_type()
 * --------------------------------------------------------------------------
 * attributedFormalType :=
 *   ( CONST | VAR ) simpleFormalType
 *   ;
 *
 * astnode: (CONSTP simpleFormalTypeNode) | (VARP simpleFormalTypeNode)
 * ----------------------------------------------------------------------- */

m2c_token_t attributed_formal_type (m2c_parser_context_t p) {
  m2c_astnode_t sftype;
  m2c_token_t lookahead;
  bool const_attr = false;
  
  PARSER_DEBUG_INFO("attributedFormalType");
  
  lookahead = m2c_next_sym(p->lexer);
  
  /* CONST */
  if (lookahead == TOKEN_CONST) {
    lookahead = m2c_consume_sym(p->lexer);
    const_attr = true;
  }
  /* | VAR */
  else if (lookahead == TOKEN_VAR) {
    lookahead = m2c_consume_sym(p->lexer);
  }
  else /* unreachable code */ {
    /* fatal error -- abort */
    exit(-1);
  } /* end if */
  
  /* simpleFormalType */
  if (match_set(p, FIRST(SIMPLE_FORMAL_TYPE),
      FOLLOW(ATTRIBUTED_FORMAL_TYPE))) {
    lookahead = simple_formal_type(p);
    sftype = p->ast;
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  if (const_attr) {
    p->ast = m2c_ast_new_node(AST_CONSTP, sftype, NULL);
  }
  else {
    p->ast = m2c_ast_new_node(AST_VARP, sftype, NULL);
  } /* end if */
  
  return lookahead;
} /* end attributed_formal_type */


/* --------------------------------------------------------------------------
 * private function procedure_header()
 * --------------------------------------------------------------------------
 * procedureHeader :=
 *   PROCEDURE procedureSignature
 *   ;
 * ----------------------------------------------------------------------- */

m2c_token_t procedure_signature (m2c_parser_context_t p);

m2c_token_t procedure_header (m2c_parser_context_t p) {
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("procedureHeader");
  
  /* PROCEDURE */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* procedureSignature */
  if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(PROCEDURE_HEADER))) {
    lookahead = procedure_signature(p);
  } /* end if */
  
  /* AST node is passed through in p->ast */
  
  return lookahead;
} /* end procedure_header */


/* --------------------------------------------------------------------------
 * private function procedure_signature()
 * --------------------------------------------------------------------------
 * procedureSignature :=
 *   Ident ( '(' formalParamList? ')' ( ':' returnedType )? )?
 *   ;
 *
 * astnode: (PROCDEF identNode formalParamListNode returnTypeNode)
 * ----------------------------------------------------------------------- */

m2c_token_t formal_param_list (m2c_parser_context_t p);

m2c_token_t procedure_signature (m2c_parser_context_t p) {
  m2c_astnode_t id, fplist, rtype;
  m2c_string_t ident;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("procedureSignature");
  
  /* Ident */
  lookahead = m2c_consume_sym(p->lexer);
  ident = m2c_lexer_current_lexeme(p->lexer);
  id = m2c_ast_new_terminal_node(AST_IDENT, ident);
  
  /* ( '(' formalParamList? ')' ( ':' returnedType )? )? */
  if (lookahead == TOKEN_LEFT_PAREN) {
    
    /* '(' */
    lookahead = m2c_consume_sym(p->lexer);
    
    /* formalParamList? */
    if ((lookahead == TOKEN_IDENTIFIER) ||
        (lookahead == TOKEN_VAR)) {
      lookahead = formal_param_list(p);
      fplist = p->ast;
    }
    else {
      fplist = m2c_ast_empty_node();
    } /* end if */
    
    /* ')' */
    if (match_token(p, TOKEN_RIGHT_PAREN, FOLLOW(PROCEDURE_TYPE))) {
      lookahead = m2c_consume_sym(p->lexer);
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
    
    /* ( ':' returnedType )? */
    if (lookahead == TOKEN_COLON) {
      /* ':' */
      lookahead = m2c_consume_sym(p->lexer);
    
      /* returnedType */
      if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(PROCEDURE_TYPE))) {
        lookahead = qualident(p);
        rtype = p->ast;
      } /* end if */
    }
    else {
      rtype = m2c_ast_empty_node();
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_PROCDEF, id, fplist, rtype, NULL);
  
  return lookahead;
} /* end procedure_signature */


/* --------------------------------------------------------------------------
 * private function formal_param_list()
 * --------------------------------------------------------------------------
 * formalParamList :=
 *   formalParams ( ';' formalParams )*
 *   ;
 *
 * astnode: (FPARAMLIST fparams+)
 * ----------------------------------------------------------------------- */

m2c_token_t formal_params (m2c_parser_context_t p);

m2c_token_t formal_param_list (m2c_parser_context_t p) {
  m2c_fifo_t tmplist;
  m2c_token_t lookahead;
  uint_t line_of_semicolon, column_of_semicolon;
  
  PARSER_DEBUG_INFO("formalParamList");
  
  /* formalParams */
  lookahead = formal_params(p);
  tmplist = m2c_fifo_new_queue(p->ast);
  
  /* ( ';' formalParams )* */
  while (lookahead == TOKEN_SEMICOLON) {
    /* ';' */
    line_of_semicolon = m2c_lexer_lookahead_line(p->lexer);
    column_of_semicolon = m2c_lexer_lookahead_column(p->lexer);
    lookahead = m2c_consume_sym(p->lexer);
    
    /* check if semicolon occurred at the end of a formal parameter list */
    if (lookahead == TOKEN_RIGHT_PAREN) {
    
      if (m2c_option_errant_semicolon()) {
        /* treat as warning */
        m2c_emit_warning_w_pos
          (M2C_SEMICOLON_AFTER_FORMAL_PARAM_LIST,
           line_of_semicolon, column_of_semicolon);
        p->warning_count++;
      }
      else /* treat as error */ {
        m2c_emit_error_w_pos
          (M2C_SEMICOLON_AFTER_FORMAL_PARAM_LIST,
           line_of_semicolon, column_of_semicolon);
        p->error_count++;
      } /* end if */
      
      /* print source line */
      if (m2c_option_verbose()) {
        m2c_print_line_and_mark_column(p->lexer,
          line_of_semicolon, column_of_semicolon);
      } /* end if */
    
      /* leave field list sequence loop to continue */
      break;
    } /* end if */
    
    /* formalParams */
    if (match_set(p, FIRST(FORMAL_PARAMS), FOLLOW(FORMAL_PARAMS))) {
      lookahead = formal_params(p);
      m2c_fifo_enqueue(tmplist, p->ast);
    } /* end if */
  } /* end while */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_list_node(AST_FPARAMLIST, tmplist);
  m2c_fifo_release(tmplist);
  
  return lookahead;
} /* end formal_param_list */


/* --------------------------------------------------------------------------
 * private function formal_params()
 * --------------------------------------------------------------------------
 * formalParams :=
 *   simpleFormalParams | attribFormalParams
 *   ;
 * ----------------------------------------------------------------------- */

m2c_token_t simple_formal_params (m2c_parser_context_t p);

m2c_token_t attrib_formal_params (m2c_parser_context_t p);

m2c_token_t formal_params (m2c_parser_context_t p) {
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("formalParams");
  
  lookahead = m2c_next_sym(p->lexer);
  
  /* simpleFormalParams */
  if (lookahead == TOKEN_IDENTIFIER) {
    lookahead = simple_formal_params(p);
  }
  /* | attribFormalParams */
  else if ((lookahead == TOKEN_CONST) || (lookahead == TOKEN_VAR)) {
    lookahead = attrib_formal_params(p);
  }
  else /* unreachable code */ {
    /* fatal error -- abort */
      exit(-1);
  } /* end if */
  
  /* AST node is passed through in p->ast */
  
  return lookahead;
} /* end formal_params */


/* --------------------------------------------------------------------------
 * private function simple_formal_params()
 * --------------------------------------------------------------------------
 * simpleFormalParams :=
 *   identList ':' simpleFormalType
 *   ;
 *
 * astnode: (FPARAMS identListNode simpleFormalTypeNode)
 * ----------------------------------------------------------------------- */

m2c_token_t simple_formal_params (m2c_parser_context_t p) {
  m2c_astnode_t idlist, sftype;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("simpleFormalParams");
  
  /* IdentList */
  lookahead = ident_list(p);
  idlist = p->ast;
  
  /* ':' */
  if (match_token(p, TOKEN_COLON, FOLLOW(SIMPLE_FORMAL_PARAMS))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* formalType */
    if (match_set(p, FIRST(FORMAL_TYPE), FOLLOW(SIMPLE_FORMAL_PARAMS))) {
      lookahead = simple_formal_type(p);
      sftype = p->ast;
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_FPARAMS, idlist, sftype, NULL);
  
  return lookahead;
} /* end simple_formal_params */


/* --------------------------------------------------------------------------
 * private function attrib_formal_params()
 * --------------------------------------------------------------------------
 * attribFormalParams :=
 *   ( CONST | VAR ) simpleFormalParams
 *   ;
 *
 * astnode: (FPARAMS identListNode formalTypeNode)
 * ----------------------------------------------------------------------- */

m2c_token_t attrib_formal_params (m2c_parser_context_t p) {
  m2c_astnode_t aftype, sftype;
  m2c_token_t lookahead;
  bool const_attr = false;
  
  PARSER_DEBUG_INFO("attribFormalParams");
  
  lookahead = m2c_next_sym(p->lexer);
  
  /* CONST */
  if (lookahead == TOKEN_CONST) {
    lookahead = m2c_consume_sym(p->lexer);
    const_attr = true;
  }
  /* | VAR */
  else if (lookahead == TOKEN_VAR) {
    lookahead = m2c_consume_sym(p->lexer);
  }
  else /* unreachable code */ {
    /* fatal error -- abort */
    exit(-1);
  } /* end if */
  
  /* simpleFormalParams */
  if (match_set(p, FIRST(SIMPLE_FORMAL_PARAMS),
      FOLLOW(ATTRIB_FORMAL_PARAMS))) {
    lookahead = simple_formal_params(p);
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  sftype = m2c_ast_subnode_for_index(p->ast, 1);
  
  if (const_attr) {
    aftype = m2c_ast_new_node(AST_CONSTP, sftype, NULL);
  }
  else {
    aftype = m2c_ast_new_node(AST_VARP, sftype, NULL);
  } /* end if */
  
  m2c_ast_replace_subnode(p->ast, 1, aftype);
  
  return lookahead;
} /* end attrib_formal_params */


/* ************************************************************************ *
 * Implementation and Program Module Syntax                                 *
 * ************************************************************************ */


/* --------------------------------------------------------------------------
 * private function implementation_module()
 * --------------------------------------------------------------------------
 * implementationModule :=
 *   IMPLEMENTATION programModule
 *   ;
 * ----------------------------------------------------------------------- */

m2c_token_t implementation_module (m2c_parser_context_t p) {
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("implementationModule");
  
  /* IMPLEMENTATION */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* programModule */
  if (match_token(p, TOKEN_MODULE, FOLLOW(PROGRAM_MODULE))) {
    lookahead = program_module(p);
  } /* end if */
  
  /* AST node is passed through in p->ast */
  
  return lookahead;
} /* end implementation_module */


/* --------------------------------------------------------------------------
 * private function program_module()
 * --------------------------------------------------------------------------
 * programModule :=
 *   MODULE moduleIdent modulePriority? ';'
 *   import* block moduleIdent '.'
 *   ;
 *
 * moduleIdent := Ident ;
 *
 * astnode: (IMPMOD identNode priorityNode importListNode blockNode)
 * ----------------------------------------------------------------------- */

m2c_token_t module_priority (m2c_parser_context_t p);

m2c_token_t block (m2c_parser_context_t p);

m2c_token_t program_module (m2c_parser_context_t p) {
  m2c_astnode_t id, prio, implist, body;
  m2c_string_t ident1, ident2;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("programModule");
  
  /* MODULE */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* moduleIdent */
  if (match_token(p, TOKEN_IDENTIFIER, RESYNC(IMPORT_OR_BLOCK))) {
    lookahead = m2c_consume_sym(p->lexer);
    ident1 = m2c_lexer_current_lexeme(p->lexer);
    
    /* modulePriority? */
    if (lookahead == TOKEN_LEFT_BRACKET) {
      lookahead = module_priority(p);
      prio = p->ast;
    }
    else /* no module priority */ {
      prio = m2c_ast_empty_node();
    } /* end while */
    
    /* ';' */
    if (match_token(p, TOKEN_SEMICOLON, RESYNC(IMPORT_OR_BLOCK))) {
      lookahead = m2c_consume_sym(p->lexer);
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  }
  else /* resync */ {
    lookahead = m2c_next_sym(p->lexer);
  } /* end if */
  
  tmplist = m2c_fifo_new_queue(NULL);
  
  /* import* */
  while ((lookahead == TOKEN_IMPORT) ||
         (lookahead == TOKEN_FROM)) {
    lookahead = import(p);
    m2c_fifo_enqueue(tmplist, p->ast);
  } /* end while */
  
  if (m2c_fifo_entry_count(tmplist) > 0) {
    implist = m2c_ast_new_list_node(AST_IMPLIST, tmplist);
  }
  else /* no import list */ {
    implist = m2c_ast_empty_node();
  } /* end if */
  
  m2c_fifo_release(tmplist);
  
  /* block */
  if (match_set(p, FIRST(BLOCK), FOLLOW(PROGRAM_MODULE))) {
    lookahead = block(p);
    body = p->ast;
    
    /* moduleIdent */
    if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(PROGRAM_MODULE))) {
      lookahead = m2c_consume_sym(p->lexer);
      ident2 = m2c_lexer_current_lexeme(p->lexer);
      
      if (ident1 != ident2) {
        /* TO DO: report error -- module identifiers don't match */ 
      } /* end if */
      
      if (match_token(p, TOKEN_PERIOD, FOLLOW(PROGRAM_MODULE))) {
        lookahead = m2c_consume_sym(p->lexer);
      } /* end if */
    } /* end if */
  } /* end if */  
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_IMPMOD, id, prio, implist, body, NULL);
  
  return lookahead;
} /* end program_module */


/* --------------------------------------------------------------------------
 * private function module_priority()
 * --------------------------------------------------------------------------
 * modulePriority :=
 *   '[' constExpression ']'
 *   ;
 * ----------------------------------------------------------------------- */

m2c_token_t module_priority (m2c_parser_context_t p) {
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("modulePriority");
  
  /* '[' */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* constExpression */
  if (match_set(p, FIRST(EXPRESSION), FOLLOW(MODULE_PRIORITY))) {
    lookahead = const_expression(p);
    
    /* ']' */
    if (match_token(p, TOKEN_RIGHT_BRACKET, FOLLOW(MODULE_PRIORITY))) {
      lookahead = m2c_consume_sym(p->lexer);
    } /* end if */
  } /* end if */
  
  /* AST node is passed through in p->ast */
  
  return lookahead;
} /* end module_priority */


/* --------------------------------------------------------------------------
 * private function block()
 * --------------------------------------------------------------------------
 * block :=
 *   declaration* ( BEGIN statementSequence )? END
 *   ;
 *
 * astnode: (BLOCK declarationListNode statementSeqNode)
 * ----------------------------------------------------------------------- */

m2c_token_t declaration (m2c_parser_context_t p);

m2c_token_t statement_sequence (m2c_parser_context_t p);

m2c_token_t block (m2c_parser_context_t p) {
  m2c_astnode_t decllist, stmtseq;
  m2c_fifo_t tmplist;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("block");
  
  lookahead = m2c_next_sym(p->lexer);
  
  tmplist = m2c_fifo_new_queue(NULL);
  
  /* declaration* */
  while ((lookahead == TOKEN_CONST) ||
         (lookahead == TOKEN_TYPE) ||
         (lookahead == TOKEN_VAR) ||
         (lookahead == TOKEN_PROCEDURE) ||
         (lookahead == TOKEN_MODULE)) {
    lookahead = declaration(p);
    m2c_fifo_enqueue(tmplist, p->ast);
  } /* end while */
  
  if (m2c_fifo_entry_count(tmplist) > 0) {
    decllist = m2c_ast_new_list_node(AST_DECLLIST, tmplist);
  }
  else /* no declarations */ {
    decllist = m2c_ast_empty_node();
  } /* end if */
  
  m2c_fifo_release(tmplist);
  
  /* ( BEGIN statementSequence )? */
  if (lookahead == TOKEN_BEGIN) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* check for empty statement sequence */
    if ((m2c_tokenset_element(FOLLOW(STATEMENT_SEQUENCE), lookahead))) {
    
        /* print warning */
        m2c_emit_warning_w_pos
          (M2C_EMPTY_STMT_SEQ,
           m2c_lexer_lookahead_line(p->lexer),
           m2c_lexer_lookahead_column(p->lexer));
        p->warning_count++;
    }
    /* statementSequence */
    else if (match_set(p, FIRST(STATEMENT_SEQUENCE), FOLLOW(STATEMENT))) {
      lookahead = statement_sequence(p);
      stmtseq = p->ast;
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  }
  else /* no statement sequence */ {
    stmtseq = m2c_ast_empty_node();
  } /* end if */
  
  /* END */
  if (match_token(p, TOKEN_END, FOLLOW(BLOCK))) {
    lookahead = m2c_consume_sym(p->lexer);
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_BLOCK, decllist_node, stmtseq, NULL);
  
  return lookahead;
} /* end block */


/* --------------------------------------------------------------------------
 * private function declaration()
 * --------------------------------------------------------------------------
 * declaration :=
 *   CONST ( constDeclaration ';' )* |
 *   TYPE ( typeDeclaration ';' )* |
 *   VAR ( variableDeclaration ';' )* |
 *   procedureDeclaration ';'
 *   moduleDeclaration ';'
 *   ;
 * 
 * constDeclaration := constDefinition ;
 * ----------------------------------------------------------------------- */

m2c_token_t type_declaration (m2c_parser_context_t p);

m2c_token_t variable_declaration (m2c_parser_context_t p);

m2c_token_t procedure_declaration (m2c_parser_context_t p);

m2c_token_t module_declaration (m2c_parser_context_t p);

m2c_token_t declaration (m2c_parser_context_t p) {
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("declaration");
  
  lookahead = m2c_next_sym(p->lexer);
  
  switch (lookahead) {
    
    /* CONST */
    case TOKEN_CONST :
      lookahead = m2c_consume_sym(p->lexer);
      
      /* ( constDeclaration ';' )* */
      while (lookahead == TOKEN_IDENTIFIER) {
        lookahead = const_definition(p);
        
        /* ';' */
        if (match_token(p, TOKEN_SEMICOLON,
            RESYNC(DECLARATION_OR_IDENT_OR_SEMICOLON))) {
          lookahead = m2c_consume_sym(p->lexer);
        } /* end if */
      } /* end while */
      break;
      
    /* | TYPE */
    case TOKEN_TYPE :
      lookahead = m2c_consume_sym(p->lexer);
      
      /* ( typeDeclaration ';' )* */
      while (lookahead == TOKEN_IDENTIFIER) {
        lookahead = type_declaration(p);
        
        /* ';' */
        if (match_token(p, TOKEN_SEMICOLON,
            RESYNC(DECLARATION_OR_IDENT_OR_SEMICOLON))) {
          lookahead = m2c_consume_sym(p->lexer);
        } /* end if */
      } /* end while */
      break;
      
    /* | VAR */
    case TOKEN_VAR :
      lookahead = m2c_consume_sym(p->lexer);
      
      /* ( variableDeclaration ';' )* */
      while (lookahead == TOKEN_IDENTIFIER) {
        lookahead = variable_declaration(p);
        
        /* ';' */
        if (match_token(p, TOKEN_SEMICOLON,
            RESYNC(DECLARATION_OR_IDENT_OR_SEMICOLON))) {
          lookahead = m2c_consume_sym(p->lexer);
        } /* end if */
      } /* end while */
      break;
      
    /* | procedureDeclaration ';' */
    case TOKEN_PROCEDURE :
      lookahead = procedure_declaration(p);
      
      /* ';' */
      if (match_token(p, TOKEN_SEMICOLON,
          RESYNC(DECLARATION_OR_SEMICOLON))) {
        lookahead = m2c_consume_sym(p->lexer);
      } /* end if */
      break;
      
    /* | moduleDeclaration ';' */
    case TOKEN_MODULE :
      lookahead = module_declaration(p);
      
      /* ';' */
      if (match_token(p, TOKEN_SEMICOLON,
          RESYNC(DECLARATION_OR_SEMICOLON))) {
        lookahead = m2c_consume_sym(p->lexer);
      } /* end if */
      break;
      
    default : /* unreachable code */
      /* fatal error -- abort */
      exit(-1);
  } /* end switch */
  
  /* AST node is passed through in p->ast */
  
  return lookahead;
} /* end declaration */


/* --------------------------------------------------------------------------
 * private function type_declaration()
 * --------------------------------------------------------------------------
 * typeDeclaration :=
 *   Ident '=' ( type | varSizeRecordType )
 *   ;
 *
 * astnode: (TYPEDECL identNode typeConstructorNode)
 * ----------------------------------------------------------------------- */

m2c_token_t var_size_record_type (m2c_parser_context_t p);

m2c_token_t type_declaration (m2c_parser_context_t p) {
  m2c_astnode_t id, tc;
  m2c_string_t ident;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("typeDeclaration");
  
  /* Ident */
  lookahead = m2c_consume_sym(p->lexer);
  ident = m2c_lexer_current_lexeme(p->lexer);
  id = m2c_ast_new_terminal_node(AST_IDENT, ident);
  
  /* '=' */
  if (match_token(p, TOKEN_EQUAL, FOLLOW(TYPE_DECLARATION))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* type | varSizeRecordType */
    if (match_set(p, FIRST(TYPE_DECLARATION_TAIL),
        FOLLOW(TYPE_DECLARATION))) {
      
      /* type */
      if (lookahead != TOKEN_VAR) {
        lookahead = type(p);
        tc = p->ast;
      }
      /* | varSizeRecordType */
      else {
        lookahead = var_size_record_type(p);
        tc = p->ast;
      } /* end if */
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_TYPEDECL, id, tc, NULL);
  
  return lookahead;
} /* end type_declaration */


/* --------------------------------------------------------------------------
 * private function var_size_record_type()
 * --------------------------------------------------------------------------
 * varSizeRecordType :=
 *   VAR RECORD fieldListSequence
 *   VAR varSizeFieldIdent ':' ARRAY sizeFieldIdent OF typeIdent
 *   END
 *   ;
 *
 * astnode:
 *  (VSREC fieldListSeqNode (VSFIELD identNode identNode identNode))
 * ----------------------------------------------------------------------- */

m2c_token_t var_size_record_type (m2c_parser_context_t p) {
  m2c_astnode_t flseq, vsfield, vsfieldid, sizeid, typeid;
  m2c_string_t ident;
  uint_t line_of_semicolon, column_of_semicolon;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("varSizeRecordType");
  
  /* VAR */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* RECORD */
  if (match_token(p, TOKEN_RECORD, FOLLOW(VAR_SIZE_RECORD_TYPE))) {
    lookahead = m2c_consume_sym(p->lexer); 
    
    /* check for empty field list sequence */
    if (lookahead == TOKEN_VAR) {

        /* empty field list sequence warning */
        m2c_emit_warning_w_pos
          (M2C_EMPTY_FIELD_LIST_SEQ,
           m2c_lexer_lookahead_line(p->lexer),
           m2c_lexer_lookahead_column(p->lexer));
        p->warning_count++;
    }
    /* fieldListSequence */
    else if (match_set(p, FIRST(FIELD_LIST_SEQUENCE),
             FOLLOW(VAR_SIZE_RECORD_TYPE))) {
      lookahead = field_list_sequence(p);
      flseq = p->ast;
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
    
    /* VAR */
    if (match_token(p, TOKEN_VAR, FOLLOW(VAR_SIZE_RECORD_TYPE))) {
      lookahead = m2c_consume_sym(p->lexer);
      
      if (lookahead == TOKEN_END) {
        m2c_emit_warning_w_pos
          (M2C_EMPTY_FIELD_LIST_SEQ,
           m2c_lexer_lookahead_line(p->lexer),
           m2c_lexer_lookahead_column(p->lexer));
           m2c_consume_sym(p->lexer);
        p->warning_count++;
      }
      /* varSizeFieldIdent */
      else if (match_token(p, TOKEN_IDENTIFIER,
               FOLLOW(VAR_SIZE_RECORD_TYPE))) {
        lookahead = m2c_consume_sym(p->lexer);
        ident = m2c_lexer_current_lexeme(p->lexer);
        vsfieldid = m2c_ast_new_terminal_node(AST_IDENT, ident);
      
        /* ':' */
        if (match_token(p, TOKEN_COLON, FOLLOW(VAR_SIZE_RECORD_TYPE))) {
          lookahead = m2c_consume_sym(p->lexer);
        
          /* ARRAY */
          if (match_token(p, TOKEN_ARRAY, FOLLOW(VAR_SIZE_RECORD_TYPE))) {
            lookahead = m2c_consume_sym(p->lexer);
          
            /* sizeFieldIdent */
            if (match_token(p, TOKEN_IDENTIFIER,
                FOLLOW(VAR_SIZE_RECORD_TYPE))) {
              lookahead = m2c_consume_sym(p->lexer);
              ident = m2c_lexer_current_lexeme(p->lexer);
              sizeid = m2c_ast_new_terminal_node(AST_IDENT, ident);
            
              /* OF */
              if (match_token(p, TOKEN_OF, FOLLOW(VAR_SIZE_RECORD_TYPE))) {
                lookahead = m2c_consume_sym(p->lexer);
              
                /* typeIdent */
                if (match_token(p, TOKEN_IDENTIFIER,
                    FOLLOW(VAR_SIZE_RECORD_TYPE))) {
                  lookahead = qualident(p);
                  typeid = p->ast;
                  
                  /* check for errant semicolon */
                  if (lookahead == TOKEN_SEMICOLON) {
                    line_of_semicolon =
                      m2c_lexer_lookahead_line(p->lexer);
                    column_of_semicolon =
                      m2c_lexer_lookahead_column(p->lexer);
                  
                    if (m2c_option_errant_semicolon()) {
                      /* treat as warning */
                      m2c_emit_warning_w_pos
                        (M2C_SEMICOLON_AFTER_FIELD_LIST_SEQ,
                         line_of_semicolon, column_of_semicolon);
                      p->warning_count++;
                    }
                    else /* treat as error */ {
                      m2c_emit_error_w_pos
                        (M2C_SEMICOLON_AFTER_FIELD_LIST_SEQ,
                         line_of_semicolon, column_of_semicolon);
                      p->error_count++;
                    } /* end if */
                    
                    m2c_consume_sym(p->lexer);
                    
                    /* print source line */
                    if (m2c_option_verbose()) {
                      m2c_print_line_and_mark_column(p->lexer,
                        line_of_semicolon, column_of_semicolon);
                    } /* end if */
                  } /* end if */
                  
                  if (match_token(p, TOKEN_END,
                      FOLLOW(VAR_SIZE_RECORD_TYPE))) {
                    lookahead = m2c_consume_sym(p->lexer);
                  } /* end if */
                } /* end if */
              } /* end if */
            } /* end if */
          } /* end if */
        } /* end if */
      } /* end if */
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  vsfield = m2c_ast_new_node(AST_VSFIELD, vsfieldid, sizeid, typeid, NULL);
  p->ast = m2c_ast_new_node(AST_VSREC, flseq, vsfield, NULL);
  
  return lookahead;
} /* end var_size_record_type */


/* --------------------------------------------------------------------------
 * private function variable_declaration()
 * --------------------------------------------------------------------------
 * variableDeclaration :=
 *   identList ':' type
 *   ;
 *
 * astnode: (VARDECL identListNode typeConstructorNode)
 * ----------------------------------------------------------------------- */

m2c_token_t variable_declaration (m2c_parser_context_t p) {
  m2c_astnode_t idlist, tc;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("variableDeclaration");
  
  /* IdentList */
  lookahead = ident_list(p);
  idlist = p->ast;
  
  /* ':' */
  if (match_token(p, TOKEN_COLON, FOLLOW(VARIABLE_DECLARATION))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* type */
    if (match_set(p, FIRST(TYPE), FOLLOW(VARIABLE_DECLARATION))) {
      lookahead = type(p);
      tc = p->ast;
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_VARDECL, idlist, tc, NULL);
  
  return lookahead;
} /* end variable_declaration */


/* --------------------------------------------------------------------------
 * private function procedure_declaration()
 * --------------------------------------------------------------------------
 * procedureDeclaration :=
 *   procedureHeader ';' block Ident
 *   ;
 *
 * astnode: (PROC procDefinitionNode blockNode)
 * ----------------------------------------------------------------------- */

m2c_token_t procedure_declaration (m2c_parser_context_t p) {
  m2c_astnode_t procdef, body;
  m2c_string_t ident;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("procedureDeclaration");
  
  /* procedureHeader */
  lookahead = procedure_header(p);
  procdef = p->ast;
  
  /* ';' */
  if (match_token(p, TOKEN_SEMICOLON, FOLLOW(PROCEDURE_DECLARATION))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* block */
    if (match_set(p, FIRST(BLOCK), FOLLOW(PROCEDURE_DECLARATION))) {
      lookahead = block(p);
      body = p->ast;
      
      /* Ident */
      if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(PROCEDURE_DECLARATION))) {
        lookahead = m2c_consume_sym(p->lexer);
        ident = m2c_lexer_current_lexeme(p->lexer);
        
        /* TO DO: check if procedure identifiers match */
      } /* end if */
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_PROC, procdef, body, NULL);
  
  return lookahead;
} /* end procedure_declaration */


/* --------------------------------------------------------------------------
 * private function module_declaration()
 * --------------------------------------------------------------------------
 * moduleDeclaration :=
 *   MODULE moduleIdent modulePriority? ';'
 *   import* export? block moduleIdent
 *   ;
 *
 * astnode:
 *  (MODDECL identNode prioNode importListNode exportListNode blockNode)
 * ----------------------------------------------------------------------- */

m2c_token_t export (m2c_parser_context_t p);

m2c_token_t module_declaration (m2c_parser_context_t p) {
  m2c_astnode_t id, prio, implist, exp, body;
  m2c_fifo_t tmplist;
  m2c_string_t ident1, ident2;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("moduleDeclaration");
  
  /* MODULE */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* moduleIdent */
  if (match_token(p, TOKEN_IDENTIFIER, RESYNC(IMPORT_OR_BLOCK))) {
    lookahead = m2c_consume_sym(p->lexer);
    ident1 = m2c_lexer_current_lexeme(p->lexer);
    
    /* modulePriority? */
    if (lookahead == TOKEN_LEFT_BRACKET) {
      lookahead = module_priority(p);
      prio = p->ast;
    }
    else /* no module priority */ {
      prio = m2c_ast_empty_node();
    } /* end while */
    
    /* ';' */
    if (match_token(p, TOKEN_SEMICOLON, RESYNC(IMPORT_OR_BLOCK))) {
      lookahead = m2c_consume_sym(p->lexer);
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  }
  else /* resync */ {
    lookahead = m2c_next_sym(p->lexer);
  } /* end if */
  
  tmplist = m2c_fifo_new_queue(NULL);
  
  /* import* */
  while ((lookahead == TOKEN_IMPORT) ||
         (lookahead == TOKEN_FROM)) {
    lookahead = import(p);
    m2c_fifo_enqueue(tmplist, p->ast);
  } /* end while */
  
  if (m2c_fifo_entry_count(tmplist) > 0) {
    implist = m2c_ast_new_list_node(AST_IMPLIST, tmplist);
  }
  else /* no import list */ {
    implist = m2c_ast_empty_node();
  } /* end if */
  
  m2c_fifo_release(tmplist);
  
  /* export? */
  if (lookahead == TOKEN_EXPORT) {
    lookahead = export(p);
    exp = p->ast;
  }
  else /* no export list */ {
    exp = m2c_ast_empty_node();
  } /* end while */
  
  /* block */
  if (match_set(p, FIRST(BLOCK), FOLLOW(MODULE_DECLARATION))) {
    lookahead = block(p);
    body = p->ast;
    
    /* moduleIdent */
    if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(MODULE_DECLARATION))) {
      lookahead = m2c_consume_sym(p->lexer);
      module_ident_end = m2c_lexer_current_lexeme(p->lexer);
      
      if (ident1 != ident2) {
        /* TO DO: report error -- module identifiers don't match */ 
      } /* end if */
    } /* end if */
  } /* end if */  
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_MODDECL, id, prio, implist, exp, body, NULL);
  
  return lookahead;
} /* end module_declaration */


/* --------------------------------------------------------------------------
 * private function export()
 * --------------------------------------------------------------------------
 * export :=
 *   EXPORT QUALIFIED? identList ';'
 *   ;
 *
 * astnode: (EXPORT identListNode) | (QUALEXP identListNode)
 * ----------------------------------------------------------------------- */

m2c_token_t export (m2c_parser_context_t p) {
  m2c_astnode_t idlist;
  m2c_token_t lookahead;
  bool qualified = false;
  
  PARSER_DEBUG_INFO("export");
  
  /* EXPORT */
  lookahead = m2c_consume_sym(p->lexer);
    
  /* QUALIFIED? */
  if (lookahead == TOKEN_QUALIFIED) {
    lookahead = m2c_consume_sym(p->lexer);
    qualified = true;
  } /* end if */
  
  /* identList */
  if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(EXPORT))) {
    lookahead = ident_list(p);
    idlist = p->ast;
    
    /* ';' */
    if (match_token(p, TOKEN_SEMICOLON, FOLLOW(EXPORT))) {
      lookahead = m2c_consume_sym(p->lexer);
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  if (qualified) {
    p->ast = m2c_ast_new_node(AST_QUALEXP, idlist, NULL);
  }
  else /* unqualified */ {
    p->ast = m2c_ast_new_node(AST_EXPORT, idlist, NULL);
  } /* end if */
  
  return lookahead;
} /* end export */


/* --------------------------------------------------------------------------
 * private function statement_sequence()
 * --------------------------------------------------------------------------
 * statementSequence :=
 *   statement ( ';' statement )*
 *   ;
 *
 * astnode: (STMTSEQ stmtNode+)
 * ----------------------------------------------------------------------- */

m2c_token_t statement (m2c_parser_context_t p);

m2c_token_t statement_sequence (m2c_parser_context_t p) {
  m2c_fifo_t tmplist;
  m2c_token_t lookahead;
  uint_t line_of_semicolon, column_of_semicolon;
  
  PARSER_DEBUG_INFO("statementSequence");
  
  /* statement */
  lookahead = statement(p);
  tmplist = m2c_fifo_new_queue(p->ast);
  
  /* ( ';' statement )* */
  while (lookahead == TOKEN_SEMICOLON) {
    /* ';' */
    line_of_semicolon = m2c_lexer_lookahead_line(p->lexer);
    column_of_semicolon = m2c_lexer_lookahead_column(p->lexer);
    lookahead = m2c_consume_sym(p->lexer);
    
    /* check if semicolon occurred at the end of a statement sequence */
    if (m2c_tokenset_element(FOLLOW(STATEMENT_SEQUENCE), lookahead)) {
    
      if (m2c_option_errant_semicolon()) {
        /* treat as warning */
        m2c_emit_warning_w_pos
          (M2C_SEMICOLON_AFTER_STMT_SEQ,
           line_of_semicolon, column_of_semicolon);
        p->warning_count++;
      }
      else /* treat as error */ {
        m2c_emit_error_w_pos
          (M2C_SEMICOLON_AFTER_STMT_SEQ,
           line_of_semicolon, column_of_semicolon);
        p->error_count++;
      } /* end if */
      
      /* print source line */
      if (m2c_option_verbose()) {
        m2c_print_line_and_mark_column(p->lexer,
          line_of_semicolon, column_of_semicolon);
      } /* end if */
    
      /* leave statement sequence loop to continue */
      break;
    } /* end if */
    
    /* statement */
    if (match_set(p, FIRST(STATEMENT),
        RESYNC(FIRST_OR_FOLLOW_OF_STATEMENT))) {
      lookahead = statement(p);
      m2c_fifo_enqueue(tmplist, p->ast);
    } /* end if */
  } /* end while */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_list_node(AST_STMTSEQ, tmplist);
  m2c_fifo_release(tmplist);
  
  return lookahead;
} /* end statement_sequence */


/* --------------------------------------------------------------------------
 * private function statement()
 * --------------------------------------------------------------------------
 * statement :=
 *   assignmentOrProcCall | returnStatement | withStatement | ifStatement |
 *   caseStatement | loopStatement | whileStatement | repeatStatement |
 *   forStatement | EXIT
 *   ;
 * ----------------------------------------------------------------------- */

m2c_token_t assignment_or_proc_call (m2c_parser_context_t p);

m2c_token_t return_statement (m2c_parser_context_t p);

m2c_token_t with_statement (m2c_parser_context_t p);

m2c_token_t if_statement (m2c_parser_context_t p);

m2c_token_t case_statement (m2c_parser_context_t p);

m2c_token_t loop_statement (m2c_parser_context_t p);

m2c_token_t while_statement (m2c_parser_context_t p);

m2c_token_t repeat_statement (m2c_parser_context_t p);

m2c_token_t for_statement (m2c_parser_context_t p);

m2c_token_t statement (m2c_parser_context_t p) {
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("statement");
  
  lookahead = m2c_next_sym(p->lexer);
  
  switch (lookahead) {
  
    /* assignmentOrProcCall */
    case TOKEN_IDENTIFIER :
      lookahead = assignment_or_proc_call(p);
      break;
      
    /* | returnStatement */
    case TOKEN_RETURN :
      lookahead = return_statement(p);
      break;
      
    /* | withStatement */
    case TOKEN_WITH :
      lookahead = with_statement(p);
      break;
      
    /* | ifStatement */
    case TOKEN_IF :
      lookahead = if_statement(p);
      break;
      
    /* | caseStatement */
    case TOKEN_CASE :
      lookahead = case_statement(p);
      break;
      
    /* | loopStatement */
    case TOKEN_LOOP :
      lookahead = loop_statement(p);
      break;
      
    /* | whileStatement */
    case TOKEN_WHILE :
      lookahead = while_statement(p);
      break;
      
    /* | repeatStatement */
    case TOKEN_REPEAT :
      lookahead = repeat_statement(p);
      break;
      
    /* | forStatement */
    case TOKEN_FOR :
      lookahead = for_statement(p);
      break;
      
    /* | EXIT */
    case TOKEN_EXIT :
      lookahead = m2c_consume_sym(p->lexer);
      p->ast = m2c_ast_new_node(AST_EXIT, NULL);
      break;
      
    default : /* unreachable code */
      /* fatal error -- abort */
      exit(-1);
    } /* end switch */
  
  return lookahead;
} /* end statement */


/* --------------------------------------------------------------------------
 * private function assignment_or_proc_call()
 * --------------------------------------------------------------------------
 * assignmentOrProcCall :=
 *   designator ( ':=' expression | actualParameters )?
 *   ;
 *
 * astnode:
 *  (ASSIGN designatorNode exprNode) | (PCALL designatorNode argsNode)
 * ----------------------------------------------------------------------- */

m2c_token_t designator (m2c_parser_context_t p);

m2c_token_t expression (m2c_parser_context_t p);

m2c_token_t actual_parameters (m2c_parser_context_t p);

m2c_token_t assignment_or_proc_call (m2c_parser_context_t p) {
  m2c_astnode_t desig;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("assignmentOrProcCall");
  
  /* designator */
  lookahead = designator(p);
  desig = p->ast;
  
  /* ( ':=' expression | actualParameters )? */
  if (lookahead == TOKEN_ASSIGN) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* expression */
    if (match_set(p, FIRST(EXPRESSION), FOLLOW(ASSIGNMENT_OR_PROC_CALL))) {
      lookahead = expression(p);
      p->ast = m2c_ast_new_node(AST_ASSIGN, desig, p->ast, NULL);
      /* astnode: (ASSIGN designatorNode exprNode) */
    } /* end if */
  }
  /* | actualParameters */
  else if (lookahead == TOKEN_LEFT_PAREN) {
    lookahead = actual_parameters(p);
    p->ast = m2c_ast_new_node(AST_PCALL, desig, p->ast, NULL);
    /* astnode: (PCALL designatorNode argsNode) */
  } /* end if */
  
  return lookahead;
} /* end assignment_or_proc_call */


/* --------------------------------------------------------------------------
 * private function actual_parameters()
 * --------------------------------------------------------------------------
 * actualParameters :=
 *   '(' ( expression ( ',' expression )* )? ')'
 *   ;
 *
 * astnode: (ARGS exprNode+) | (EMPTY)
 * ----------------------------------------------------------------------- */

m2c_token_t actual_parameters (m2c_parser_context_t p) {
  m2c_fifo_t tmplist;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("actualParameters");
  
  /* '(' */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* ( expression ( ',' expression )* )? */
  if (m2c_tokenset_member(FIRST(EXPRESSION), lookahead)) {
    /* expression */
    lookahead = expression(p);
    tmplist = m2c_fifo_new_queue(p->ast);
  
    /* ( ',' expression )* */
    while (lookahead == TOKEN_COMMA) {
      /* ',' */
      lookahead = m2c_consume_sym(p->lexer);
    
      /* expression */
      if (match_set(p, FIRST(EXPRESSION), FOLLOW(EXPRESSION))) {
        lookahead = expression(p);
        m2c_fifo_enqueue(tmplist, p->ast);
      } /* end if */
    } /* end while */
    
    p->ast = m2c_ast_new_list_node(AST_ARGS, tmplist);
    m2c_fifo_release(tmplist);
  }
  else /* no arguments */ {
    p->ast = m2c_ast_empty_node();
  } /* end if */
  
  /* ')' */
  if (match_token(p, TOKEN_RIGHT_PAREN, FOLLOW(ACTUAL_PARAMETERS))) {
    lookahead = m2c_consume_sym(p->lexer);
  } /* end if */
  
  return lookahead;
} /* end actual_parameters */


/* --------------------------------------------------------------------------
 * private function return_statement()
 * --------------------------------------------------------------------------
 * returnStatement :=
 *   RETURN expression?
 *   ;
 *
 * astnode: (RETURN exprNode) | (RETURN (EMPTY))
 * ----------------------------------------------------------------------- */

m2c_token_t return_statement (m2c_parser_context_t p) {
  m2c_astnode_t expr;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("returnStatement");
  
  /* RETURN */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* expression? */
  if (m2c_tokenset_element(FIRST(EXPRESSION), lookahead)) {
    lookahead = expression(p);
    expr = p->ast;
  }
  else {
    expr = m2c_ast_empty_node();
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_RETURN, expr, NULL);
  
  return lookahead;
} /* end return_statement */


/* --------------------------------------------------------------------------
 * private function with_statement()
 * --------------------------------------------------------------------------
 * withStatement :=
 *   WITH designator DO statementSequence END
 *   ;
 *
 * astnode: (WITH designatorNode statementSeqNode)
 * ----------------------------------------------------------------------- */

m2c_token_t with_statement (m2c_parser_context_t p) {
  m2c_astnode_t desig, stmtseq;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("withStatement");
  
  /* WITH */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* designator */
  if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(WITH_STATEMENT))) {
    lookahead = designator(p);
    desig = p->ast;
    
    /* DO */
    if (match_token(p, TOKEN_DO, FOLLOW(WITH_STATEMENT))) {
      lookahead = m2c_consume_sym(p->lexer);
      
      /* check for empty statement sequence */
      if (lookahead == TOKEN_END) {
    
        /* empty statement sequence warning */
        m2c_emit_warning_w_pos
          (M2C_EMPTY_STMT_SEQ,
           m2c_lexer_lookahead_line(p->lexer),
           m2c_lexer_lookahead_column(p->lexer));
        p->warning_count++;
             
        /* END */
        lookahead = m2c_consume_sym(p->lexer);
      }
      /* statementSequence */
      else if (match_set(p, FIRST(STATEMENT_SEQUENCE),
               FOLLOW(WITH_STATEMENT))) {
        lookahead = statement_sequence(p);
        stmtseq = p->ast;
        
        /* END */
        if (match_token(p, TOKEN_END, FOLLOW(WITH_STATEMENT))) {
          lookahead = m2c_consume_sym(p->lexer);
        } /* end if */
      } /* end if */
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_WITH, desig, stmtseq, NULL);
  
  return lookahead;
} /* end with_statement */


/* --------------------------------------------------------------------------
 * private function if_statement()
 * --------------------------------------------------------------------------
 * ifStatement :=
 *   IF boolExpression THEN statementSequence
 *   ( ELSIF boolExpression THEN statementSequence )*
 *   ( ELSE statementSequence )?
 *   END
 *   ;
 *
 * boolExpression := expression ;
 *
 * astnode: (IF exprNode ifBranchNode elsifSeqNode elseBranchNode)
 * ----------------------------------------------------------------------- */

m2c_token_t if_statement (m2c_parser_context_t p) {
  m2c_astnode_t ifexpr, ifseq, elif, expr, stmtseq, elifseq, elseseq;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("ifStatement");
  
  /* IF */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* boolExpression */
  if (match_set(p, FIRST(EXPRESSION), RESYNC(ELSIF_OR_ELSE_OR_END))) {
    lookahead = expression(p);
    ifexpr = p->ast;
    
    /* THEN */
    if (match_token(p, TOKEN_THEN, RESYNC(ELSIF_OR_ELSE_OR_END))) {
      lookahead = m2c_consume_sym(p->lexer);
      
      /* check for empty statement sequence */
      if ((m2c_tokenset_element(RESYNC(ELSIF_OR_ELSE_OR_END), lookahead))) {
    
          /* empty statement sequence warning */
          m2c_emit_warning_w_pos
            (M2C_EMPTY_STMT_SEQ,
             m2c_lexer_lookahead_line(p->lexer),
             m2c_lexer_lookahead_column(p->lexer));
          p->warning_count++;
      }
      /* statementSequence */
      else if (match_set(p, FIRST(STATEMENT_SEQUENCE),
          RESYNC(ELSIF_OR_ELSE_OR_END))) {
        lookahead = statement_sequence(p);
        ifseq = p->ast;
      }
      else /* resync */ {
        lookahead = m2c_next_sym(p->lexer);
      } /* end if */
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  } /* end if */
  
  /* ( ELSIF boolExpression THEN statementSequence )* */
  while (lookahead == TOKEN_ELSIF) {
    
    /* ELSIF */
    lookahead = m2c_consume_sym(p->lexer);
    
    /* boolExpression */
    if (match_set(p, FIRST(EXPRESSION), RESYNC(ELSIF_OR_ELSE_OR_END))) {
      lookahead = expression(p);
      expr = p->ast;
    
      /* THEN */
      if (match_token(p, TOKEN_THEN, RESYNC(ELSIF_OR_ELSE_OR_END))) {
        lookahead = m2c_consume_sym(p->lexer);
      
        /* check for empty statement sequence */
        if ((m2c_tokenset_element
            (RESYNC(ELSIF_OR_ELSE_OR_END), lookahead))) {
    
            /* empty statement sequence warning */
            m2c_emit_warning_w_pos
              (M2C_EMPTY_STMT_SEQ,
               m2c_lexer_lookahead_line(p->lexer),
               m2c_lexer_lookahead_column(p->lexer));
            p->warning_count++;
        }
        /* statementSequence */
        else if (match_set(p, FIRST(STATEMENT_SEQUENCE),
            RESYNC(ELSIF_OR_ELSE_OR_END))) {
          lookahead = statement_sequence(p);
          stmtseq = p->ast;
          
          elif = m2c_ast_new_node(AST_ELSIF, expr, stmtseq, NULL);
          m2c_fifo_enqueue(tmplist, elif);
        }
        else /* resync */ {
          lookahead = m2c_next_sym(p->lexer);
        } /* end if */
      }
      else /* resync */ {
        lookahead = m2c_next_sym(p->lexer);
      } /* end if */
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  } /* end while */
  
  if (m2c_fifo_entry_count(tmplist) > 0) {
    elifseq = m2c_ast_new_list_node(ELSIFSEQ, tmplist);
  }
  else /* no ELSIF branches */ {
    elifseq = m2c_ast_empty_node();
  } /* end if */
  
  m2c_fifo_release(tmplist);
  
  /* ( ELSE statementSequence )? */
  if (lookahead == TOKEN_ELSE) {
  
    /* ELSE */
    lookahead = m2c_consume_sym(p->lexer);
  
    /* check for empty statement sequence */
    if (lookahead == TOKEN_END) {
  
        /* empty statement sequence warning */
        m2c_emit_warning_w_pos
          (M2C_EMPTY_STMT_SEQ,
           m2c_lexer_lookahead_line(p->lexer),
           m2c_lexer_lookahead_column(p->lexer));
        p->warning_count++;
    }
    /* statementSequence */
    else if (match_set(p, FIRST(STATEMENT_SEQUENCE), FOLLOW(IF_STATEMENT))) {
      lookahead = statement_sequence(p);
      elseseq = p->ast;
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  }
  else {
    elseseq = m2c_ast_empty_node();
  } /* end if */
  
  /* END */
  if (match_token(p, TOKEN_END, FOLLOW(IF_STATEMENT))) {
    lookahead = m2c_consume_sym(p->lexer);
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_IF, ifexpr, ifseq, elifseq, elseseq, NULL);
  
  return lookahead;
} /* end if_statement */


/* --------------------------------------------------------------------------
 * private function case_statement()
 * --------------------------------------------------------------------------
 * caseStatement :=
 *   CASE expression OF case ( '|' case )*
 *   ( ELSE statementSequence )?
 *   END
 *   ;
 *
 * NB: 'case' is a reserved word in C, we use case_branch() here instead
 *
 * astnode: (SWITCH exprNode (CASELIST caseBranchNode+) elseBranchNode)
 * ----------------------------------------------------------------------- */

m2c_token_t case_branch (m2c_parser_context_t p);

m2c_token_t case_statement (m2c_parser_context_t p) {
  m2c_astnode_t expr, caselist, elseseq;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("caseStatement");
  
  /* CASE */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* expression */
  if (match_set(p, FIRST(EXPRESSION), RESYNC(ELSE_OR_END))) {
    lookahead = expression(p);
    expr = p->ast;
    
    /* OF */
    if (match_token(p, TOKEN_OF, RESYNC(ELSE_OR_END))) {
      lookahead = m2c_consume_sym(p->lexer);
      
      /* case */
      if (match_set(p, FIRST(CASE), RESYNC(ELSE_OR_END))) {
        lookahead = case_branch(p);
        tmplist = m2c_fifo_new_queue(p->ast);
        
        /* ( '| case )* */
        while (lookahead == TOKEN_BAR) {
          /* '|' */
          lookahead = m2c_consume_sym(p->lexer);
          
          /* case */
          if (match_set(p, FIRST(CASE), RESYNC(ELSE_OR_END))) {
            lookahead = case_branch(p);
            m2c_fifo_enqueue(tmplist, p->ast);
          }
          else /* resync */ {
            lookahead = m2c_next_sym(p->lexer);
          } /* end if */
        } /* end while */
      }
      else /* resync */ {
        lookahead = m2c_next_sym(p->lexer);
      } /* end if */
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  } /* end if */
  
  caselist = m2c_ast_new_list_node(AST_CASELIST, tmplist);
  m2c_fifo_release(tmplist);
  
  /* ( ELSE statementSequence )? */
  if (lookahead == TOKEN_ELSE) {
  
    /* ELSE */
    lookahead = m2c_consume_sym(p->lexer);
  
    /* check for empty statement sequence */
    if (lookahead == TOKEN_END) {
  
        /* empty statement sequence warning */
        m2c_emit_warning_w_pos
          (M2C_EMPTY_STMT_SEQ,
           m2c_lexer_lookahead_line(p->lexer),
           m2c_lexer_lookahead_column(p->lexer));
        p->warning_count++;
    }
    /* statementSequence */
    else if
      (match_set(p, FIRST(STATEMENT_SEQUENCE), FOLLOW(CASE_STATEMENT))) {
      lookahead = statement_sequence(p);
      else_branch = p->ast;
    }
    else /* resync */ {
      lookahead = m2c_next_sym(p->lexer);
    } /* end if */
  }
  else /* no ELSE branch */ {
    elseseq = m2c_ast_empty_node();
  } /* end if */
  
  /* END */
  if (match_token(p, TOKEN_END, FOLLOW(CASE_STATEMENT))) {
    lookahead = m2c_consume_sym(p->lexer);
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_SWITCH, expr, caselist, elseseq, NULL);
  
  return lookahead;
} /* end case_statement */


/* --------------------------------------------------------------------------
 * private function case_branch()
 * --------------------------------------------------------------------------
 * case :=
 *   caseLabelList ':' statementSequence
 *   ;
 *
 * NB: 'case' is a reserved word in C, we use case_branch() here instead
 *
 * astnode: (CASE caseLabelListNode statementSeqNode)
 * ----------------------------------------------------------------------- */

m2c_token_t case_branch (m2c_parser_context_t p) {
  m2c_astnode_t cllist, stmtseq;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("case");
  
  /* caseLabelList */
  lookahead = case_label_list(p);
  cllist = p->ast;
  
  /* ':' */
  if (match_token(p, TOKEN_COLON, FOLLOW(CASE))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* check for empty statement sequence */
    if ((m2c_tokenset_element(FOLLOW(CASE), lookahead))) {
  
        /* empty statement sequence warning */
        m2c_emit_warning_w_pos
          (M2C_EMPTY_STMT_SEQ,
           m2c_lexer_lookahead_line(p->lexer),
           m2c_lexer_lookahead_column(p->lexer));
        p->warning_count++;
    }
    /* statementSequence */
    else if (match_set(p, FIRST(STATEMENT_SEQUENCE), FOLLOW(CASE))) {
      lookahead = statement_sequence(p);
      stmtseq = p->ast;
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_CASE, cllist, stmtseq, NULL);
  
  return lookahead;
} /* end case_branch */


/* --------------------------------------------------------------------------
 * private function loop_statement()
 * --------------------------------------------------------------------------
 * loopStatement :=
 *   LOOP statementSequence END
 *   ;
 *
 * astnode: (LOOP statementSeqNode)
 * ----------------------------------------------------------------------- */

m2c_token_t loop_statement (m2c_parser_context_t p) {
  m2c_astnode_t stmtseq;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("loopStatement");
  
  /* LOOP */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* check for empty statement sequence */
  if (lookahead == TOKEN_END) {

    /* empty statement sequence warning */
    m2c_emit_warning_w_pos
      (M2C_EMPTY_STMT_SEQ,
       m2c_lexer_lookahead_line(p->lexer),
       m2c_lexer_lookahead_column(p->lexer));
    p->warning_count++;
         
    /* END */
    lookahead = m2c_consume_sym(p->lexer);
  }
  /* statementSequence */
  else if (match_set(p, FIRST(STATEMENT_SEQUENCE), FOLLOW(LOOP_STATEMENT))) {
    lookahead = statement_sequence(p);
    stmtseq = p->ast;
    
    /* END */
    if (match_token(p, TOKEN_END, FOLLOW(LOOP_STATEMENT))) {
      lookahead = m2c_consume_sym(p->lexer);
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_LOOP, stmtseq, NULL);
  
  return lookahead;
} /* end loop_statement */


/* --------------------------------------------------------------------------
 * private function while_statement()
 * --------------------------------------------------------------------------
 * whileStatement :=
 *   WHILE boolExpression DO statementSequence END
 *   ;
 *
 * boolExpression := expression ;
 *
 * astnode: (WHILE exprNode statementSeqNode)
 * ----------------------------------------------------------------------- */

m2c_token_t while_statement (m2c_parser_context_t p) {
  m2c_astnode_t expr, stmtseq;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("whileStatement");
  
  /* WHILE */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* boolExpression */
  if (match_set(p, FIRST(EXPRESSION), FOLLOW(WHILE_STATEMENT))) {
    lookahead = expression(p);
    expr = p->ast;
    
    /* DO */
    if (match_token(p, TOKEN_DO, FOLLOW(WHILE_STATEMENT))) {
      lookahead = m2c_consume_sym(p->lexer);
      
      /* check for empty statement sequence */
      if (lookahead == TOKEN_END) {

        /* empty statement sequence warning */
        m2c_emit_warning_w_pos
          (M2C_EMPTY_STMT_SEQ,
           m2c_lexer_lookahead_line(p->lexer),
           m2c_lexer_lookahead_column(p->lexer));
        p->warning_count++;
         
        /* END */
        lookahead = m2c_consume_sym(p->lexer);
      }
      /* statementSequence */
      else if
        (match_set(p, FIRST(STATEMENT_SEQUENCE), FOLLOW(WHILE_STATEMENT))) {
        lookahead = statement_sequence(p);
        stmtseq = p->ast;
    
        /* END */
        if (match_token(p, TOKEN_END, FOLLOW(WHILE_STATEMENT))) {
          lookahead = m2c_consume_sym(p->lexer);
        } /* end if */
      } /* end if */
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_WHILE, expr, stmtseq, NULL);
  
  return lookahead;
} /* end while_statement */


/* --------------------------------------------------------------------------
 * private function repeat_statement()
 * --------------------------------------------------------------------------
 * repeatStatement :=
 *   REPEAT statementSequence UNTIL boolExpression
 *   ;
 *
 * boolExpression := expression ;
 *
 * astnode: (REPEAT statementSeqNode exprNode)
 * ----------------------------------------------------------------------- */

m2c_token_t repeat_statement (m2c_parser_context_t p) {
  m2c_astnode_t stmtseq, expr;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("repeatStatement");
  
  /* REPEAT */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* check for empty statement sequence */
  if (lookahead == TOKEN_UNTIL) {
  
    /* empty statement sequence warning */
    m2c_emit_warning_w_pos
      (M2C_EMPTY_STMT_SEQ,
       m2c_lexer_lookahead_line(p->lexer),
       m2c_lexer_lookahead_column(p->lexer));
    p->warning_count++;
  }
  /* statementSequence */
  else if (match_set(p,
           FIRST(STATEMENT_SEQUENCE), FOLLOW(STATEMENT_SEQUENCE))) {
    lookahead = statement_sequence(p);
    stmtseq = p->ast;
  }
  else /* resync */ {
    lookahead = m2c_next_sym(p->lexer);
  } /* end if */
    
  /* UNTIL */
  if (match_token(p, TOKEN_UNTIL, FOLLOW(REPEAT_STATEMENT))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* boolExpression */
    if (match_set(p, FIRST(EXPRESSION), FOLLOW(REPEAT_STATEMENT))) {
      lookahead = expression(p);
      expr = p->ast;
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_REPEAT, stmtseq, expr, NULL);
  
  return lookahead;
} /* end repeat_statement */


/* --------------------------------------------------------------------------
 * private function for_statement()
 * --------------------------------------------------------------------------
 * forStatement :=
 *   FOR forLoopVariant ':=' startValue TO endValue
 *   ( BY stepValue )? DO statementSequence END
 *   ;
 *
 * forLoopVariant := Ident ;
 *
 * startValue, endValue := ordinalExpression ;
 *
 * ordinalExpression := expression
 *
 * stepValue := constExpression
 *
 * astnode: (FORTO identNode exprNode exprNode exprNode statementSeqNode)
 * ----------------------------------------------------------------------- */

m2c_token_t for_statement (m2c_parser_context_t p) {
  m2c_astnode_t id, start, end, step, stmtseq;
  m2c_string_t ident;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("forStatement");
  
  /* FOR */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* forLoopVariant */
  if (match_token(p, TOKEN_IDENTIFIER, RESYNC(FOR_LOOP_BODY))) {
    lookahead = m2c_consume_sym(p->lexer);
    ident = m2c_lexer_current_lexeme(p->lexer);
    id = m2c_ast_new_terminal_node(AST_IDENT, ident);
    
    /* ':=' */
    if (match_token(p, TOKEN_ASSIGN, RESYNC(FOR_LOOP_BODY))) {
      lookahead = m2c_consume_sym(p->lexer);
      
      /* startValue */
      if (match_set(p, FIRST(EXPRESSION), RESYNC(FOR_LOOP_BODY))) {
        lookahead = expression(p);
        start = p->ast;
        
        /* TO */
        if (match_token(p, TOKEN_TO, RESYNC(FOR_LOOP_BODY))) {
          lookahead = m2c_consume_sym(p->lexer);
          
          /* endValue */
          if (match_set(p, FIRST(EXPRESSION), RESYNC(FOR_LOOP_BODY))) {
            lookahead = expression(p);
            end = p->ast;
            
            /* ( BY stepValue )? */
            if (lookahead == TOKEN_BY) {
              lookahead = m2c_consume_sym(p->lexer);
              
              if (match_set(p, FIRST(EXPRESSION), RESYNC(FOR_LOOP_BODY))) {
                lookahead = const_expression(p);
                step = p->ast;
              } /* end if */
            }
            else /* no step value */ {
              step = m2c_ast_empty_node();
            } /* end if */
          } /* end if */
        } /* end if */
      } /* end if */
    } /* end if */
  } /* end if */
  
  /* resync */
  lookahead = m2c_next_sym(p->lexer);
  
  /* DO -- The FOR loop body */
  if (match_token(p, TOKEN_DO, FOLLOW(FOR_STATEMENT))) {
    lookahead = m2c_consume_sym(p->lexer);
    
    /* check for empty statement sequence */
    if (lookahead == TOKEN_END) {

      /* empty statement sequence warning */
      m2c_emit_warning_w_pos
        (M2C_EMPTY_STMT_SEQ,
         m2c_lexer_lookahead_line(p->lexer),
         m2c_lexer_lookahead_column(p->lexer));
      p->warning_count++;
       
      /* END */
      lookahead = m2c_consume_sym(p->lexer);
    }
    /* statementSequence */
    else if
      (match_set(p, FIRST(STATEMENT_SEQUENCE), FOLLOW(FOR_STATEMENT))) {
      lookahead = statement_sequence(p);
      stmtseq = p->ast;
  
      /* END */
      if (match_token(p, TOKEN_END, FOLLOW(FOR_STATEMENT))) {
        lookahead = m2c_consume_sym(p->lexer);
      } /* end if */
    } /* end if */
  } /* end if */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_node(AST_FORTO, id, start, end, step, stmtseq, NULL);
  
  return lookahead;
} /* end for_statement */


/* --------------------------------------------------------------------------
 * private function designator()
 * --------------------------------------------------------------------------
 * designator :=
 *   qualident ( '^' | selector )*
 *   ;
 *
 * astnode: identNode | (DEREF expr) | (DESIG headNode tailNode)
 * ----------------------------------------------------------------------- */

m2c_token_t selector (m2c_parser_context_t p);

m2c_token_t designator (m2c_parser_context_t p) {
  m2c_astnode_t head;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("designator");
  
  /* qualident */
  lookahead = qualident(p);
  /* astnode: (IDENT ident) | (QUALIDENT q0 q1 q2 ... qN ident) */
  
  /* ( '^' | selector )* */
  while ((lookahead == TOKEN_DEREF) ||
         (lookahead == TOKEN_PERIOD) ||
         (lookahead == TOKEN_LEFT_BRACKET)) {
    head = p->ast;
    
    if /* '^' */ (lookahead == TOKEN_DEREF) {
      lookahead = m2c_consume_sym(p->lexer);
      p->ast = m2c_ast_new_node(AST_DEREF, head, NULL);
      /* astnode: (DEREF expr) */
    }
    else /* selector */ {
      lookahead = selector(p);
      p->ast = m2c_ast_new_node(AST_DESIG, head, p->ast, NULL);
      /* astnode: (DESIG headNode tailNode) */
    } /* end if */
  } /* end if */
  
  return lookahead;
} /* end designator */


/* --------------------------------------------------------------------------
 * private function selector()
 * --------------------------------------------------------------------------
 * selector :=
 *   '.' Ident | '[' indexList ']'
 *   ;
 *
 * astnode: (FIELD identNode) | (INDEX exprNode+)
 * ----------------------------------------------------------------------- */

m2c_token_t index_list (m2c_parser_context_t p);

m2c_token_t selector (m2c_parser_context_t p) {
  m2c_string_t ident;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("selector");
  
  lookahead = m2c_next_sym(p->lexer);
      
  switch (lookahead) {
    
    /* '.' Ident */
    case TOKEN_PERIOD :
      /* '.' */
      lookahead = m2c_consume_sym(p->lexer);
      
      /* Ident */
      if (match_token(p, TOKEN_IDENTIFIER, FOLLOW(SELECTOR))) {
        lookahead = m2c_consume_sym(p->lexer);
        ident = m2c_lexer_current_lexeme(p->lexer);
        id = m2c_ast_new_terminal_node(AST_IDENT, ident);
        p->ast = m2c_ast_new_node(AST_FIELD, id, NULL);
        /* astnode: (FIELD identNode) */
      } /* end if */
      break;
      
    /* '[' */
    case TOKEN_LEFT_BRACKET :
      /* '[' */
      lookahead = m2c_consume_sym(p->lexer);
      
      /* indexList ']' */
      if (match_set(p, FIRST(EXPRESSION), FOLLOW(SELECTOR))) {
      
        /* indexList */
        lookahead = index_list(p);
        /* astnode: (INDEX exprNode+) */
        
        /* ']' */
        if (match_token(p, TOKEN_RIGHT_BRACKET, FOLLOW(SELECTOR))) {
          lookahead = m2c_consume_sym(p->lexer);
        } /* end if */
      } /* end if */
      break;
      
    default : /* unreachable code */
      /* fatal error -- abort */
      exit(-1);
  } /* end switch */
      
  return lookahead;
} /* end selector */


/* --------------------------------------------------------------------------
 * private function index_list()
 * --------------------------------------------------------------------------
 * indexList :=
 *   expression ( ',' expression )*
 *   ;
 *
 * astnode: (INDEX exprNode+)
 * ----------------------------------------------------------------------- */

m2c_token_t index_list (m2c_parser_context_t p) {
  m2c_fifo_t tmplist;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("expressionList");
  
  /* expression */
  lookahead = expression(p);
  tmplist = m2c_fifo_new_queue(p->ast);
  
  /* ( ',' expression )* */
  while (lookahead == TOKEN_COMMA) {
    /* ',' */
    lookahead = m2c_consume_sym(p->lexer);
    
    /* expression */
    if (match_set(p, FIRST(EXPRESSION), FOLLOW(EXPRESSION))) {
      lookahead = expression(p);
      m2c_fifo_enqueue(tmplist, p->ast);
    } /* end if */
  } /* end while */
  
  /* build AST node and pass it back in p->ast */
  p->ast = m2c_ast_new_list_node(AST_INDEX, tmplist);
  m2c_fifo_release(tmplist);
  
  return lookahead;
} /* end index_list */


/* --------------------------------------------------------------------------
 * private function expression()
 * --------------------------------------------------------------------------
 * expression :=
 *   simpleExpression ( operL1 simpleExpression )?
 *   ;
 *
 * operL1 := '=' | '#' | '<' | '<=' | '>' | '>=' | IN ;
 *
 * astnode:
 *  (EQ expr expr) | (NEQ expr expr) | (LT expr expr) | (LTEQ expr expr) |
 *  (GT expr expr) | (GTEQ expr expr) | (IN expr expr) | simpleExprNode
 * ----------------------------------------------------------------------- */

#define IS_LEVEL1_OPERATOR(_t) \
  (((_t) == TOKEN_EQUAL) || ((_t) == TOKEN_NOTEQUAL) || \
   ((_t) == TOKEN_LESS) || ((_t) == TOKEN_LESS_EQUAL) || \
   ((_t) == TOKEN_GREATER) || ((_t) == TOKEN_GREATER_EQUAL) || \
   ((_t) == TOKEN_IN))

m2c_token_t simple_expression (m2c_parser_context_t p);

m2c_token_t expression (m2c_parser_context_t p) {
  m2c_ast_nodetype_t nodetype;
  m2c_astnode_t left;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("expression");
  
  /* simpleExpression */
  lookahead = simple_expression(p);
  
  /* ( operL1 simpleExpression )? */
  if (IS_LEVEL1_OPERATOR(lookahead)) {
    left = p->ast;
    
    switch (lookahead) {
      case TOKEN_IN :
        nodetype = AST_IN;
        break;
      
      case TOKEN_EQUAL :
        nodetype = AST_EQ;
        break;
      
      case TOKEN_NOTEQUAL :
        nodetype = AST_NEQ;
        break;
      
      case TOKEN_LESS :
        nodetype = AST_LT;
        break;
      
      case TOKEN_LESS_EQUAL :
        nodetype = AST_LTEQ;
        break;
      
      case TOKEN_GREATER :
        nodetype = AST_GT;
        break;
      
      case TOKEN_GREATER_EQUAL :
        nodetype = AST_GTEQ;
        break;
    } /* end switch */
    
    lookahead = m2c_consume_sym(p->lexer);
    
    /* simpleExpression */
    if (match_set(p, FIRST(EXPRESSION), FOLLOW(SIMPLE_EXPRESSION))) {
      lookahead = simple_expression(p);
      p->ast = m2c_ast_new_node(nodetype, left, p->ast, NULL);
    } /* end if */
  } /* end if */
  
  return lookahead;
} /* end expression */


/* --------------------------------------------------------------------------
 * private function simple_expression()
 * --------------------------------------------------------------------------
 * simpleExpression :=
 *   ( '+' | '-' )? term ( operL2 term )*
 *   ;
 *
 * operL2 := '+' | '-' | OR ;
 *
 * astnode:
 *  (NEG expr) |
 *  (PLUS expr expr) | (MINUS expr expr) | (OR expr expr) | termNode
 * ----------------------------------------------------------------------- */

#define IS_LEVEL2_OPERATOR(_t) \
  (((_t) == TOKEN_PLUS) || ((_t) == TOKEN_MINUS) || ((_t) == TOKEN_OR))

m2c_token_t term (m2c_parser_context_t p);

m2c_token_t simple_expression (m2c_parser_context_t p) {
  m2c_ast_nodetype_t nodetype;
  m2c_astnode_t left;
  m2c_token_t lookahead;
  bool unary_minus = false;
  
  PARSER_DEBUG_INFO("simpleExpression");
  
  lookahead = m2c_next_sym(p->lexer);
  
  /* ( '+' | '-' )? */
  if (lookahead == TOKEN_PLUS) {
    lookahead = m2c_consume_sym(p->lexer);
  }
  else if (lookahead == TOKEN_MINUS) {
    lookahead = m2c_consume_sym(p->lexer);
    unary_minus = true;
  } /* end if */
  
  /* term */
  if (match_set(p, FIRST(TERM), FOLLOW(TERM))) {
    lookahead = term(p);
    
    if (unary_minus) {
      p->ast = m2c_ast_new_node(AST_NEG, p->ast, NULL);
    } /* end if */
  
    /* ( operL2 term )* */
    while (IS_LEVEL2_OPERATOR(lookahead)) {
      left = p->ast;
      
      /* operL2 */
      switch (lookahead) {
        case TOKEN_OR :
          nodetype = AST_OR;
          break;
      
        case TOKEN_PLUS :
          nodetype = AST_PLUS;
          break;
      
        case TOKEN_MINUS :
          nodetype = AST_MINUS;
          break;
      } /* end switch */
      
      lookahead = m2c_consume_sym(p->lexer);
    
      /* term */
      if (match_set(p, FIRST(TERM), FOLLOW(TERM))) {
        lookahead = term(p);        
        p->ast = m2c_ast_new_node(nodetype, left, p->ast, NULL);
      } /* end if */
    } /* end while */
  } /* end if */
    
  return lookahead;
} /* end simple_expression */


/* --------------------------------------------------------------------------
 * private function term()
 * --------------------------------------------------------------------------
 * term :=
 *   simpleTerm ( operL3 simpleTerm )*
 *   ;
 *
 * operL3 := '*' | '/' | DIV | MOD | AND ;
 *
 * astnode:
 *  (ASTERISK expr expr) | (SOLIDUS expr expr) |
 *  (DIV expr expr) | (MOD expr expr) | (AND expr expr) | simpleTermNode
 * ----------------------------------------------------------------------- */

#define IS_LEVEL3_OPERATOR(_t) \
  (((_t) == TOKEN_ASTERISK) || ((_t) == TOKEN_SOLIDUS) || \
   ((_t) == TOKEN_DIV) || ((_t) == TOKEN_MOD) || ((_t) == TOKEN_AND))

m2c_token_t simple_term (m2c_parser_context_t p);

m2c_token_t term (m2c_parser_context_t p) {
  m2c_ast_nodetype_t operation;
  m2c_astnode_t left, right;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("term");
  
  /* simpleTerm */
  lookahead = simple_term(p);
  left = p->ast;
  
  /* ( operL3 simpleTerm )* */
  while (IS_LEVEL3_OPERATOR(lookahead)) {
    left = p->ast;
    
    /* operL3 */
    switch (lookahead) {
      case TOKEN_AND :
        nodetype = AST_AND;
        break;
    
      case TOKEN_DIV :
        nodetype = AST_DIV;
        break;
    
      case TOKEN_MOD :
        nodetype = AST_MOD;
        break;
    
      case TOKEN_ASTERISK :
        nodetype = AST_ASTERISK;
        break;
    
      case TOKEN_SOLIDUS :
        nodetype = AST_SOLIDUS;
        break;
    } /* end switch */
      
    lookahead = m2c_consume_sym(p->lexer);
    
    /* simpleTerm */
    if (match_set(p, FIRST(SIMPLE_TERM), FOLLOW(SIMPLE_TERM))) {
      lookahead = simple_term(p);      
      p->ast = m2c_ast_new_node(nodetype, left, p->ast, NULL);
    } /* end if */
  } /* end while */
  
  return lookahead;
} /* end term */


/* --------------------------------------------------------------------------
 * private function simple_term()
 * --------------------------------------------------------------------------
 * simpleTerm :=
 *   NOT? factor
 *   ;
 *
 * astnode: (NOT expr) | factorNode
 * ----------------------------------------------------------------------- */

m2c_token_t factor (m2c_parser_context_t p);

m2c_token_t simple_term (m2c_parser_context_t p) {
  m2c_token_t lookahead;
  bool negation = false;
  
  PARSER_DEBUG_INFO("simpleTerm");
  
  lookahead = m2c_next_sym(p->lexer);
  
  /* NOT? */
  if (lookahead == TOKEN_NOT) {
    lookahead = m2c_consume_sym(p->lexer);
    negation = true;
  } /* end if */
  
  /* factor */
  if (match_set(p, FIRST(FACTOR), FOLLOW(FACTOR))) {
    lookahead = factor(p);
    
    if (negation) {
      p->ast = m2c_ast_new_node(AST_NOT, p->ast, NULL);
    } /* end if */
  } /* end if */

  return lookahead;
} /* end simple_term */


/* --------------------------------------------------------------------------
 * private function factor()
 * --------------------------------------------------------------------------
 * factor :=
 *   NumberLiteral | StringLiteral | setValue |
 *   designatorOrFuncCall | '(' expression ')'
 *   ;
 *
 * astnode:
 *  (INTVAL value) | (REALVAL value) | (CHRVAL value) | (QUOTEDVAL value) |
 *  setValNode | designatorNode | funcCallNode | exprNode
 * ----------------------------------------------------------------------- */

m2c_token_t set_value (m2c_parser_context_t p);

m2c_token_t designator_or_func_call (m2c_parser_context_t p);

m2c_token_t factor (m2c_parser_context_t p) {
  m2c_string_t lexeme;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("factor");
  
  lookahead = m2c_next_sym(p->lexer);
  
  switch (lookahead) {
  
    /* NumberLiteral */
    case TOKEN_INTEGER :
      lookahead = m2c_consume_sym(p->lexer);
      lexeme = m2c_lexer_current_lexeme(p->lexer);
      p->ast = m2c_ast_new_terminal_node(AST_INTVAL, lexeme);
      break;
      
    case TOKEN_REAL :
      lookahead = m2c_consume_sym(p->lexer);
      lexeme = m2c_lexer_current_lexeme(p->lexer);
      p->ast = m2c_ast_new_terminal_node(AST_REALVAL, lexeme);
      break;
      
    case TOKEN_CHAR :
      lookahead = m2c_consume_sym(p->lexer);
      lexeme = m2c_lexer_current_lexeme(p->lexer);
      p->ast = m2c_ast_new_terminal_node(AST_CHRVAL, lexeme);
      break;
          
    /* | StringLiteral */
    case TOKEN_STRING :
      lookahead = m2c_consume_sym(p->lexer);
      lexeme = m2c_lexer_current_lexeme(p->lexer);
      p->ast = m2c_ast_new_terminal_node(AST_QUOTEDVAL, lexeme);
      break;
      
    /* | setValue */
    case TOKEN_LEFT_BRACE :
      lookahead = set_value(p);
      break;
      
    /* | designatorOrFuncCall */
    case TOKEN_IDENTIFIER :
      lookahead = designator_or_func_call(p);
      break;
      
    /* | '(' expression ')' */
    case TOKEN_LEFT_PAREN :
      lookahead = m2c_consume_sym(p->lexer);
      
      /* expression */
      if (match_set(p, FIRST(EXPRESSION), FOLLOW(FACTOR))) {
        lookahead = expression(p);
        
        /* ')' */
        if (match_token(p, TOKEN_RIGHT_PAREN, FOLLOW(FACTOR))) {
          lookahead = m2c_consume_sym(p->lexer);
        } /* end if */
      } /* end if */
      break;
      
    default : /* unreachable code */
      /* fatal error -- abort */
      exit(-1);
  } /* end switch */
  
  return lookahead;
} /* end factor */


/* --------------------------------------------------------------------------
 * private function designator_or_func_call()
 * --------------------------------------------------------------------------
 * designatorOrFuncCall :=
 *   designator ( setValue | actualParameters )?
 *   ;
 *
 * astnode:
 *  (SETVAL designatorNode elemListNode) | (FCALL designatorNode argsNode)
 * ----------------------------------------------------------------------- */

m2c_token_t designator_or_func_call (m2c_parser_context_t p) {
  m2c_astnode_t desig;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("designatorOrFuncCall");
  
  /* designator */
  lookahead = designator(p);
  desig = p->ast;
  
  /* setValue */
  if (lookahead == TOKEN_LEFT_BRACE) {
    lookahead = set_value(p);
    /* TO DO: check designator for IDENT or QUALIDENT */
    p->ast = m2c_ast_replace_subnode(p->ast, 0, desig);
    /* astnode: (SETVAL designatorNode elemListNode) */
  }
  /* actualParameters */
  else if (lookahead == TOKEN_LEFT_PAREN) {
    lookahead = actual_parameters(p);
    p->ast = m2c_ast_new_node(AST_FCALL, desig, p->ast, NULL);
    /* astnode: (FCALL designatorNode argsNode) */
    
    if (match_token(p, TOKEN_RIGHT_PAREN, FOLLOW(DESIGNATOR_OR_FUNC_CALL))) {
      lookahead = m2c_consume_sym(p->lexer);
    } /* end if */
  } /* end if */
    
  return lookahead;
} /* end designator_or_func_call */


/* --------------------------------------------------------------------------
 * private function set_value()
 * --------------------------------------------------------------------------
 * setValue :=
 *   '{' element ( ',' element )* '}'
 *   ;
 *
 * astnode: (SETVAL (EMPTY) elemListNode)
 * ----------------------------------------------------------------------- */

m2c_token_t element (m2c_parser_context_t p);

m2c_token_t set_value (m2c_parser_context_t p) {
  m2c_astnode_t empty, elemlist;
  m2c_ast_fifo_t tmplist;
  m2c_token_t lookahead;
  
  PARSER_DEBUG_INFO("setValue");
  
  /* '{' */
  lookahead = m2c_consume_sym(p->lexer);
  
  /* element */
  if (match_set(p, FIRST(ELEMENT), FOLLOW(SET_VALUE))) {
    lookahead = element(p);
    tmplist = m2c_fifo_new_queue(p->ast);
    
    /* ( ',' element )* */
    while (lookahead == TOKEN_COMMA) {
      /* ',' */
      lookahead = m2c_consume_sym(p->lexer);
    
      /* element */
      if (match_set(p, FIRST(ELEMENT), FOLLOW(SET_VALUE))) {
        lookahead = element(p);
        m2c_fifo_enqueue(tmplist, p->ast);
      }
      else /* resync */ {
        lookahead = m2c_next_sym(p->lexer);
      } /* end if */
    } /* end while */
    
    /* '}' */
    if (match_token(p, TOKEN_RIGHT_BRACE, FOLLOW(SET_VALUE))) {
      lookahead = m2c_consume_sym(p->lexer);
    } /* end if */
  } /* end if */
  
  if (m2c_fifo_entry_count(tmplist) > 0) {
    elemlist = m2c_ast_new_list_node(AST_ELEMLIST, tmplist);
  }
  else /* empty set */ {
    elemlist = m2c_ast_empty_node();
  } /* end if */
  
  m2c_fifo_release(tmplist);
  
  /* build AST node and pass it back in p->ast */
  empty = m2c_ast_empty_node();
  p->ast = m2c_ast_new_node(AST_SETVAL, empty, elemlist, NULL);
  
  return lookahead;
} /* end set_value */


/* --------------------------------------------------------------------------
 * private function element()
 * --------------------------------------------------------------------------
 * element :=
 *   expression ( '..' expression )?
 *   ;
 *
 * astnode: exprNode | (RANGE expr expr)
 * ----------------------------------------------------------------------- */

m2c_token_t element (m2c_parser_context_t p) {
  m2c_astnode_t lower;
  m2c_token_t lookahead;
    
  PARSER_DEBUG_INFO("element");
  
  /* expression */
  lookahead = expression(p);
  
  /* ( '..' expression )? */
  if (lookahead == TOKEN_RANGE) {
    lower = p->ast;
    lookahead = m2c_consume_sym(p->lexer);
    
    /* expression */
    if (match_set(p, FIRST(EXPRESSION), FOLLOW(ELEMENT))) {
      lookahead = expression(p);
      p->ast = m2c_ast_new_node(AST_RANGE, lower, p->ast, NULL);
    } /* end if */
  } /* end if */
  
  return lookahead;
} /* end element */

/* END OF FILE */