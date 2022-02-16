#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

    char* string = malloc(32*sizeof(char));

    sprintf(string, "Hello World\n");
    printf("%s", string);

  return 0;
} /* main */
