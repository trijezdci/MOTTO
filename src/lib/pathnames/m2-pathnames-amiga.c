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
 * m2-pathnames-posix.c
 *
 * Implementation of AmigaOS pathname and filename parsing.
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
 * directory separator
 * ----------------------------------------------------------------------- */

#define DIRSEP '/'


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

#define parse_filename parse_path_component

static uint_t parse_path_component
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
 *   rootPath | parentPath | relativePath | filenameOnly
 *   ;
 *
 * rootPath :=
 *   devname? ':' relativePath
 *   ;
 *
 * devname :=
 *   Letter ComponentChar* ( ' ' ComponentChar+ )*
 *   ;
 *
 * parentPath :=
 *   '/'+ relativePath
 *   ;
 *
 * relativePath :=
 *   ( pathComponent '/' )* pathComponent?
 *   ;
 *
 * pathComponent :=
 *   '.'? pathSubComponent ( '.' pathSubComponent )*
 *   ;
 *
 * pathSubComponent :=
 *   ComponentLeadChar ComponentChar* ( ' ' ComponentChar+ )*
 *   ;
 *
 * filenameOnly := pathComponent ;
 *
 * ComponentChar :=
 *   ComponentLeadChar | '-' [# | '~' #]
 *   ;
 *
 * ComponentLeadChar :=
 *   Letter | Digit | '_'
 *   ;
 *
 * Digit :=
 *   '0' .. '9'
 *   ;
 *
 * Letter :=
 *   'a' .. 'z' | 'A' .. 'Z'
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
 * filenameOnly := pathComponent ;
 *
 * pathComponent :=
 *   '.'? pathSubComponent ( '.' pathSubComponent )*
 *   ;
 *
 * pathSubComponent :=
 *   ComponentLeadChar ComponentChar* ( ' ' ComponentChar+ )*
 *   ;
 *
 * ComponentChar :=
 *   ComponentLeadChar | '-' [# | '~' #]
 *   ;
 *
 * ComponentLeadChar :=
 *   Letter | Digit | '_'
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
     ((suffix[1] == 'D') && (suffix[2] == 'E') && (suffix[3] == 'F')));
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
  uint_t dir_len;
  
  if ((dirpath == NULL) || (basename == NULL) || (dirpath[0] == ASCII_NUL)) {
    return NULL;
  } /* end if */
  
  dir_len = cstr_length(dirpath);
  
  if (dir_len == 0) {
    return NULL;
  } /* end if */
    
  if /* directory separator missing */ (dirpath[dir_len - 1] != DIRSEP) {
    new_path = new_cstr_by_concat(dirpath, "/", basename, suffix, NULL);
  }
  else /* directory separator present */ {
    new_path = new_cstr_by_concat(dirpath, basename, suffix, NULL);
  } /* end if */
  
  return new_path;
} /* end new_path_w_components */


/* *********************************************************************** *
 * Private Functions
 * *********************************************************************** */

/* --------------------------------------------------------------------------
 * forward declarations
 * ----------------------------------------------------------------------- */

static uint_t parse_root_path
  (const char *path, uint_t index, bool *invalid, int *last_dirsep);

static uint_t parse_devname (const char *path, uint_t index);

static uint_t parse_relative_path
  (const char *path, uint_t index, bool *invalid, int *filename_index);

static uint_t parse_path_subcomponent (const char *path, uint_t index);


/* --------------------------------------------------------------------------
 * macro IS_PATH_COMPONENT_CHAR (ch)
 * --------------------------------------------------------------------------
 * PathComponentChar :=
 *   PathComponentLeadChar | '-' [# | '~' #]
 *   ;
 * ----------------------------------------------------------------------- */

#define IS_PATH_COMPONENT_CHAR(_ch) \
  (IS_PATH_COMPONENT_LEAD_CHAR(_ch) || IS_OPTIONAL_COMPONENT_CHAR(_ch))


