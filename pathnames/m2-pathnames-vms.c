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
 * m2-pathnames-vms.c
 *
 * Implementation of OpenVMS pathname and filename parsing.
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

#include "m2-pathnames.h"
#include "m2-pathname-policy.h"

#include "cstring.h"
#include <stddef.h>


/* --------------------------------------------------------------------------
 * sentinel values
 * ----------------------------------------------------------------------- */

#define NO_FILENAME_FOUND (-1)
#define NO_SUFFIX_FOUND (-1)


/* --------------------------------------------------------------------------
 * forward declarations
 * ----------------------------------------------------------------------- */

static uint_t parse_pathname
  (const char *path, uint_t index, bool *invalid, int *filename_index);

static uint_t parse_filename
  (const char *path, uint_t index, bool *invalid, int *suffix_index);


/* --------------------------------------------------------------------------
 * function split_pathname(path, dirpath, filename, chars_processed)
 * --------------------------------------------------------------------------
 * Verifies path against EBNF rule pathname (see below) and returns a status
 * code.  On success, path's directory path and filename are copied to newly
 * allocated NUL terminated C strings and passed back in out-parameters
 * dirpath and filename respectively.  However, if NULL is passed in for an
 * out-parameter, the out-parameter is ignored and no value is passed back.
 * If path contains no directory path, NULL is passed back in dirpath.  If it
 * contains no filename, NULL is passed back in filename.  The index of the
 * last processed character is passed back in chars_processed.  On success,
 * it represents the length of path.  On failure, it represents the index
 * of the first offending character found in path.
 * --------------------------------------------------------------------------
 * pathname :=
 *   ( workPath | relativePath | absolutePath |
 *     nodePath | devicePath | filenameOnly )
 *   ;
 *
 * workPath :=
 *   ( '[]' | '<>' ) filename?
 *   ;
 *
 * relativePath :=
 *   '[' ( '.' dirPath | parentDirPath ) ']' filename? |
 *   '<' ( '.' dirPath | parentDirPath ) '>' filename?
 *   ;
 *
 * dirPath :=
 *   pathComponent ( '.' pathComponent )*
 *   ;
 *
 * parentDirPath :=
 *   ( '-' '.' )* dirPath | '-'+ ( '.' dirPath )?
 *   ;
 *
 * absolutePath :=
 *   ( '[' dirPath ']' | '<' dirPath '>' ) filename?
 *   ;
 *
 * nodePath :=
 *   nodeName? '::' ( devicePath | filenameOnly )?
 *   ;
 *
 * nodeName :=
 *   ComponentLeadChar ComponentChar{0..5}
 *   ;
 *
 * devicePath :=
 *   deviceName ':' ( absolutePath | filename )
 *   ;
 *
 * deviceName :=
 *   ComponentLeadChar ComponentChar{0..38}
 *   ;
 *
 * filenameOnly := filename ;
 *
 * filename :=
 *   pathComponent ( '.' pathComponent )?
 *   ;
 *
 * pathComponent :=
 *   [# '^' '.' #] pathSubComponent [# ( '^' '.' pathSubComponent )* #]
 *   ;
 *
 * pathSubComponent :=
 *   ComponentLeadChar ComponentChar* [# ( '^' ' ' ComponentChar+ )* #]
 *   ;
 *
 * ComponentChar :=
 *   ComponentLeadChar | '-' | '~'
 *   ;
 *
 * ComponentLeadChar :=
 *   'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '$' | '_'
 *   ;
 *
 * Terms enclosed in [# and #] may be enabled in m2-pathname-policy.h
 * ----------------------------------------------------------------------- */

