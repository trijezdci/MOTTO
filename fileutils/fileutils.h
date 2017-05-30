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
 * fileutils.h
 *
 * Platform independent interface to file system utility functions.
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

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <stdbool.h>


/* --------------------------------------------------------------------------
 * function file_exists(path)
 * --------------------------------------------------------------------------
 * Tests if path is a valid pathname indicating an existing filesystem entry.
 * Returns true on success, or false on failure.
 * ----------------------------------------------------------------------- */

bool file_exists (const char *path);


/* --------------------------------------------------------------------------
 * function is_directory(path)
 * --------------------------------------------------------------------------
 * Tests if path is a valid pathname indicating an existing directory.
 * Returns true on success, or false on failure.
 * ----------------------------------------------------------------------- */

bool is_directory (const char *path);


/* --------------------------------------------------------------------------
 * function is_regular_file(path)
 * --------------------------------------------------------------------------
 * Tests if path is a valid pathname indicating an existing regular file.
 * Returns true on success, or false on failure.
 * ----------------------------------------------------------------------- */

bool is_regular_file (const char *path);


/* --------------------------------------------------------------------------
 * function get_filesize(path, size)
 * --------------------------------------------------------------------------
 * Tests if path is a valid pathname indicating an existing regular file and
 * if so, it copies the file's size to out-parameter size and returns true.
 * Otherwise it leaves the out-parameter unmodified and returns false.
 * ----------------------------------------------------------------------- */

bool get_filesize (const char *path, long int *size);


/* --------------------------------------------------------------------------
 * function get_filetime(path, timestamp)
 * --------------------------------------------------------------------------
 * Tests if path is a valid pathname indicating an existing regular file and
 * if so, it copies the file's last modification time to out-parameter
 * timestamp and returns true.  Otherwise it leaves the out-parameter
 * unmodified and returns false.
 * ----------------------------------------------------------------------- */

bool get_filetime (const char *path, long int *timestamp);


/* --------------------------------------------------------------------------
 * function new_path_w_current_workdir()
 * --------------------------------------------------------------------------
 * Returns a newly allocated NUL terminated character string containing the
 * absolute path of the current working directory.  Returns NULL on failure.
 * ----------------------------------------------------------------------- */

const char *new_path_w_current_workdir (void);


#endif /* FILEUTILS_H */

/* END OF FILE */