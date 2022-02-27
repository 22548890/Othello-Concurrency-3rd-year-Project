#include <stdio.h>
#include "fileio.h"

int append_array_to_file(char* filename, int* array, int size) {  
  
  FILE* fptr = fopen(filename, "a");
  if (fptr == NULL) {
    printf("File %s could not be opened for writing\n", filename);
    return -1;
  }

  for (int i = 0; i < size; i++) {
      fprintf(fptr, "%d\n", array[i]);
  }

  fclose(fptr);

  return 0;
}

int write_array_to_file(char* filename, int* array, int size) {  
  
  FILE* fptr = fopen(filename, "w");
  if (fptr == NULL) {
    printf("File %s could not be opened for writing\n", filename);
    return -1;
  }

  for (int i = 0; i < size; i++) {
      fprintf(fptr, "%d\n", array[i]);
  }

  fclose(fptr);

  return 0;
}
