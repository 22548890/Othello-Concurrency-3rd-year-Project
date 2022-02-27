echo "parallel for using critical region to sum values 0 - 10m on 1 thr"
time ./nth_tri 10000000 1 0
echo "parallel for using manual private vars to sum values 0 - 10m on 1 threads"
time ./nth_tri 10000000 1 1
echo "parallel for using reduction to sum values 0 - 10m on 1 threads"
time ./nth_tri 10000000 1 2
echo "***********************"
echo "parallel for; sum values in #pragma omp critical 2 threads"
time ./nth_tri 10000000 2 0
echo "parallel for using manual private vars to sum values 0 - 10m on 2 threads"
time ./nth_tri 10000000 2 1
echo "parallel for using reduction to sum values 0 - 10m on 2 threads"
time ./nth_tri 10000000 2 2
echo "***********************"
echo "parallel for; sum values in #pragma omp critical 4 threads"
time ./nth_tri 10000000 4 0 
echo "parallel for using manual private vars to sum values 0 - 10m on 4 threads"
time ./nth_tri 10000000 4 1 
echo "parallel for using reduction to sum values 0 - 10m on 4 threads"
time ./nth_tri 10000000 4 2 
