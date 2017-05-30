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
 * m2-filereader.c
 *
 * Implementation of M2C source file reader module.
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

#include "m2-filereader.h"
#include "m2-common.h"
#include "m2-fileutils.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>


/* --------------------------------------------------------------------------
 * private type m2c_infile_struct_t
 * --------------------------------------------------------------------------
 * record type representing a Modula-2 source file.
 * ----------------------------------------------------------------------- */

struct m2c_infile_struct_t {
  /* file */            FILE *file;
  /* filename */        m2c_string_t filename;
  /* index */           size_t index;
  /* line */            uint_t line;
  /* column */          uint_t column;
  /* marker_set */      bool marker_set;
  /* marker_index */    size_t marked_index;
  /* status */          m2c_infile_status_t status;
  /* buflen */          size_t buflen;
  /* buffer */          char buffer[];
};

typedef struct m2c_infile_struct_t m2c_infile_struct_t;


/* --------------------------------------------------------------------------
 * procedure m2c_open_infile(infile, filename, status)
 * --------------------------------------------------------------------------
 * Allocates a new object of type m2c_infile_t, opens an input file and
 * associates the opened file with the newly created infile object.
 *
 * pre-conditions:
 * o  parameter file must be NULL upon entry
 * o  parameter status may be NULL
 *
 * post-conditions:
 * o  pointer to newly allocated and opened file is passed back in infile
 * o  line and column counters of the newly allocated infile are set to 1
 * o  M2C_INFILE_STATUS_SUCCESS is passed back in status, unless NULL
 *
 * error-conditions:
 * o  if infile is not NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * o  if the file represented by filename cannot be found
 *    status M2C_INFILE_STATUS_FILE_NOT_FOUND is returned
 * o  if the file represented by filename cannot be accessed
 *    status M2C_INFILE_STATUS_FILE_ACCESS_DENIED is returned
 * o  if no infile object could be allocated
 *    status M2C_INFILE_STATUS_ALLOCATION_FAILED is returned
 * --------------------------------------------------------------------------
 */

/* TO DO : Check for file size limit */

m2c_infile_t m2c_open_infile
  (m2c_string_t filename, m2c_infile_status_t *status) {
  
  FILE *file; size_t size;
  m2c_infile_t new_infile;
  
  /* check pre-conditions */
  if (filename == NULL) {
    SET_STATUS(status, M2C_INFILE_STATUS_INVALID_REFERENCE);    
    return NULL;
  } /* end if */
  
  /* open file */
  file = fopen(m2c_string_char_ptr(filename), "r");
  
  /* if operation failed, pass back status and return */
  if (file == NULL) {
    if (status != NULL) {
      if ((errno == ENOENT) ||
          (errno == ENOTDIR) ||
          (errno == ENAMETOOLONG)) {
        *status = M2C_INFILE_STATUS_FILE_NOT_FOUND;
      }
      else if (errno == EACCES) {
        *status = M2C_INFILE_STATUS_FILE_ACCESS_DENIED;
      }
      else if (errno == ENOMEM) {
        *status = M2C_INFILE_STATUS_ALLOCATION_FAILED;
      }
      else {
        *status = M2C_INFILE_STATUS_IO_SUBSYSTEM_ERROR;
      } //
    } /* end if */
    return NULL;
  } /* end if */
  
  /* allocate new infile */
  size = filesize(m2c_string_char_ptr(filename));
  new_infile = malloc(sizeof(m2c_infile_struct_t) + size + 1);
  
  /* if allocation failed, close file, pass status and return */
  if (new_infile == NULL) {
    fclose(file);
    
    SET_STATUS(status, M2C_INFILE_STATUS_ALLOCATION_FAILED);    
    return NULL;
  } /* end if */
  
  /* read file contents into buffer */
  new_infile->buflen = fread(&new_infile->buffer, sizeof(char), size, file);
  
  /* if file empty, close file, deallocate infile, pass status and return */
  if (new_infile->buflen == 0) {
    free(new_infile);
    fclose(file);
    
    SET_STATUS(status, M2C_INFILE_STATUS_FILE_EMPTY);
    return NULL;
  } /* end if */
  
  /* initialise newly allocated infile */
  new_infile->file = file;
  new_infile->filename = filename;
  new_infile->index = 0;
  new_infile->line = 1;
  new_infile->column = 1;
  new_infile->marker_set = false;
  new_infile->marked_index = 0;
  new_infile->status = M2C_INFILE_STATUS_SUCCESS;
  
  return new_infile;
} /* m2c_open_infile */