m2c_pathname_status_t split_pathname
  (const char *path,         /* in, may not be NULL */
   char_ptr_t *dirpath,      /* out, pass in NULL to ignore */
   char_ptr_t *filename,     /* out, pass in NULL to ignore */
   uint_t *chars_processed)  /* out, pass in NULL to ignore */ {
   
  bool invalid;
  int fn_index;
  uint_t final_index, dplen, fnlen;
  
  if (path == NULL) {
    return M2C_PATHNAME_STATUS_INVALID_REFERENCE;
  } /* end if */
  
  if (path[0] == ASCII_NUL) {
    WRITE_OUTPARAM(chars_processed, 0);
    return M2C_PATHNAME_STATUS_INVALID_PATH;
  } /* end if */
  
  final_index = parse_pathname(path, 0, &invalid, &fn_index);
  
  if (invalid) {
    WRITE_OUTPARAM(chars_processed, final_index);
    return M2C_PATHNAME_STATUS_INVALID_PATH;
  } /* end if */
    
  if (fn_index == NO_FILENAME_FOUND) {
    dplen = final_index;
    fnlen = 0;
  }
  else /* filename found */ {
    dplen = fn_index;
    fnlen = final_index - dplen;
  } /* end if */
  
  /* allocate and copy dirpath, pass it back */
  WRITE_OUTPARAM(dirpath, new_cstr_from_slice(path, 0, dplen));
  
  /* allocate and copy filename, pass it back */
  WRITE_OUTPARAM(filename, new_cstr_from_slice(path, dplen, fnlen));
  
  /* pass back number of characters processed */
  WRITE_OUTPARAM(chars_processed, final_index);
   
  return M2C_PATHNAME_STATUS_SUCCESS;
} /* end split_pathname */


/* --------------------------------------------------------------------------
 * function is_valid_pathname(path)
 * --------------------------------------------------------------------------
 * Returns true if pathname is a valid pathname, otherwise false.
 * ----------------------------------------------------------------------- */

bool is_valid_pathname (const char *path) {  
  bool invalid;
  
  if ((path == NULL) || (path[0] == ASCII_NUL)) {
    return false;
  } /* end if */
  
  parse_pathname(path, 0, &invalid, NULL);
  
  return (invalid == false);
} /* end is_valid_pathname */


/* --------------------------------------------------------------------------
 * function split_filename(filename, basename, suffix, chars_processed)
 * --------------------------------------------------------------------------
 * Verifies filename against EBNF rule filename (see below) and returns a
 * status code.  On success, filename's basename and suffix are copied to
 * newly allocated NUL terminated C strings and passed back in out-parameters
 * basename and suffix respectively.  However, if NULL is passed in for an
 * out-parameter, the out-parameter is ignored and no value is passed back.
 * If filename contains no suffix, NULL is passed back in suffix.  If the
 * filename starts with a period its basename starts with that period.  The
 * index of the last processed character is passed back in chars_processed.
 * On success, it represents the length of filename.  On failure, it
 * represents the index of the first offending character found in filename.
 * --------------------------------------------------------------------------
 * filename :=
 *   pathComponent ( '.' pathComponent )?
 *   ;
 *
 * pathComponent :=
 *   [# '^' '.' #] pathSubComponent [# ( '^' '.' pathSubComponent )* #]
 *   ;
 *
 * pathSubComponent :=
 *   ComponentLeadChar ComponentChar* [# ( '^' ' ' ComponentChar+ )* #]
 *   ;
 *
 * ComponentChar :=
 *   TailComponentLeadChar | '-' | '~'
 *   ;
 *
 * ComponentLeadChar :=
 *   'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '$' | '_'
 *   ;
 *
 * Terms enclosed in [# and #] may be enabled in m2-pathname-policy.h
 * ----------------------------------------------------------------------- */

m2c_pathname_status_t split_filename
  (const char *filename,     /* in, may not be NULL */
   char_ptr_t *basename,     /* out, pass in NULL to ignore */
   char_ptr_t *suffix,       /* out, pass in NULL to ignore */
   uint_t *chars_processed)  /* out, pass in NULL to ignore */ {
   
  bool invalid;
  int suffixpos;
  uint_t final_index, baselen, suffixlen;
  
  if (filename == NULL) {
    return M2C_PATHNAME_STATUS_INVALID_REFERENCE;
  } /* end if */
  
  if (filename[0] == ASCII_NUL) {
    WRITE_OUTPARAM(chars_processed, 0);
    return M2C_PATHNAME_STATUS_INVALID_FILENAME;
  } /* end if */
  
  final_index = parse_filename(filename, 0, &invalid, &suffixpos);
  
  if (invalid) {
    WRITE_OUTPARAM(chars_processed, final_index);
    return M2C_PATHNAME_STATUS_INVALID_FILENAME;
  } /* end if */
    
  if (suffixpos == NO_SUFFIX_FOUND) {
    baselen = final_index;
    suffixlen = 0;
  }  
  else /* suffix found */ {
    baselen = suffixpos;
    suffixlen = final_index - suffixpos;
  } /* end if */
  
  /* allocate and copy basename, pass it back */
  WRITE_OUTPARAM(basename, new_cstr_from_slice(filename, 0, baselen));
  
  /* allocate and copy suffix, pass it back */
  WRITE_OUTPARAM(suffix, new_cstr_from_slice(filename, baselen, suffixlen));
  
  /* pass back number of characters processed */
  WRITE_OUTPARAM(chars_processed, final_index);
   
  return M2C_PATHNAME_STATUS_SUCCESS;
} /* end split_filename */


