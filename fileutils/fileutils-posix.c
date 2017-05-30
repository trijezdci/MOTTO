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
 * fileutils-posix.c
 *
 * POSIX implementation of file system utility functions.
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

#include "fileutils.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>


/* --------------------------------------------------------------------------
 * function file_exists(path)
 * --------------------------------------------------------------------------
 * Tests if path is a valid pathname indicating an existing filesystem entry.
 * Returns true on success, or false on failure.
 * ----------------------------------------------------------------------- */

bool file_exists (const char *path) {
  int status;
  
  /* path may not be NULL or empty */
  if ((path == NULL) || (path[0] == 0)) {
    return false;
  } /* end if */
  
  /* check file */
  status = access(path, F_OK);
  
  return (status == 0);
} /* end file_exists */


/* --------------------------------------------------------------------------
 * function is_directory(path)
 * --------------------------------------------------------------------------
 * Tests if path is a valid pathname indicating an existing directory.
 * Returns true on success, or false on failure.
 * ----------------------------------------------------------------------- */

bool is_directory (const char *path) {
  struct stat st;
  int status;
  
  /* path may not be NULL or empty */
  if ((path == NULL) || (path[0] == 0)) {
    return false;
  } /* end if */
  
  /* obtain file info */
  status = stat(path, &st);
  
  /* check if file exists and is a directory */
  if ((status != 0) || !(S_ISDIR(st.st_mode))) {
    return false;
  } /* end if */
  
  return true;
} /* end is_directory */


/* --------------------------------------------------------------------------
 * function is_regular_file(path)
 * --------------------------------------------------------------------------
 * Tests if path is a valid pathname indicating an existing regular file.
 * Returns true on success, or false on failure.
 * ----------------------------------------------------------------------- */

bool is_regular_file (const char *path) {
  struct stat st;
  int status;
  
  /* path may not be NULL or empty */
  if ((path == NULL) || (path[0] == 0)) {
    return false;
  } /* end if */
  
  /* obtain file info */
  status = stat(path, &st);
  
  /* check if file exists and is a regular file */
  if ((status != 0) || !(S_ISREG(st.st_mode))) {
    return false;
  } /* end if */
  
  return true;
} /* end is_regular_file */


/* --------------------------------------------------------------------------
 * function get_filesize(path, size)
 * --------------------------------------------------------------------------
 * Tests if path is a valid pathname indicating an existing regular file and
 * if so, it copies the file's size to out-parameter size and returns true.
 * Otherwise it leaves the out-parameter unmodified and returns false.
 * ----------------------------------------------------------------------- */

bool get_filesize (const char *path, long int *size) {
  struct stat st;
  int status;
  
  /* path may not be NULL or empty */
  if ((path == NULL) || (path[0] == 0)) {
    return false;
  } /* end if */
  
  /* obtain file info */
  status = stat(path, &st);
  
  /* check if file exists and is a regular file */
  if ((status != 0) || !(S_ISREG(st.st_mode))) {
    return false;
  } /* end if */
  
  /* pass back size */
  *size = (long int) st.st_size;
  
  return true;
} /* end filesize */


/* --------------------------------------------------------------------------
 * function get_filetime(path, timestamp)
 * --------------------------------------------------------------------------
 * Tests if path is a valid pathname indicating an existing regular file and
 * if so, it copies the file's last modification time to out-parameter
 * timestamp and returns true.  Otherwise it leaves the out-parameter
 * unmodified and returns false.
 * ----------------------------------------------------------------------- */

bool get_filetime (const char *path, long int *timestamp) {
  struct stat st;
  int status;
  
  /* path may not be NULL or empty */
  if ((path == NULL) || (path[0] == 0)) {
    return false;
  } /* end if */
  
  /* obtain file info */
  status = stat(path, &st);
  
  /* check if file exists and is a regular file */
  if ((status != 0) || !(S_ISREG(st.st_mode))) {
    return false;
  } /* end if */
  
  /* pass back timestamp */
  *timestamp = (long int) st.st_mtime;
  
  return true;
} /* end get_filetime */


/* --------------------------------------------------------------------------
 * function new_path_w_current_workdir()
 * --------------------------------------------------------------------------
 * Returns a newly allocated NUL terminated character string containing the
 * absolute path of the current working directory.  Returns NULL on failure.
 * ----------------------------------------------------------------------- */

const char *new_path_w_current_workdir (void) {
  char *str, *path;
  size_t size;
  
  /* obtain length of current working directory */
  size = pathconf(".", _PC_PATH_MAX);
  
  if (NOT(size > 0)) {
    return NULL;
  } /* end if */
  
  size++;
  
  /* allocate string */
  str = malloc(size);
  
  if (str == NULL) {
    return NULL;
  } /* end if */
  
  /* copy current working directory */
  path = getcwd(str, size);
  
  return path;
} /* end new_path_w_current_workdir */


/* END OF FILE */