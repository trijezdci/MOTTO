/* M2C Modula-2 Compiler & Translator
 * Copyright (c) 2015, 2016 Benjamin Kowarsch
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
 * m2-fileio-status.h
 *
 * Common status codes for file IO operations.
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

#ifndef M2C_FILEIO_STATUS_H
#define M2C_FILEIO_STATUS_H

/* --------------------------------------------------------------------------
 * type m2c_infile_status_t
 * --------------------------------------------------------------------------
 * Status codes for file IO operations.
 * ----------------------------------------------------------------------- */

typedef enum {
  M2C_FILEIO_STATUS_SUCCESS,
  M2C_FILEIO_INVALID_FILENAME,
  M2C_FILEIO_FILE_NOT_FOUND,
  M2C_FILEIO_ACCESS_DENIED,
  M2C_FILEIO_DEVICE_ERROR,
  /* ... */
} m2c_fileio_status_t;

#endif /* M2C_FILEIO_STATUS_H */

/* END OF FILE */