/* --------------------------------------------------------------------------
 * function is_valid_filename(filename)
 * --------------------------------------------------------------------------
 * Returns true if filename is a valid filename, otherwise false.
 * ----------------------------------------------------------------------- */

bool is_valid_filename (const char *filename) {  
  bool invalid;
  
  if ((filename == NULL) || (filename[0] == ASCII_NUL)) {
    return false;
  } /* end if */
  
  parse_filename(filename, 0, &invalid, NULL);
  
  return (invalid == false);
} /* end is_valid_filename */


/* --------------------------------------------------------------------------
 * function is_def_suffix(suffix)
 * --------------------------------------------------------------------------
 * Returns true if suffix is ".def" or ".DEF", otherwise false.
 * ----------------------------------------------------------------------- */

bool is_def_suffix (const char *suffix) {
  return (suffix[0] == '.') && (suffix[4] == ASCII_NUL) &&
    (((suffix[1] == 'd') && (suffix[2] == 'e') && (suffix[3] == 'f')) ||
     ((suffix[1] == 'D') && (suffix[2] == 'D') && (suffix[3] == 'F')));
} /* end is_def_suffix */


/* --------------------------------------------------------------------------
 * function is_mod_suffix(suffix)
 * --------------------------------------------------------------------------
 * Returns true if suffix is ".mod" or ".MOD", otherwise false.
 * ----------------------------------------------------------------------- */

bool is_mod_suffix (const char *suffix) {
  return (suffix[0] == '.') && (suffix[4] == ASCII_NUL) &&
    (((suffix[1] == 'm') && (suffix[2] == 'o') && (suffix[3] == 'd')) ||
     ((suffix[1] == 'M') && (suffix[2] == 'O') && (suffix[3] == 'D')));
} /* end is_mod_suffix */


/* --------------------------------------------------------------------------
 * function new_path_w_components(dirpath, basename, suffix)
 * --------------------------------------------------------------------------
 * Returns a newly allocated NUL terminated C string containing a pathname
 * composed of components dirpath, basename and file suffix.  Returns NULL
 * if any of dirpath or basename is NULL or empty or if allocation failed.
 * ----------------------------------------------------------------------- */

const char *new_path_w_components
  (const char *dirpath, const char *basename, const char *suffix) {
  
  const char *new_path;
  
  if ((dirpath == NULL) || (dirpath[0] == ASCII_NUL)) {
    return NULL;
  } /* end if */
  
  if ((basename == NULL) || (dirpath[0] == ASCII_NUL)) {
    return NULL;
  } /* end if */
  
  new_path = new_cstr_by_concat(dirpath, basename, suffix, NULL);
  
  return new_path;
} /* end new_path_w_components */


/* *********************************************************************** *
 * Private Functions
 * *********************************************************************** */

/* --------------------------------------------------------------------------
 * forward declarations
 * ----------------------------------------------------------------------- */

static uint_t parse_relative_path
  (const char *path, uint_t index, bool *invalid, int *filename_index);

static uint_t parse_absolute_path
  (const char *path, uint_t index, bool *invalid, int *filename_index);

static uint_t parse_node_path
  (const char *path, uint_t index, bool *invalid, int *filename_index);

static uint_t parse_device_path
  (const char *path, uint_t index, bool *invalid, int *filename_index);

static uint_t parse_dir_path
  (const char *path, uint_t index, bool *invalid);

static uint_t parse_parent_dir_path
  (const char *path, uint_t index, bool *invalid);

static uint_t parse_path_sub_component
  (const char *path, uint_t index, bool *invalid);

#if (M2C_PATHCOMP_MAY_CONTAIN_PERIOD != 0)
static uint_t parse_path_component_w_period
  (const char *path, uint_t index, bool *invalid);
#define parse_path_component parse_path_component_w_period
#else
#define parse_path_component parse_path_sub_component
#endif


/* --------------------------------------------------------------------------
 * private function macro IS_WORKING_DIR(path, index)
 * --------------------------------------------------------------------------
 * Returns true if path[index] is the start of '[]' or '<>'.
 * ----------------------------------------------------------------------- */

