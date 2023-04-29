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
 * m2-common.h
 *
 * Common definitions.
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

#ifndef M2C_COMMON_H
#define M2C_COMMON_H

/* --------------------------------------------------------------------------
 * Import integer types
 * ----------------------------------------------------------------------- */

#if defined(VMS) || defined(__VMS)
#include <inttypes.h>
#else
#include <stdint.h>
#endif


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

typedef const char *char_ptr_t;

typedef void *address_t;


/* --------------------------------------------------------------------------
 * Common M2 types
 * ----------------------------------------------------------------------- */

#define m2c_byte_t m2c_octet_t

#define m2c_address_t address_t

typedef uint8_t m2c_octet_t;

typedef unsigned char m2c_char_t;

typedef unsigned int m2c_cardinal_t;

typedef long unsigned int m2c_longcard_t;

typedef int m2c_integer_t;

typedef long int m2c_longint_t;

typedef float m2c_real_t;

typedef double m2c_longreal_t;


/* ==========================================================================
 * Verify build parameters
 * ======================================================================= */

#include "m2-build-params.h"


/* --------------------------------------------------------------------------
 * Verify M2C_COROUTINES_IMPLEMENTED
 * ----------------------------------------------------------------------- */

#if !defined(M2C_COROUTINES_IMPLEMENTED)
#error "no value defined for M2C_COROUTINES_IMPLEMENTED"
#endif


/* --------------------------------------------------------------------------
 * Verify M2C_LOCAL_MODULES_IMPLEMENTED
 * ----------------------------------------------------------------------- */

#if !defined(M2C_LOCAL_MODULES_IMPLEMENTED)
#error "no value defined for M2C_LOCAL_MODULES_IMPLEMENTED"
#endif


#endif /* M2C_COMMON_H */

/* END OF FILE */