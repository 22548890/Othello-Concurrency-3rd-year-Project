echo "time ./omp_matmult 1 8000000 8" 
time ./omp_matmult 1 8000000 8 
echo "time ./omp_matmult 2 8000000 8" 
time ./omp_matmult 2 8000000 8 
echo "time ./omp_matmult 4 8000000 8" 
time ./omp_matmult 4 8000000 8 

echo "***********************************"
echo "***********************************"
echo "time ./omp_matmult 1 8 8000000" 
time ./omp_matmult 1 8 8000000 
echo "time ./omp_matmult 2 8 8000000" 
time ./omp_matmult 2 8 8000000 
echo "time ./omp_matmult 4 8 8000000" 
time ./omp_matmult 4 8 8000000 