#define IS_WORKING_DIR(_path,_index) \
  (((_path[_index] == '[') && (_path[_index+1] == ']')) || \
   ((_path[_index] == '<') && (_path[_index+1] == '>')))


/* --------------------------------------------------------------------------
 * private function macro IS_RELATIVE_DIR(path, index)
 * --------------------------------------------------------------------------
 * Returns true if path[index] is the start of a relative directory path.
 * ----------------------------------------------------------------------- */

#define IS_RELATIVE_DIR(_path,_index) \
  (((_path[_index] == '[') || (_path[_index] == '<')) && \
   ((_path[_index+1] == '.') || (_path[_index+1] == '-')))


/* --------------------------------------------------------------------------
 * private function macro IS_ABSOLUTE_DIR(path, index)
 * --------------------------------------------------------------------------
 * Returns true if path[index] is the start of an absolute directory path.
 * ----------------------------------------------------------------------- */

#define IS_ABSOLUTE_DIR(_path,_index) \
  (((_path[_index] == '[') || (_path[_index] == '<')) && \
   (IS_PATH_COMPONENT_LEAD_CHAR(_path[_index+1])))


/* --------------------------------------------------------------------------
 * macro IS_PATH_COMPONENT_CHAR (ch)
 * --------------------------------------------------------------------------
 * PathComponentChar :=
 *   PathComponentLeadChar | '-' | '~'
 *   ;
 * ----------------------------------------------------------------------- */

#define IS_PATH_COMPONENT_CHAR(_ch) \
  (IS_PATH_COMPONENT_LEAD_CHAR(_ch) || IS_OPTIONAL_COMPONENT_CHAR(_ch))


/* --------------------------------------------------------------------------
 * macro IS_PATH_COMPONENT_LEAD_CHAR (ch)
 * --------------------------------------------------------------------------
 * PathComponentFirstChar :=
 *   'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '$' | '_'
 *   ;
 * ----------------------------------------------------------------------- */

#define IS_PATH_COMPONENT_LEAD_CHAR(_ch) \
  (IS_ALPHANUMERIC(_ch) || ((_ch) == '$') || ((_ch) == '_') || \
   ((M2C_PATHCOMP_MAY_CONTAIN_PERIOD != 0) && ((_ch) == '^')))


/* --------------------------------------------------------------------------
 * macro IS_OPTIONAL_COMPONENT_CHAR (ch)
 * --------------------------------------------------------------------------
 *   '-' | '~'
 * ----------------------------------------------------------------------- */

#define IS_OPTIONAL_COMPONENT_CHAR(_ch) \
  (((M2C_PATHCOMP_MAY_CONTAIN_MINUS) && ((_ch) == '-')) || \
   ((M2C_PATHCOMP_MAY_CONTAIN_TILDE) && ((_ch) == '~')))


/* --------------------------------------------------------------------------
 * private function has_node_prefix(path)
 * --------------------------------------------------------------------------
 * Pre-scans path to search for an occurrence of '::' within the first seven
 * characters to determine if path starts with a node name.
 * ----------------------------------------------------------------------- */

#define MAX_NODENAME_LENGTH 6

static bool has_node_prefix (const char *path) {
  uint_t index = 0;
  
  while ((index <= MAX_NODENAME_LENGTH) && (path[index] != ASCII_NUL)) {
    if ((path[index] == ':') && (path[index+1] == ':')) {
      return true;
    } /* end if */
    index++;
  } /* end while */  
  
  return false;
} /* end has_node_prefix */


/* --------------------------------------------------------------------------
 * private function has_device_prefix(path, start_index)
 * --------------------------------------------------------------------------
 * Pre-scans path to search for an occurrence of ':' within 40 characters of
 * start_index to determine if the slice contains a device name.
 * ----------------------------------------------------------------------- */

#define MAX_DEVICENAME_LENGTH 39

static bool has_device_prefix (const char *path, uint_t index) {
  
  uint_t limit = index + MAX_DEVICENAME_LENGTH;
  
  while ((index <= limit) && (path[index] != ASCII_NUL)) {
    if ((path[index] == ':')  && (path[index+1] != ':')) {
      return true;
    } /* end if */
    index++;
  } /* end while */  
  
  return false;
} /* end has_device_prefix */


