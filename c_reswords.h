/* C Reserved Word Matching Library
 * Copyright (c) 2015 Benjamin Kowarsch
 *
 * @file
 *
 * c_reswords.h
 *
 * Public interface of C reserved word matching library.
 *
 * @license
 *
 * This library is free software: you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) either ver 2.1
 * or at your choice ver 3 as published by the Free Software Foundation.
 *
 * It is distributed in the hope that it will be useful,  but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  Read the license for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * with the library.  If not, see <https://www.gnu.org/copyleft/lesser.html>.
 */

#ifndef C_RESWORDS_H
#define C_RESWORDS_H

#include <stdbool.h>


/* --------------------------------------------------------------------------
 * compile time parameter C_RESWORDS_INCLUDE_PSEUDO_RESWORDS
 * --------------------------------------------------------------------------
 * If set to 0, function is_c_reswords tests for reserved words only.
 * Otherwise, function is_c_reswords also tests for pseudo reserved words.
 * ----------------------------------------------------------------------- */

#define C_RESWORDS_INCLUDE_PSEUDO_RESWORDS 1


/* --------------------------------------------------------------------------
 * function is_c_resword(cstr)
 * --------------------------------------------------------------------------
 * Tests if cstr matches reserved words and pseudo reserved words depending
 * on macro C_RESWORDS_INCLUDE_PSEUDO_RESWORDS.  Returns true in case of a
 * match, otherwise false.
 *
 * Only reserved words are tested if C_RESWORDS_INCLUDE_PSEUDO_RESWORDS is 0,
 * otherwise both reserved words and pseudo reserved words are tested.
 *
 * Reserved words:
 *   auto, break, case, char, const, continue, default, do, double, else,
 *   enum, extern, float, for, goto, if, inline, int, long, register,
 *   restrict, return, short, signed, sizeof, static, struct, switch,
 *   typedef, union, unsigned, void, volatile, while;
 *
 * Pseudo reserved words:
 *   NULL, alignas, alignof,  bool, complex, exit, false, free, imaginary,
 *   main, malloc, noreturn; 
 * ----------------------------------------------------------------------- */

bool is_c_resword (const char *cstr);


#endif /* C_RESWORDS_H */

/* END OF FILE */