/* --------------------------------------------------------------------------
 * macro IS_PATH_COMPONENT_LEAD_CHAR (ch)
 * --------------------------------------------------------------------------
 * PathComponentFirstChar :=
 *   'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_'
 *   ;
 * ----------------------------------------------------------------------- */

#define IS_PATH_COMPONENT_LEAD_CHAR(_ch) \
  (IS_ALPHANUMERIC(_ch) || ((_ch) == '_'))


/* --------------------------------------------------------------------------
 * macro IS_OPTIONAL_COMPONENT_CHAR (ch)
 * --------------------------------------------------------------------------
 *   '-' [# | '~' #]
 * ----------------------------------------------------------------------- */

#define IS_OPTIONAL_COMPONENT_CHAR(_ch) \
  (((M2C_PATHCOMP_MAY_CONTAIN_MINUS) && ((_ch) == '-')) || \
   ((M2C_PATHCOMP_MAY_CONTAIN_TILDE) && ((_ch) == '~')))


/* --------------------------------------------------------------------------
 * private type path_type_t.
 * ----------------------------------------------------------------------- */

typedef enum {
  M2C_PATH_TYPE_ROOT_PATH,
  M2C_PATH_TYPE_PARENT_PATH,
  M2C_PATH_TYPE_RELATIVE_PATH,
  M2C_PATH_TYPE_FILENAME_ONLY,
  M2C_PATH_TYPE_INVALID_PATH
} path_type_t;


/* --------------------------------------------------------------------------
 * function get_path_type(path, index)
 * --------------------------------------------------------------------------
 * Pre-scans path to search for the first occurrence of '/', ':' or NUL to
 * determine the type of path and returns a value of path_type_t.
 * ----------------------------------------------------------------------- */

#define IS_TELL_TALE_CHAR(_ch) \
  (((_ch) == ASCII_NUL) || ((_ch) == DIRSEP) || ((_ch) == ':'))

static path_type_t get_path_type (const char* path, uint_t index) {
  char ch;
  
  ch = path[index];
  
  /* leading directory separator indicates parent path */
  if (ch == DIRSEP) {
    return M2C_PATH_TYPE_PARENT_PATH;
  } /* end if */
  
  /* search for first occurrence of directory separator or colon */
  while (!IS_TELL_TALE_CHAR(ch)) {
    index++; ch = path[index];
  } /* end while */
  
  switch (ch) {
    /* absence of directory separator and colon indicates filename */
    case ASCII_NUL :
      return M2C_PATH_TYPE_FILENAME_ONLY;
      
    /* non-leading directory separator indicates relative path */
    case DIRSEP :
      return M2C_PATH_TYPE_RELATIVE_PATH;
      
    /* colon indicates root path */
    case ':' :
      return M2C_PATH_TYPE_ROOT_PATH;
  } /* end switch */
  
  return M2C_PATH_TYPE_INVALID_PATH;
} /* end get_path_type */