/* --------------------------------------------------------------------------
 * function parse_pathname(path, index, invalid, filename_index)
 * --------------------------------------------------------------------------
 * pathname :=
 *   ( workPath | relativePath | absolutePath |
 *     nodePath | devicePath | filenameOnly ) ASCII_NUL
 *   ;
 *
 * workPath :=
 *   ( '[]' | '<>' ) filename?
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_pathname
  (const char *path, uint_t index, bool *invalid, int *filename_index) {
  
  /* init out-parameters */
  *invalid = false;
  *filename_index = NO_FILENAME_FOUND;
  
  /* workPath */
  if (IS_WORKING_DIR(path, index)) {
    /* ( '[]' | '<>' ) */
    index = index + 2;
    
    /* filename? */
    if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
      *filename_index = index;
      index = parse_filename(path, index, invalid, NULL);
    } /* end if */
  }
  /* | relativePath */
  else if (IS_RELATIVE_DIR(path, index)) {
    index = parse_relative_path(path, index, invalid, filename_index);
  }
  /* | absolutePath */
  else if (IS_ABSOLUTE_DIR(path, index)) {
    index = parse_absolute_path(path, index, invalid, filename_index);
  }
  else if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
  
    /* | nodePath */
    if (has_node_prefix(path)) {
      index = parse_node_path(path, index, invalid, filename_index);
    }
    /* | devicePath */
    else if (has_device_prefix(path, index)) {
      index = parse_device_path(path, index, invalid, filename_index);
    }
    /* | filenameOnly */
    else {
      *filename_index = index;
      index = parse_filename(path, index, invalid, NULL);
    } /* end if */
  }
  else /* invalid pathname */ {
    *invalid = true;
    return index;
  } /* end if */
  
  if (path[index] != ASCII_NUL) {
    *invalid = true;
  } /* end if */
  
  return index;
} /* end parse_pathname */


/* --------------------------------------------------------------------------
 * function parse_relative_path(path, index, invalid, filename_index)
 * --------------------------------------------------------------------------
 * relativePath :=
 *   '[' ( '.' dirPath | parentDirPath ) ']' filename? |
 *   '<' ( '.' dirPath | parentDirPath ) '>' filename?
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_relative_path
  (const char *path, uint_t index, bool *invalid, int *filename_index) {
  
  char closing_delimiter;
  
  if (path[index] == '[') {
    closing_delimiter = ']';
  }
  else {
    closing_delimiter = '>';
  } /* end if */
  
  /* opening delimiter */
  index++;
  
  /* '.' dirPath */
  if (path[index] == '.') {
    /* '.' */
    index++;
    
    /* dirPath */
    if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
      index = parse_dir_path(path, index, invalid);
    }
    else /* invalid pathname */ {
      *invalid = true;
    } /* end if */
  }
  /* | parentDirPath ) */
  else {
    index = parse_parent_dir_path(path, index, invalid);
  } /* end if */
  
  if (*invalid) {
    return index;
  } /* end if */
  
  /* closing delimiter */
  if (path[index] == closing_delimiter) {
    index++;
  }
  else /* invalid pathname */ {
    *invalid = true;
    return index;
  } /* end if */
  
  /* filename? */
  if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
    *filename_index = index;
    index = parse_filename(path, index, invalid, NULL);
  } /* end if */
  
  return index;
} /* end parse_relative_path */


/* --------------------------------------------------------------------------
 * function parse_parent_dir_path(path, index, invalid)
 * --------------------------------------------------------------------------
 * parentDirPath :=
 *   ( '-' '.' )* dirPath | '-'+ ( '.' dirPath )?
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_parent_dir_path
  (const char *path, uint_t index, bool *invalid) {
  
  /* ( '-' '.' )* dirPath */
  if (path[index+1] == '.') {
    /* ( '-' '.' )* */
    index = index + 2;
    while ((path[index] == '-') && (path[index+1] == '.')) {
      index = index + 2;
    } /* end while */
    
    /* dirPath */
    if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
      index = parse_dir_path(path, index, invalid);
    }
    else /* invalid pathname */ {
      *invalid = true;
    } /* end if */
  }
  
  /* | '-'+ ( '.' dirPath )? */
  else {
    /* '-'+ */
    while (path[index] == '-') {
      index++;
    } /* end while */
    
    /* ( '.' dirPath )? */
    if (path[index] == '.') {
      /* '.' */
      index++;
      
      /* dirPath */
      if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
        index = parse_dir_path(path, index, invalid);
      }
      else /* invalid pathname */ {
        *invalid = true;
      } /* end if */
    } /* end if */
  } /* end if */
  
  return index;
} /* end parse_parent_dir_path */


