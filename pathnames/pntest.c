/* test program for pathname parser, by B.Kowarsch 2016 */

#include "m2-pathnames.h"

#include <stdio.h>
#include <stdlib.h>


void print_usage(void) {
  printf("test program for pathname parser\n");
  printf("usage: pntest pathname\n");
} /* end print_usage */


int main (int argc, char *argv[]) {
  const char *path, *dirpath, *filename, *basename, *suffix;
  m2c_pathname_status_t status;
  unsigned index = 0;
  
  if (argc < 2) {
    print_usage();
    exit(EXIT_FAILURE);
  } /* end if */
  
  path = argv[1];
    
  status = split_pathname(path, &dirpath, &filename, &index);
  
  if (status == M2C_PATHNAME_STATUS_INVALID_PATH) {
    printf("invalid path, offending character at index %u\n", index);
    exit(EXIT_FAILURE);
  } /* end if */
  
  printf("dirpath  : %s\n", dirpath);
  printf("filename : %s\n", filename);
  
  if (filename != NULL) {
    status = split_filename(filename, &basename, &suffix, &index);
  
    if (status == M2C_PATHNAME_STATUS_INVALID_FILENAME) {
      printf("invalid filename, offending character at index %u\n", index);
      exit(EXIT_FAILURE);
    } /* end if */
  
    printf("basename : %s\n", basename);
    printf("suffix   : %s\n", suffix);
  } /* end if */
  
  return 0;
} /* end main */

/* END OF FILE */