/* --------------------------------------------------------------------------
 * function m2c_read_char(infile)
 * --------------------------------------------------------------------------
 * Reads the lookahead character from infile, advancing the current reading
 * position, updating line and column counter and returns its character code.
 * Returns EOF if the lookahead character lies beyond the end of infile.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  character code of lookahead character or EOF is returned
 * o  current reading position and line and column counters are updated
 * o  file status is set to M2C_INFILE_STATUC_SUCCESS
 *
 * error-conditions:
 * o  if infile is NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * --------------------------------------------------------------------------
 */

/* TO DO : Check for line and column counter limit */

int m2c_read_char (m2c_infile_t infile) {
  char ch;

  /* check pre-conditions */
  if (infile == NULL) {
    return ASCII_NUL;
  } /* end if */
  
  if (infile->index == infile->buflen) {
    infile->status = M2C_INFILE_STATUS_ATTEMPT_TO_READ_PAST_EOF;
    return ASCII_EOT;
  } /* end if */
  
  ch = infile->buffer[infile->index];
  infile->index++;
  
  /* if new line encountered, update line and column counters */
  if (ch == ASCII_LF) {
    infile->line++;
    infile->column = 1;
  }
  else if (ch == ASCII_CR) {
    infile->line++;
    infile->column = 1;
    
    /* if LF follows, skip it */
    if ((infile->index < infile->buflen) &&
        (infile->buffer[infile->index] == ASCII_LF)) {
      infile->index++;
    } /* end if */
        
    ch = ASCII_LF;
  }
  else {
    infile->column++;
  } /* end if */
  
  infile->status = M2C_INFILE_STATUS_SUCCESS;
  return ch;
} /* end m2c_read_char */


/* --------------------------------------------------------------------------
 * function m2c_mark_lexeme(infile)
 * --------------------------------------------------------------------------
 * Marks the current lookahead character as the start of a lexeme.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  position of lookahead character is stored internally
 *
 * error-conditions:
 * o  if infile is NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * --------------------------------------------------------------------------
 */

void m2c_mark_lexeme (m2c_infile_t infile) {
  
  /* check pre-conditions */
  if (infile == NULL) {
    return;
  } /* end if */
  
  /* set marker */
  infile->marker_set = true;
  infile->marked_index = infile->index;
  
  return;
} /* end m2c_mark_lexeme */


/* --------------------------------------------------------------------------
 * function m2c_read_marked_lexeme(infile)
 * --------------------------------------------------------------------------
 * Returns a string object with the character sequence starting with the
 * character that has been marked using procedure m2c_mark_lexeme() and
 * ending with the last consumed character.  Returns NULL if no marker
 * has been set or if the marked character has not been consumed.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  marked position is cleared
 * o  dynamic string with lexeme is returned
 *
 * error-conditions:
 * o  if infile is NULL upon entry,
 *    no operation is carried out and NULL is returned
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_read_marked_lexeme (m2c_infile_t infile) {
  m2c_string_t lexeme;
  unsigned int length;
  m2c_string_status_t status;
  
  /* check pre-conditions */
  if ((!infile->marker_set) || (infile->marked_index == infile->index)) {
    return NULL;
  } /* end if */
  
  /* determine length */
  length = infile->index - infile->marked_index;
  
  /* copy lexeme */
  lexeme = m2c_get_string_for_slice
    (infile->buffer, infile->marked_index, length, &status);
  
  if (status == M2C_STRING_STATUS_ALLOCATION_FAILED) {
    infile->status = M2C_INFILE_STATUS_ALLOCATION_FAILED;
    return NULL;
  } /* end if */
  
  /* clear marker */
  infile->marker_set = false;
  
  return lexeme;
} /* end m2c_read_marked_lexeme */


static size_t m2c_infile_index_for_line (m2c_infile_t infile, uint_t line) {
  size_t index = 0;
  uint_t this_line = 1;
  
  while (this_line != line) {
    /* find end of line */
    while ((index < infile->buflen) &&
           (infile->buffer[index] != ASCII_CR) &&
           (infile->buffer[index] != ASCII_LF)) {
     index++;
     } /* end while */
     
     /* skip LF */
     if (infile->buffer[index] == ASCII_LF) {
       index++;
     }
     /* skip CR or CR LF */
     else if (infile->buffer[index] == ASCII_CR) {
       index++;
       if (infile->buffer[index] == ASCII_LF) {
         index++;
       } /* end if */
     }
     else /* target line does not exist */ {
       return 0;
     } /* end if */
     
     /* next line */
     this_line++;
  } /* end while */
  
  return index;
} /* end m2c_infile_index_for_line */