/* --------------------------------------------------------------------------
 * function parse_pathname(path, index, invalid, end_of_dirpath)
 * --------------------------------------------------------------------------
 * pathname :=
 *   rootPath | parentPath | relativePath | filenameOnly
 *   ;
 *
 * rootPath :=
 *   devname? ':' relativePath
 *   ;
 *
 * parentPath :=
 *   '/'+ relativePath
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_pathname
  (const char *path,    /* in */
   uint_t index,        /* in */
   bool *invalid,       /* out, may not be NULL */
   int *filename_index) /* out, but may be NULL */ {
  
  path_type_t path_type;
  
  /* intermediate filename position */
  int filename_pos = NO_FILENAME_FOUND;
  
  /* pre-scan to determine start symbol */
  path_type = get_path_type(path, index);
  
  switch (path_type) {
    /* rootPath */
    case M2C_PATH_TYPE_ROOT_PATH :
      /* devname? */
      if (IS_LETTER(path[index])) {
        index = parse_devname(path, index);
      } /* end if */
      
      /* ':' */
      if (path[index] == ':') {
        index++;
      }
      else /* invalid path */ {
        *invalid = true;
        return index;
      } /* end if */
      
      /* relativePath */
      index = parse_relative_path(path, index, invalid, &filename_pos);
      break;
      
    /* | parentPath */
    case M2C_PATH_TYPE_PARENT_PATH :
      /* '/'+ */
      while (path[index] == DIRSEP) {
        index++;
      } /* end while */
      
      /* relativePath */
      index = parse_relative_path(path, index, invalid, &filename_pos);
      break;
      
    /* | relativePath */
    case M2C_PATH_TYPE_RELATIVE_PATH :
      /* relativePath */
      index = parse_relative_path(path, index, invalid, &filename_pos);
      break;
            
    /* | filenameOnly */
    case M2C_PATH_TYPE_FILENAME_ONLY :
      filename_pos = index;
      index = parse_filename(path, index, invalid, NULL);
      break;
      
    /* invalid path */
    default :
      *invalid = true;
  } /* end switch */ 
  
  /* pathname should end here, otherwise it is invalid */
  if (path[index] != ASCII_NUL) {
    *invalid = true;
  } /* end if */
  
  /* if successful, pass back filename index */
  if (NOT(*invalid)) {
    WRITE_OUTPARAM(filename_index, filename_pos);
  } /* end if */
  
  return index;
} /* end parse_pathname */  


/* --------------------------------------------------------------------------
 * function parse_devname(path, index)
 * --------------------------------------------------------------------------
 * devname :=
 *   Letter ComponentChar* ( ' ' ComponentChar+ )*
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_devname (const char *path, uint_t index) {
  
  /* Letter */
  index++;
  
  /* ComponentChar* */
  while (IS_PATH_COMPONENT_CHAR(path[index])) {
    index++;
  } /* end while */
  
#if (M2C_PATHCOMP_MAY_CONTAIN_SPACE != 0)
  /* ( ' ' ComponentChar+ )* */
  while ((path[index] == ' ') && (IS_PATH_COMPONENT_CHAR(path[index]))) {
   /* ' ' ComponentChar */
   index = index + 2;
   
   /* ComponentChar* */
   while (IS_PATH_COMPONENT_CHAR(path[index])) {
     index++;
   } /* end while */
  } /* end while */
#endif
  
  return index;
} /* end parse_devname */


/* --------------------------------------------------------------------------
 * function parse_relative_path()
 * --------------------------------------------------------------------------
 * Verifies a substring of path starting at the given index against the EBNF
 * rule for rootPath (see below) and returns the index of the character that
 * follows the last matched character.  If the substring does not match,
 * processing stops at the first mismatched character and true is passed in
 * out-parameter invalid, otherwise false.  Upon success, the index of the
 * last found directory separator within the matched substring is passed in
 * out-parameter last_dirsep, unless it is NULL.  Value NO_DIRSEP_FOUND
 * indicates that no such separator was found within the matched substring.
 * --------------------------------------------------------------------------
 * relativePath :=
 *   ( pathComponent '/' )* pathComponent?
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_relative_path
  (const char *path,    /* in */
   uint_t index,        /* in */
   bool *invalid,       /* out, may not be NULL */
   int *filename_index) /* out, but may be NULL */ {
  
  /* intermediate filename position */
  int filename_pos = NO_FILENAME_FOUND;
  
  while ((path[index] == '.') ||
         (IS_PATH_COMPONENT_LEAD_CHAR(path[index]))) {
    
    /* possibly a filename, remember position */
    filename_pos = index;
    
    /* pathComponent */
    index = parse_path_component(path, index, invalid, NULL);
    
    /* bail if error occurred */
    if (*invalid == true) {
      return index;
    } /* end if */
    
    /* '/' */
    if (path[index] == DIRSEP) {
      /* last path component was not a filename */
      filename_pos = NO_FILENAME_FOUND;
      index++;
    }
    /* pathComponent? */
    else if (path[index] == ASCII_NUL) {
      break;
    } /* end if */
  } /* end if */
  
  /* pass back index of last path component */
  WRITE_OUTPARAM(filename_index, filename_pos);
  
  /* pass back validity */
  *invalid = false;
  
  return index;
} /* end parse_relative_path */


