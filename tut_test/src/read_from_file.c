#include <stdio.h>
#include "fileio.h"

int read_array_from_file(char* filename, int* array, int size) {
 
  FILE* fptr = fopen(filename, "r");
  
  if (fptr == NULL) {
    printf("Failed to open the file %s\n", filename);
    return -1;
  }
 
  int i = 0, num = 0; 

  fscanf(fptr, "%d", &num);
  while (num != EOF && i < size) {
    array[i++] = num; 
    fscanf(fptr, "%d", &num);
  }
 
  fclose(fptr);
  
  return 0;
}
