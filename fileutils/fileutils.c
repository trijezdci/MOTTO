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
 * fileutils.c
 *
 * Aggregator to select platform specific implementation.
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


/* --------------------------------------------------------------------------
 * Select implementation for AmigaOS host platform
 * ----------------------------------------------------------------------- */

#if defined(__amigaos__)
#include "fileutils-amiga.c"


/* --------------------------------------------------------------------------
 * Select implementation for POSIX and Unix-like host platforms
 * ----------------------------------------------------------------------- */

#elif (defined(__MACH__)) || (defined(__unix)) || \
      ((defined(__unix__)) && (!defined(_WIN32)))
#include "fileutils-posix.c"


/* --------------------------------------------------------------------------
 * Select implementation for OpenVMS host platforms
 * ----------------------------------------------------------------------- */

#elif defined(VMS) || defined(__VMS)
#include "fileutils-posix.c"


/* --------------------------------------------------------------------------
 * Select implementation for Windows-64 host platforms
 * ----------------------------------------------------------------------- */

#elif (defined(_WIN64))
#include "fileutils-win.c"


/* --------------------------------------------------------------------------
 * Select implementation for Windows-32, MS-DOS and OS/2 host platforms
 * ----------------------------------------------------------------------- */

#elif (defined(_WIN32)) || ((defined(MSDOS)) || defined(OS2))
/* The DJGPP compiler provides a POSIX based standard library */
#if (defined(__DJGPP__))
#include "fileutils-posix.c"
#else /* other compilers */
#include "fileutils-win.c"
#endif /* __DJGPP__ */


/* --------------------------------------------------------------------------
 * Emit error and abort compilation for unsupported host platforms
 * ----------------------------------------------------------------------- */

#else
#error "Unsupported host platform."
#endif


/* END OF FILE */