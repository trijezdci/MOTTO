/* M2T -- Sorce to Source Modula-2 Translator
 *
 * Copyright (c) 2016-2023 Benjamin Kowarsch
 *
 * Author & Maintainer: Benjamin Kowarsch <org.m2sf>
 *
 * @synopsis
 *
 * M2T is a multi-dialect Modula-2 source-to-source translator. It translates
 * source files  written in the  classic dialects  to semantically equivalent
 * source files in  Modula-2 Revision 2010 (M2R10).  It supports  the classic
 * Modula-2 dialects  described in  the 2nd, 3rd and 4th editions  of Niklaus
 * Wirth's book "Programming in Modula-2" (PIM) published by Springer Verlag.
 *
 * For more details please visit: https://github.com/trijezdci/m2t/wiki
 *
 * @repository
 *
 * https://github.com/trijezdci/m2t
 *
 * @file
 *
 * m2t-common.h
 *
 * Common definitions.
 *
 * @license
 *
 * M2T is free software: you can redistribute and/or modify it under the
 * terms of the GNU Lesser General Public License (LGPL) either version 2.1
 * or at your choice version 3 as published by the Free Software Foundation.
 *
 * M2T is distributed in the hope that it will be useful,  but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  Read the license for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with m2t.  If not, see <https://www.gnu.org/copyleft/lesser.html>.
 */

#ifndef M2T_COMMON_H
#define M2T_COMMON_H

/* --------------------------------------------------------------------------
 * Import integer types
 * ----------------------------------------------------------------------- */

#if defined(VMS) || defined(__VMS)
#include <inttypes.h>
#else
#include <stdint.h>
#endif

/* --------------------------------------------------------------------------
 * Identification
 * ----------------------------------------------------------------------- */

#define M2T_IDENTIFICATION "M2T Modula-2 Translator"

#define M2T_VERSION "1.00"


/* --------------------------------------------------------------------------
 * Common constants
 * ----------------------------------------------------------------------- */

#define ASCII_NUL 0

#define ASCII_EOT 4

#define ASCII_TAB 9

#define ASCII_LF 10

#define ASCII_CR 13

#define ASCII_SPACE 32

#define EMPTY_STRING "\0"


/* --------------------------------------------------------------------------
 * Common macros
 * ----------------------------------------------------------------------- */

#define NOT(_expr) \
  (!(_expr))

#define CAST(_type,_var) \
  ((_type) _var)

#define SET_STATUS(_status_ptr,_value) \
  { if (_status_ptr != NULL) {*_status_ptr = _value; }; }

#define WRITE_OUTPARAM(_outparam_ptr,_value) \
  { if (_outparam_ptr != NULL) {*_outparam_ptr = _value; }; }

#define IS_DIGIT(_ch) \
  (((_ch) >= '0') && ((_ch) <= '9'))

#define IS_A_TO_F(_ch) \
  (((_ch) >= 'A') && ((_ch) <= 'F'))

#define IS_UPPER(_ch) \
  (((_ch) >= 'A') && ((_ch) <= 'Z'))

#define IS_LOWER(_ch) \
  (((_ch) >= 'a') && ((_ch) <= 'z'))

#define IS_LETTER(_ch) \
  (IS_LOWER(_ch) || IS_UPPER(_ch))

#define IS_ALPHANUMERIC(_ch) \
  (IS_LETTER(_ch) || IS_DIGIT(_ch))

#define IS_CONTROL_CHAR(_ch) \
  ((((_ch) >= 0) && ((_ch) <= 0x1f)) || ((_ch) == 0x7f))


/* --------------------------------------------------------------------------
 * Common C types
 * ----------------------------------------------------------------------- */

typedef unsigned int uint_t;

typedef char *char_ptr_t;

typedef void *address_t;


/* --------------------------------------------------------------------------
 * Common M2 types
 * ----------------------------------------------------------------------- */

#define m2t_byte_t m2c_octet_t

#define m2t_address_t address_t

typedef uint8_t m2t_octet_t;

typedef unsigned char m2t_char_t;

typedef unsigned int m2t_cardinal_t;

typedef long unsigned int m2t_longcard_t;

typedef int m2t_integer_t;

typedef long int m2t_longint_t;

typedef float m2t_real_t;

typedef double m2t_longreal_t;


/* ==========================================================================
 * Verify build parameters
 * ======================================================================= */

#include "m2t-build-params.h"

/* --------------------------------------------------------------------------
 * Verify M2T_MAX_IDENT_LENGTH
 * ----------------------------------------------------------------------- */

#if !defined(M2T_MAX_IDENT_LENGTH)
#error "no value defined for M2T_MAX_IDENT_LENGTH"
#endif


/* --------------------------------------------------------------------------
 * Verify M2T_MAX_NUMBER_LENGTH
 * ----------------------------------------------------------------------- */

#if !defined(M2T_MAX_NUMBER_LENGTH)
#error "no value defined for M2T_MAX_NUMBER_LENGTH"
#endif


/* --------------------------------------------------------------------------
 * Verify M2T_MAX_STRING_LENGTH
 * ----------------------------------------------------------------------- */

#if !defined(M2T_MAX_STRING_LENGTH)
#error "no value defined for M2T_MAX_STRING_LENGTH"
#endif


/* --------------------------------------------------------------------------
 * Verify M2T_MAX_COMMENT_LENGTH
 * ----------------------------------------------------------------------- */

#if !defined(M2T_MAX_COMMENT_LENGTH)
#error "no value defined for M2T_MAX_COMMENT_LENGTH"
#endif


/* --------------------------------------------------------------------------
 * Verify M2T_COMMENT_NESTING_LIMIT
 * ----------------------------------------------------------------------- */

#if !defined(M2T_COMMENT_NESTING_LIMIT)
#error "no value defined for M2T_COMMENT_NESTING_LIMIT"
#endif


/* --------------------------------------------------------------------------
 * Verify M2T_COROUTINES_IMPLEMENTED
 * ----------------------------------------------------------------------- */

#if !defined(M2T_COROUTINES_IMPLEMENTED)
#error "no value defined for M2T_COROUTINES_IMPLEMENTED"
#endif


/* --------------------------------------------------------------------------
 * Verify M2C_LOCAL_MODULES_IMPLEMENTED
 * ----------------------------------------------------------------------- */

#if !defined(M2T_LOCAL_MODULES_IMPLEMENTED)
#error "no value defined for M2T_LOCAL_MODULES_IMPLEMENTED"
#endif


/* --------------------------------------------------------------------------
 * Verify M2C_VARIANT_RECORDS_IMPLEMENTED
 * ----------------------------------------------------------------------- */

#if !defined(M2C_VARIANT_RECORDS_IMPLEMENTED)
#error "no value defined for M2C_VARIANT_RECORDS_IMPLEMENTED"
#endif


/* --------------------------------------------------------------------------
 * Verify M2T_COMMENT_PRESRVN_IMPLEMENTED
 * ----------------------------------------------------------------------- */

#if !defined(M2T_COMMENT_PRESRVN_IMPLEMENTED)
#error "no value defined for M2T_COMMENT_PRESRVN_IMPLEMENTED"
#endif


#endif /* M2T_COMMON_H */

/* END OF FILE */