/* --------------------------------------------------------------------------
 * function m2c_infile_source_for_line(infile, line)
 * --------------------------------------------------------------------------
 * Returns a string object with the source of the given line number.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 * o  parameter line must not be zero upon entry
 *
 * post-conditions:
 * o  dynamic string with source of line is returned
 *
 * error-conditions:
 * o  if infile is NULL, or line is zero upon entry,
 *    no operation is carried out and NULL is returned
 * ----------------------------------------------------------------------- */

m2c_string_t m2c_infile_source_for_line (m2c_infile_t infile, uint_t line) {
  m2c_string_t source;
  size_t start, offset, length;
  m2c_string_status_t status;
  
  /* check pre-conditions */
  if ((infile == NULL) || (line == 0)) {
    return NULL;
  } /* end if */
  
  /* determine start of line */
  start = m2c_infile_index_for_line(infile, line);
  
  if (start == 0) {
    return NULL;
  } /* end if */
  
  /* determine end of line */
  offset = start;
  while ((offset < infile->buflen) &&
         (infile->buffer[offset] != ASCII_LF) &&
         (infile->buffer[offset] != ASCII_CR)) {
    offset++;
  } /* end while */
  
  /* calculate line length */
  length = offset - start;
    
  /* copy current line */
  source = m2c_get_string_for_slice
    (infile->buffer, start, length, &status);
  
  if (status == M2C_STRING_STATUS_ALLOCATION_FAILED) {
    infile->status = M2C_INFILE_STATUS_ALLOCATION_FAILED;
    return NULL;
  } /* end if */
  
  return source;
} /* end m2c_infile_source_for_line */


/* --------------------------------------------------------------------------
 * function m2c_consume_char(infile)
 * --------------------------------------------------------------------------
 * Consumes the current lookahead character, advancing the current reading
 * position, updating line and column counter and returns the character code
 * of the new lookahead character that follows the consumed character.
 * Returns EOF if the lookahead character lies beyond the end of infile.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  character code of lookahead character or EOF is returned
 * o  current reading position and line and column counters are updated
 * o  file status is set to M2C_INFILE_STATUC_SUCCESS
 *
 * error-conditions:
 * o  if infile is NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * --------------------------------------------------------------------------
 */

inline int m2c_consume_char (m2c_infile_t infile) {
  
  /* consume lookahead character */
  m2c_read_char(infile);
  
  /* return new lookahead character */
  return m2c_next_char(infile);
} /* end m2c_consume_char */


/* --------------------------------------------------------------------------
 * function m2c_next_char(infile)
 * --------------------------------------------------------------------------
 * Reads the lookahead character from infile without advancing the current
 * reading position and returns its character code.  Returns EOF if the
 * lookahead character lies beyond the end of infile.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  character code of lookahead character or EOF is returned
 * o  current reading position and line and column counters are NOT updated
 * o  file status is set to M2C_INFILE_STATUC_SUCCESS
 *
 * error-conditions:
 * o  if infile is NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * --------------------------------------------------------------------------
 */

int m2c_next_char (m2c_infile_t infile) {
  char ch;

  /* check pre-conditions */
  if (infile == NULL) {
    return ASCII_NUL;
  } /* end if */
  
  if (infile->index == infile->buflen) {
    infile->status = M2C_INFILE_STATUS_ATTEMPT_TO_READ_PAST_EOF;
    return ASCII_EOT;
  } /* end if */
  
  ch = infile->buffer[infile->index];
  
  /* return LF for CR */
  if (ch == ASCII_CR) {
    ch = ASCII_LF;
  } /* end if */
  
  infile->status = M2C_INFILE_STATUS_SUCCESS;
  return ch;  
} /* end m2c_next_char */


/* --------------------------------------------------------------------------
 * function m2c_la2_char(infile)
 * --------------------------------------------------------------------------
 * Reads the second lookahead character from infile without advancing the
 * current reading position and returns its character code.  Returns EOF
 * if the second lookahead character lies beyond the end of infile.
 *
 * pre-conditions:
 * o  parameter infile must not be NULL upon entry
 *
 * post-conditions:
 * o  character code of second lookahead character or EOF is returned
 * o  current reading position and line and column counters are NOT updated
 * o  file status is set to M2C_INFILE_STATUC_SUCCESS
 *
 * error-conditions:
 * o  if infile is NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * --------------------------------------------------------------------------
 */