/* --------------------------------------------------------------------------
 * function parse_absolute_path(path, index, invalid, filename_index)
 * --------------------------------------------------------------------------
 * absolutePath :=
 *   ( '[' dirPath ']' | '<' dirPath '>' ) filename?
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_absolute_path
  (const char *path, uint_t index, bool *invalid, int *filename_index) {
  
  char closing_delimiter;
  
  if (path[index] == '[') {
    closing_delimiter = ']';
  }
  else {
    closing_delimiter = '>';
  } /* end if */
  
  /* opening delimiter */
  index++;
  
  /* dirPath */
  if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
    index = parse_dir_path(path, index, invalid);
  }
  else /* invalid pathname */ {
    *invalid = true;
  } /* end if */
  
  if (*invalid) {
    return index;
  } /* end if */
  
  /* closing delimiter */
  if (path[index] == closing_delimiter) {
    index++;
  }
  else /* invalid pathname */ {
    *invalid = true;
    return index;
  } /* end if */
  
  /* filename? */
  if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
    *filename_index = index;
    index = parse_filename(path, index, invalid, NULL);
  } /* end if */
  
  return index;
} /* end parse_absolute_path */


/* --------------------------------------------------------------------------
 * function parse_node_path(path, index, invalid, filename_index)
 * --------------------------------------------------------------------------
 * nodePath :=
 *   nodeName? '::' ( devicePath | filenameOnly )?
 *   ;
 *
 * nodeName :=
 *   ComponentLeadChar ComponentChar{0..5}
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_node_path
  (const char *path, uint_t index, bool *invalid, int *filename_index) {
  
  const uint_t limit = index + MAX_NODENAME_LENGTH;
  
  /* nodeName? */
  if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
    /* ComponentLeadChar */
    index++;
    
    /* ComponentChar{0..5} */
    while ((index < limit) && (IS_PATH_COMPONENT_CHAR(path[index]))) {
      index++;
    } /* end while */
  } /* end if */
  
  /* '::' */
  if (path[index] == ':') {
    index++;
  }
  else /* invalid pathname : missing colon */ {
    *invalid = true;
    return index;
  } /* end if */
  
  if (path[index] == ':') {
    index++;
  }
  else /* invalid pathname : missing colon after colon */ {
    *invalid = true;
    return index;
  } /* end if */
  
  /* ( devicePath | filenameOnly )? */
  if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
    /* devicePath */
    if (has_device_prefix(path, index)) {
      index = parse_device_path(path, index, invalid, filename_index);
    }
    /* | filenameOnly */
    else {
      *filename_index = index;
      index = parse_filename(path, index, invalid, NULL);
    } /* end if*/
  } /* end if */
  
  return index;
} /* end parse_node_path */


/* --------------------------------------------------------------------------
 * function parse_device_path(path, index, invalid, filename_index)
 * --------------------------------------------------------------------------
 * devicePath :=
 *   deviceName ':' ( absolutePath | filename )
 *   ;
 *
 * deviceName :=
 *   ComponentLeadChar ComponentChar{0..38}
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_device_path
  (const char *path, uint_t index, bool *invalid, int *filename_index) {
  
  const uint_t limit = index + MAX_DEVICENAME_LENGTH;
  
  /* deviceName */
  if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
    /* ComponentLeadChar */
    index++;
    
    /* ComponentChar{0..38} */
    while ((index < limit) && (IS_PATH_COMPONENT_CHAR(path[index]))) {
      index++;
    } /* end while */
  }
  else /* invalid pathname : no device name */ {
    *invalid = true;
    return index;
  } /* end if */
  
  /* ':' */
  if (path[index] == ':') {
    index++;
  }
  else /* invalid pathname : missing colon */ {
    *invalid = true;
    return index;
  } /* end if */
  
  /* absolutePath */
  if ((path[index] == '[') || (path[index] == '<')) {
    /* directory */
    index = parse_absolute_path(path, index, invalid, filename_index);
  }
  /* | filename */
  else if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
    *filename_index = index;
    index = parse_filename(path, index, invalid, NULL);
  } /* end if */
  
  return index;
} /* end parse_device_path */


