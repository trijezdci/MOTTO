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
 * m2-pathname-policy.h
 *
 * Pathname policy settings.
 * Required by module m2-pathnames.
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

#ifndef M2C_PATHNAME_POLICY_H
#define M2C_PATHNAME_POLICY_H

/* --------------------------------------------------------------------------
 * All settings are applied at compile time and cannot be changed at runtime!
 * ----------------------------------------------------------------------- */


/* --------------------------------------------------------------------------
 * Use of special characters in pathname components
 * --------------------------------------------------------------------------
 * The following constants define whether certain special characters may or
 * may not occur legally within the basename of a filename or directory:
 *
 * M2C_PATHCOMP_MAY_CONTAIN_PERIOD
 *   enable (1) or disable (0) use of period ('.') in a pathname component.
 *
 *   Restrictions
 *   - a period may not be trailing nor consecutive,
 *   - a period must be escaped with a caret ('^') on OpenVMS.
 *
 * M2C_PATHCOMP_MAY_CONTAIN_SPACE
 *   enable (1) or disable (0) use of space (' ') in a pathname component.
 *
 *   Restrictions
 *   - a space may not be leading nor trailing nor consecutive,
 *   - a space must be escaped with a caret ('^') on OpenVMS.
 *
 * M2C_PATHCOMP_MAY_CONTAIN_MINUS
 *   enable (1) or disable (0) use of minus ('-') in a pathname component.
 *
 *   Restrictions
 *   - a minus may not be leading.
 *
 * M2C_PATHCOMP_MAY_CONTAIN_TILDE
 *   enable (1) or disable (0) use of tilde ('~') in a pathname component.
 *
 *   Restrictions
 *   - a tilde may not be leading.
 * ----------------------------------------------------------------------- */


/* --------------------------------------------------------------------------
 * Settings for MacOS X
 * ----------------------------------------------------------------------- */

#if defined(__APPLE__)
#define M2C_PATHCOMP_MAY_CONTAIN_PERIOD 1
#define M2C_PATHCOMP_MAY_CONTAIN_SPACE 1
#define M2C_PATHCOMP_MAY_CONTAIN_MINUS 1
#define M2C_PATHCOMP_MAY_CONTAIN_TILDE 0


/* --------------------------------------------------------------------------
 * Settings for AmigaOS
 * ----------------------------------------------------------------------- */

#elif defined(__amigaos)
#define M2C_PATHCOMP_MAY_CONTAIN_PERIOD 1
#define M2C_PATHCOMP_MAY_CONTAIN_SPACE 1
#define M2C_PATHCOMP_MAY_CONTAIN_MINUS 1
#define M2C_PATHCOMP_MAY_CONTAIN_TILDE 0


/* --------------------------------------------------------------------------
 * Settings for MS-DOS and OS/2
 * ----------------------------------------------------------------------- */

#elif defined(MSDOS) || defined(OS2)
#define M2C_PATHCOMP_MAY_CONTAIN_PERIOD 0
#define M2C_PATHCOMP_MAY_CONTAIN_SPACE 0
#define M2C_PATHCOMP_MAY_CONTAIN_MINUS 1
#define M2C_PATHCOMP_MAY_CONTAIN_TILDE 1


/* --------------------------------------------------------------------------
 * Settings for OpenVMS
 * ----------------------------------------------------------------------- */

#elif defined(VMS) || defined(__VMS)
#define M2C_PATHCOMP_MAY_CONTAIN_PERIOD 0
#define M2C_PATHCOMP_MAY_CONTAIN_SPACE 0
#define M2C_PATHCOMP_MAY_CONTAIN_MINUS 1
#define M2C_PATHCOMP_MAY_CONTAIN_TILDE 1


/* --------------------------------------------------------------------------
 * Settings for Microsoft Windows
 * ----------------------------------------------------------------------- */

#elif defined(_WIN32) || defined(_WIN64)
#define M2C_PATHCOMP_MAY_CONTAIN_PERIOD 0
#define M2C_PATHCOMP_MAY_CONTAIN_SPACE 1
#define M2C_PATHCOMP_MAY_CONTAIN_MINUS 1
#define M2C_PATHCOMP_MAY_CONTAIN_TILDE 1


/* --------------------------------------------------------------------------
 * Default settings for Unix or Unix-like systems
 * ----------------------------------------------------------------------- */

#elif defined(unix) || defined(__unix__)
#define M2C_PATHCOMP_MAY_CONTAIN_PERIOD 1
#define M2C_PATHCOMP_MAY_CONTAIN_SPACE 0
#define M2C_PATHCOMP_MAY_CONTAIN_MINUS 1
#define M2C_PATHCOMP_MAY_CONTAIN_TILDE 0


/* --------------------------------------------------------------------------
 * Default settings for any other systems
 * ----------------------------------------------------------------------- */

#else /* D E F A U L T */
#define M2C_PATHCOMP_MAY_CONTAIN_PERIOD 0
#define M2C_PATHCOMP_MAY_CONTAIN_SPACE 0
#define M2C_PATHCOMP_MAY_CONTAIN_MINUS 1
#define M2C_PATHCOMP_MAY_CONTAIN_TILDE 0
#endif

#endif /* M2C_PATHNAME_POLICY_H */

/* END OF FILE */