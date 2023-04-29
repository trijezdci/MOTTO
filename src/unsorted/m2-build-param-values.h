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
 * m2-build-param-values.h
 *
 * Definition of possible values for build parameters.
 *
 * @license
 *
 * M2C is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation;  either version 2 of the License (GPL2),
 * or (at your option) any later version.
 *
 * M2C is distributed in the hope that it will be useful,  but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with m2c.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef M2C_BUILD_PARAM_VALUES_H
#define M2C_BUILD_PARAM_VALUES_H


/* --------------------------------------------------------------------------
 * Values for build parameter M2C_TARGET_OS
 * ----------------------------------------------------------------------- */

#define OS_TYPE_UNKNOWN 0
#define OS_TYPE_MIN_VALUE 1
#define OS_TYPE_POSIX 1
#define OS_TYPE_DOS 2
#define OS_TYPE_VMS 3
#define OS_TYPE_WIN 4
#define OS_TYPE_MAX_VALUE 4


/* --------------------------------------------------------------------------
 * Values for build parameter M2C_TARGET_FILENAMING
 * ----------------------------------------------------------------------- */

#define FILENAMING_UNKNOWN 0
#define FILENAMING_MIN_VALUE 1
#define FILENAMING_POSIX 1
#define FILENAMING_FAT83 2
#define FILENAMING_FILES11 3
#define FILENAMING_NTFS 4
#define FILENAMING_MAX_VALUE 4


#endif /* M2C_BUILD_PARAM_VALUES_H */

/* END OF FILE */