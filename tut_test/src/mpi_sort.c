#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

void merge(int *, int *, int, int, int);
void mergeSort(int *, int *, int, int);

int main(int argc, char **argv)
{

    int n = 10; //specifies length of arr
    int *arrNumbers= malloc(n * sizeof(int));

    int i;
    //hard-coded given values
    arrNumbers[0]=2; 
    arrNumbers[1]=60;  
    arrNumbers[2]=70;  
    arrNumbers[3]=85;  
    arrNumbers[4]=75; 
    arrNumbers[5]=34; 
    arrNumbers[6]=56;  
    arrNumbers[7]=345;  
    arrNumbers[8]=54;  
    arrNumbers[9]=732;  

    int rank;
    int comm_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    int size = n / comm_size; //divides in equal pieces

    //assign the pieces to each array and distribute into merge
    int *arr_part = malloc(size * sizeof(int));
    MPI_Scatter(arrNumbers, size, MPI_INT, arr_part, size, MPI_INT, 0, MPI_COMM_WORLD);
    int *arr_temp = malloc(size * sizeof(int)); //each calls merge sort
    mergeSort(arr_part, arr_temp, 0, (size - 1));

    int *arr_sorted = NULL;
    if (rank == 0)
    {
        //rank 0
        arr_sorted = malloc(n * sizeof(int));
    }

    MPI_Gather(arr_part, size, MPI_INT, arr_sorted, size, MPI_INT, 0, MPI_COMM_WORLD); //collects the results of each path

    //recieve the final two parts
    //merge them
    if (rank == 0)
    {
        int *other_array = malloc(n * sizeof(int));
        mergeSort(arr_sorted, other_array, 0, (n - 1));

        printf("This is the sorted array: ");
        //print the final list
        for (i = 0; i < n; i++)
        {
            printf("%d ", arr_sorted[i]);
        }

        printf("\n");
        printf("\n");

        free(arr_sorted);
        free(other_array);
    }
    free(arrNumbers);
    free(arr_part);
    free(arr_temp);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}
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
void merge(int *arr1, int *arr2, int s1, int s2, int s3) //sourced from internet
{

    int h, i, j, k;
    h = s1;
    i = s1;
    j = s2 + 1;

    while ((h <= s2) && (j <= s3))
    {
        if (arr1[h] <= arr1[j])
        {
            arr2[i] = arr1[h];
            h++;
        }
        else
        {
            arr2[i] = arr1[j];
            j++;
        }
        i++;
    }
    if (s2 < h)
    {
        for (k = j; k <= s3; k++)
        {
            arr2[i] = arr1[k];
            i++;
        }
    }
    else
    {
        for (k = h; k <= s2; k++)
        {
            arr2[i] = arr1[k];
            i++;
        }
    }
    for (k = s1; k <= s3; k++)
    {
        arr1[k] = arr2[k];
    }
}
void mergeSort(int *arr1, int *arr2, int sz1, int sz2)
{
    int m;
    if (sz1 < sz2)
    {
        m = (sz1 + sz2) / 2;
        mergeSort(arr1, arr2, sz1, m);
        mergeSort(arr1, arr2, (m + 1), sz2);
        merge(arr1, arr2, sz1, m, sz2);
    }
}