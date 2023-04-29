/* C Reserved Word Matching Library
 * Copyright (c) 2015 Benjamin Kowarsch
 *
 * @file
 *
 * c_reswords.c
 *
 * Implementation of C reserved word matching library.
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

#include "c_reswords.h"
#include "cstring.h"


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
 *   alignas, alignof,  bool, complex, exit, false, free, imaginary, main,
 *   malloc, noreturn; 
 * ----------------------------------------------------------------------- */

bool is_c_resword (const char *cstr) {
  
  unsigned length = cstr_length(cstr);
  
  switch (length) {
    case 0 :
    case 1 :
      return false;
    
    /* length 2 */ 
    case 2 :
      switch (cstr[0]) {
        /* do */
        case 'd' :
          return (cstr[1] == 'o');
          
        /* if */
        case 'i' :
          return (cstr[1] == 'f');
          
        default :
          return false;
      } /* end switch */
    
    /* length 3 */ 
    case 3 :
      switch (cstr[0]) {
        /* for */
        case 'f' :
          return (cstr[1] == 'o') && (cstr[2] == 'r');
          
        /* int */
        case 'i' :
          return (cstr[1] == 'n') && (cstr[2] == 't');
          
        default :
          return false;
      } /* end switch */
    
    /* length 4 */ 
    case 4 :
      switch (cstr[0]) {
#if C_RESWORDS_INCLUDE_PSEUDO_RESWORDS
        /* NULL */
        case 'N' :
          return (cstr[1] == 'U') && (cstr[2] == 'L') && (cstr[3] == 'L');
#endif /* C_RESWORDS_INCLUDE_PSEUDO_RESWORDS */
          
        /* auto */
        case 'a' :
          return (cstr[1] == 'u') && (cstr[2] == 't') && (cstr[3] == 'o');
          
#if C_RESWORDS_INCLUDE_PSEUDO_RESWORDS
        /* bool */
        case 'b' :
          return (cstr[1] == 'o') && (cstr[2] == 'o') && (cstr[3] == 'l');
#endif /* C_RESWORDS_INCLUDE_PSEUDO_RESWORDS */
          
        /* case, char */
        case 'c' :
          switch (cstr[1]) {
            /* case */
            case 'a' :
              return (cstr[2] == 's') && (cstr[3] == 'e');
          
            /* char */
            case 'h' :
              return (cstr[2] == 'a') && (cstr[3] == 'r');
          
            default :
              return false;
          } /* end switch */
          
        /* else, enum, exit */
        case 'e' :
          switch (cstr[1]) {
            /* else */
            case 'l' :
              return (cstr[2] == 's') && (cstr[3] == 'e');
          
            /* enum */
            case 'n' :
              return (cstr[2] == 'u') && (cstr[3] == 'm');
          
#if C_RESWORDS_INCLUDE_PSEUDO_RESWORDS
            /* exit */
            case 'x' :
              return (cstr[2] == 'i') && (cstr[3] == 't');
#endif /* C_RESWORDS_INCLUDE_PSEUDO_RESWORDS */
            
            default :
              return false;
          } /* end switch */
          
#if C_RESWORDS_INCLUDE_PSEUDO_RESWORDS
        /* free */
        case 'f' :
          return (cstr[1] == 'r') && (cstr[2] == 'e') && (cstr[3] == 'e');
#endif /* C_RESWORDS_INCLUDE_PSEUDO_RESWORDS */
        
        /* goto */
        case 'g' :
          return (cstr[1] == 'o') && (cstr[2] == 't') && (cstr[3] == 'o');
        
        /* long */
        case 'l' :
          return (cstr[1] == 'o') && (cstr[2] == 'n') && (cstr[3] == 'g');
        
#if C_RESWORDS_INCLUDE_PSEUDO_RESWORDS
        /* main */
        case 'm' :
          return (cstr[1] == 'a') && (cstr[2] == 'i') && (cstr[3] == 'n');
        
        /* true */
        case 't' :
          return (cstr[1] == 'r') && (cstr[2] == 'u') && (cstr[3] == 'e');
#endif /* C_RESWORDS_INCLUDE_PSEUDO_RESWORDS */
        
        /* void */
        case 'v' :
          return (cstr[1] == 'o') && (cstr[2] == 'i') && (cstr[3] == 'd');
      
        default :
          return false;
      } /* end switch */
    
    /* length 5 */ 
    case 5 :
      switch (cstr[0]) {
        /* break */
        case 'b' :
          return cstr_match(cstr, "break");
          
        /* const */
        case 'c' :
          return cstr_match(cstr, "const");
        
        /* false */
        case 'f' :
          switch (cstr[1]) {
#if C_RESWORDS_INCLUDE_PSEUDO_RESWORDS
            /* false */
            case 'a' :
              return cstr_match(cstr, "false");
#endif /* C_RESWORDS_INCLUDE_PSEUDO_RESWORDS */
          
            /* float */
            case 'l' :
              return cstr_match(cstr, "float");
          
            default :
              return false;
          } /* end switch */
        
        /* short */
        case 's' :
          return cstr_match(cstr, "short");
        
        /* union */
        case 'u' :
          return cstr_match(cstr, "union");
        
        /* while */
        case 'w' :
          return cstr_match(cstr, "while");
        
        default :
          return false;
      } /* end switch */
      
    /* length 6 */ 
    case 6 :
      switch (cstr[0]) {
        /* double */
        case 'd' :
          return cstr_match(cstr, "double");
        
        /* extern */
        case 'e' :
          return cstr_match(cstr, "extern");
        
        /* inline */
        case 'i' :
          return cstr_match(cstr, "inline");
        
#if C_RESWORDS_INCLUDE_PSEUDO_RESWORDS
        /* malloc */
        case 'm' :
          return cstr_match(cstr, "malloc");
#endif /* C_RESWORDS_INCLUDE_PSEUDO_RESWORDS */
        
        /* return */
        case 'r' :
          return cstr_match(cstr, "return");
        
        case 's' :
          switch (cstr[2]) {
            /* signed */
            case 'g' :
              return cstr_match(cstr, "signed");
            
            /* sizeof */
            case 'z' :
              return cstr_match(cstr, "sizeof");
            
            /* static */
            case 'a' :
              return cstr_match(cstr, "static");
            
            /* struct */
            case 'r' :
              return cstr_match(cstr, "struct");
            
            /* switch */
            case 'i' :
              return cstr_match(cstr, "switch");
            
            default :
              return false;
          } /* end switch */
        
        default :
          return false;
      } /* end switch */
    
    /* length 7 */ 
    case 7 :
      switch (cstr[0]) {
#if C_RESWORDS_INCLUDE_PSEUDO_RESWORDS
        case 'a' :
          /* alignas */
          if (cstr[6] == 's') {
            return cstr_match(cstr, "alignas");
          }
          /* alignof */
          else if (cstr[6] == 'f') {
            return cstr_match(cstr, "alignof");
          }
          else {
            return false;
          } /* end if */
          
        /* complex */
        case 'c' :
          return cstr_match(cstr, "complex");
#endif /* C_RESWORDS_INCLUDE_PSEUDO_RESWORDS */
        
        /* default */
        case 'd' :
          return cstr_match(cstr, "default");
        
        /* typedef */
        case 't' :
          return cstr_match(cstr, "typedef");
        
        default :
          return false;
      } /* end switch */
    
    /* length 8 */ 
    case 8 :
      switch (cstr[4]) {
        /* continue */
        case 'i' :
          return cstr_match(cstr, "continue");
        
#if C_RESWORDS_INCLUDE_PSEUDO_RESWORDS
        /* noreturn */
        case 'e' :
          return cstr_match(cstr, "noreturn");
#endif /* C_RESWORDS_INCLUDE_PSEUDO_RESWORDS */
        
        /* register */
        case 's' :
          return cstr_match(cstr, "register");
        
        /* unsigned */
        case 'g' :
          return cstr_match(cstr, "unsigned");
        
        /* volatile */
        case 't' :
          return cstr_match(cstr, "volatile");
        
        /* restrict */
        case 'r' :
          return cstr_match(cstr, "restrict");
        
        default :
          return false;
      } /* end switch */
    
#if C_RESWORDS_INCLUDE_PSEUDO_RESWORDS
    /* length 9 */ 
    case 9 :
      /* imaginary */
      if (cstr[0] == 'i') {
        return cstr_match(cstr, "imaginary");
      }
      else {
        return false;
      } /* end if */
#endif /* C_RESWORDS_INCLUDE_PSEUDO_RESWORDS */
  
  } /* end switch */
  
  /* anything else */
  return false;
} /* end is_c_resword */


/* END OF FILE */