int m2c_la2_char (m2c_infile_t infile) {
  char la2;
  
  /* check pre-conditions */
  if (infile == NULL) {
    return ASCII_NUL;
  } /* end if */
  
  if (infile->index+1 == infile->buflen) {
    infile->status = M2C_INFILE_STATUS_ATTEMPT_TO_READ_PAST_EOF;
    return ASCII_EOT;
  } /* end if */
  
  la2 = infile->buffer[infile->index+1];
  
  /* skip CR LF sequence if encountered */
  if ((infile->buffer[infile->index] == ASCII_CR) && (la2 == ASCII_LF)) {
    if (infile->index+2 == infile->buflen) {
      infile->status = M2C_INFILE_STATUS_ATTEMPT_TO_READ_PAST_EOF;
      return EOF;
    } /* end if */
    
    la2 = infile->buffer[infile->index+2];
  } /* end if */
  
  /* return LF for CR */
  if (la2 == ASCII_CR) {
    la2 = ASCII_LF;
  } /* end if */
    
  infile->status = M2C_INFILE_STATUS_SUCCESS;
  return la2;
} /* end m2c_la2_char */


/* --------------------------------------------------------------------------
 * function m2c_infile_filename(infile)
 * --------------------------------------------------------------------------
 * Returns the filename associated with infile.
 * --------------------------------------------------------------------------
 */

m2c_string_t m2c_infile_filename (m2c_infile_t infile) {

  /* check pre-conditions */
  if (infile == NULL) {
    return NULL;
  } /* end if */
  
  return infile->filename;
}  /* end m2c_infile_status */


/* --------------------------------------------------------------------------
 * function m2c_infile_status(infile)
 * --------------------------------------------------------------------------
 * Returns the status of the last operation on file.
 * --------------------------------------------------------------------------
 */

m2c_infile_status_t m2c_infile_status (m2c_infile_t infile) {

  /* check pre-conditions */
  if (infile == NULL) {
    return M2C_INFILE_STATUS_INVALID_REFERENCE;
  } /* end if */
  
  return infile->status;
}  /* end m2c_infile_status */


/* --------------------------------------------------------------------------
 * function m2c_infile_eof(infile)
 * --------------------------------------------------------------------------
 * Returns true if the current reading position of infile lies beyond the end
 * of the associated file, returns false otherwise.
 * --------------------------------------------------------------------------
 */

bool m2c_infile_eof (m2c_infile_t infile) {

  /* check pre-conditions */
  if (infile == NULL) {
    return true;
  } /* end if */
  
  return (infile->index == infile->buflen);
} /* end m2c_infile_eof */


/* --------------------------------------------------------------------------
 * function m2c_infile_current_line(infile)
 * --------------------------------------------------------------------------
 * Returns the current line counter of infile.
 * --------------------------------------------------------------------------
 */

unsigned int m2c_infile_current_line (m2c_infile_t infile) {

  /* check pre-conditions */
  if (infile == NULL) {
    return 0;
  } /* end if */
  
  return infile->line;
} /* end m2c_infile_current_line */


/* --------------------------------------------------------------------------
 * function m2c_infile_current_column(infile)
 * --------------------------------------------------------------------------
 * Returns the current column counter of infile.
 * --------------------------------------------------------------------------
 */

unsigned int m2c_infile_current_column (m2c_infile_t infile) {

  /* check pre-conditions */
  if (infile == NULL) {
    return 0;
  } /* end if */
  
  return infile->column;
} /* end m2c_infile_current_column */


/* --------------------------------------------------------------------------
 * procedure m2c_close_infile(file, status)
 * --------------------------------------------------------------------------
 * Closes the file associated with handle file, deallocates its file object
 * and returns NULL in handle file.
 *
 * pre-conditions:
 * o  parameter file must not be NULL upon entry
 * o  parameter status may be NULL
 *
 * post-conditions:
 * o  file object is deallocated
 * o  NULL is passed back in file
 * o  M2C_INFILE_STATUS_SUCCESS is passed back in status, unless NULL
 *
 * error-conditions:
 * o  if file is NULL upon entry, no operation is carried out
 *    and status M2C_INFILE_STATUS_INVALID_REFERENCE is returned
 * -------------------------------------------------------------------------- 
 */

void m2c_close_infile (m2c_infile_t *infptr, m2c_infile_status_t *status) {
  m2c_infile_t infile;
  
  /* check pre-conditions */
  if (infptr == NULL) {
    SET_STATUS(status, M2C_INFILE_STATUS_INVALID_REFERENCE);
    return;
  } /* end if */
  
  infile = *infptr;
  
  fclose(infile->file);
  free(infile);
  *infptr = NULL;
  
  SET_STATUS(status, M2C_INFILE_STATUS_SUCCESS);
  return;
} /* end m2c_close_infile */

/* END OF FILE */