/* --------------------------------------------------------------------------
 * function parse_dir_path(path, index, invalid)
 * --------------------------------------------------------------------------
 * dirPath :=
 *   pathComponent ( '.' pathComponent )*
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_dir_path
  (const char *path, uint_t index, bool *invalid) {
  
  /* pathComponent */
  index = parse_path_component(path, index, invalid);
  
  /* ( '.' pathComponent )* */
  while (NOT(*invalid) && (path[index] == '.')) {
    /* '.' */
    index++;
    
    /* pathComponent */
    if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
      index = parse_path_component(path, index, invalid);
    }
    else /* invalid pathname */ {
      *invalid = true;
    } /* end if */
  } /* end while */
  
  return index;
} /* end parse_dir_path */


/* --------------------------------------------------------------------------
 * function parse_filename(path, index, invalid, suffix_index)
 * --------------------------------------------------------------------------
 * filename :=
 *   pathComponent ( '.' pathComponent )?
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_filename
  (const char *path, uint_t index, bool *invalid, int *suffix_index) {
  
  int suffix_pos = NO_SUFFIX_FOUND;
  
  /* pathComponent */
  index = parse_path_component(path, index, invalid);
  
  /* ( '.' pathComponent )? */
  if (NOT(*invalid) && (path[index] == '.')) {
    suffix_pos = index;
    
    /* '.' */
    index++;
    
    /* pathComponent */
    if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
      index = parse_path_component(path, index, invalid);
    }
    else /* invalid pathname */ {
      *invalid = true;
    } /* end if */
  } /* end if */
  
  /* pass back index of suffix */
  WRITE_OUTPARAM(suffix_index, suffix_pos);
  
  return index;
} /* end parse_filename */


/* --------------------------------------------------------------------------
 * function parse_path_component_w_period(path, index, invalid)
 * --------------------------------------------------------------------------
 * pathComponent :=
 *   [# '^' '.' #] pathSubComponent [# ( '^' '.' pathSubComponent )* #]
 *   ;
 * ----------------------------------------------------------------------- */

#if (M2C_PATHCOMP_MAY_CONTAIN_PERIOD != 0)
static uint_t parse_path_component_w_period
  (const char *path, uint_t index, bool *invalid) {
  
  /* '^' '.' pathSubComponent */
  if ((path[index] == '^') && (path[index+1] == '.')) {
    index = index + 2;
    
    if (IS_PATH_COMPONENT_CHAR(path[index])) {
      index = parse_path_sub_component(path, index, invalid);
    } /* end if */
  }
  /* | pathSubComponent */
  else {
    index = parse_path_sub_component(path, index, invalid);
  } /* end if */
  
  /* ( '^' '.' pathSubComponent )* */
  while ((NOT(*invalid)) && (path[index] == '^') && (path[index+1] == '.')) {
    index = index + 2;
    
    /* pathSubComponent */
    if (IS_PATH_COMPONENT_CHAR(path[index])) {
      index = parse_path_sub_component(path, index, invalid);
    }
    else /* invalid pathname */ {
      *invalid = true;
    } /* end if */
  } /* end while */
  
  return index;
} /* end parse_path_component_w_period */
#endif


/* --------------------------------------------------------------------------
 * function parse_path_sub_component(path, index, invalid)
 * --------------------------------------------------------------------------
 * pathSubComponent :=
 *   ComponentLeadChar ComponentChar* [# ( '^' ' ' ComponentChar+ )* #]
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_path_sub_component
  (const char *path, uint_t index, bool *invalid) {
  
  /* ComponentLeadChar */
  index++;
  
  /* ComponentChar* */
  while (IS_PATH_COMPONENT_CHAR(path[index])) {
    index++;
  } /* end while */
  
#if (M2C_PATHCOMP_MAY_CONTAIN_SPACE != 0)
  /* ( '^' ' ' ComponentChar+ )* */
  while ((path[index] == '^') && (path[index+1] == ' ')) {
    index = index + 2;
    
    /* ComponentChar */
    if (IS_PATH_COMPONENT_CHAR(path[index])) {
      index++;
    }
    else /* invalid pathname */ {
      *invalid = true;
      return index;
    } /* end if */
    
    /* ComponentChar* */
    while (IS_PATH_COMPONENT_CHAR(path[index])) {
      index++;
    } /* end while */
  } /* end while */
#endif
  
  return index;
} /* end parse_path_sub_component */


/* END OF FILE */