/* --------------------------------------------------------------------------
 * function parse_path_component(path, index, invalid, suffix_index)
 * --------------------------------------------------------------------------
 * Verifies a substring of path starting at the given index against the EBNF
 * rule for pathComponent (see below) and returns the index of the character
 * that follows the last matched character.  If the substring does not match,
 * processing stops at the first mismatched character and true is passed in
 * out-parameter invalid, otherwise false.  Upon success, the index of the
 * suffix found within the matched substring is passed in out-parameter
 * suffix_index, unless it is NULL.  Value NO_SUFFIX_FOUND indicates that
 * no suffix was found within the matched substring.
 * --------------------------------------------------------------------------
 * pathComponent :=
 *   '.'? pathSubComponent ( '.' pathSubComponent )*
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_path_component
  (const char *path,  /* in */
   uint_t index,      /* in */
   bool *invalid,     /* out, may not be NULL */
   int *suffix_index) /* out, but may be NULL */ {
  
  /* intermediate suffix index */
  int suffix_pos = NO_SUFFIX_FOUND;
  
#if (M2C_PATHCOMP_MAY_CONTAIN_PERIOD != 0)
  /* '.'? */
  if (path[index] == '.') {
    index++;
  } /* end if */
#endif
  
  /* pathSubComponent */
  if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
      index = parse_path_subcomponent(path, index);
  }
  else /* invalid path */ {
    *invalid = true;
    return index;
  } /* end if */
  
#if (M2C_PATHCOMP_MAY_CONTAIN_PERIOD != 0)
  /* ( '.' pathSubComponent )* */
  while (path[index] == '.')
#else /* only one period for suffix */
  /* ( '.' pathSubComponent )? */
  if (path[index] == '.')
#endif
  /* body */ {
    /* '.' */
    suffix_pos = index;
    index++;
    
    /* pathSubComponent */
    if (IS_PATH_COMPONENT_LEAD_CHAR(path[index])) {
      index = parse_path_subcomponent(path, index);
    }
    else /* invalid path */ {
      *invalid = true;
      return index;
    } /* end if */
  } /* end while */
  
  /* pass back index of last period found */
  WRITE_OUTPARAM(suffix_index, suffix_pos);
  
  /* pass back validity */
  *invalid = false;
  
  return index;
} /* end parse_path_component */


/* --------------------------------------------------------------------------
 * function parse_path_subcomponent(path, index)
 * --------------------------------------------------------------------------
 * pathSubComponent :=
 *   ComponentLeadChar ComponentChar* ( ' ' ComponentChar+ )*
 *   ;
 * ----------------------------------------------------------------------- */

static uint_t parse_path_subcomponent (const char *path, uint_t index) {
  
  /* ComponentLeadChar */
  index++;
  
  /* ComponentChar* */
  while (IS_PATH_COMPONENT_CHAR(path[index])) {
    index++;
  } /* end while */
  
#if (M2C_PATHCOMP_MAY_CONTAIN_SPACE != 0)
  /* ( ' ' ComponentChar+ )* */
  while ((path[index] == ' ') && (IS_PATH_COMPONENT_CHAR(path[index+1]))) {
    /* ' ' ComponentChar */
    index = index + 2;
    
    /* ComponentChar* */
    while (IS_PATH_COMPONENT_CHAR(path[index])) {
      index++;
    } /* end while */
  } /* end while */
#endif
  
  return index;
} /* end parse_path_subcomponent */


/* END OF FILE */