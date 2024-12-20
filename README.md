# Longest Common Subsequence (LCS) Implementations

## Compilation

To compile the project, run the following command:

make

This will generate the following 5 executables:

1. lcs_serial  
   - The serial version implementation.

2. lcs_parallel_columns_1  
   - The first implementation of the parallel version (partitioned by columns).

3. lcs_parallel_columns_2  
   - The second implementation of the parallel version (partitioned by columns).

4. lcs_parallel_diagonal  
   - The parallel version implementation partitioned by diagonals.

5. lcs_mpi  
   - The MPI-based implementation for distributed systems.

---

## Generating Input Files

Use the provided Python script to generate input data:

python3 inputs/generator.py <length> <output_file>

### Example:

python3 inputs/generator.py 5000 inputs/rand_5000

This will create a random input file of length 5000 and save it as inputs/rand_5000.

---

## Testing

To test the executables, use the following commands:

### Serial Version:

./lcs_serial --nThreads <number_of_threads> --inputFile <path_to_input_file>

### Parallel Versions:

1. Partitioned by Columns (Implementation 1):

   ./lcs_parallel_columns_1 --nThreads <number_of_threads> --inputFile <path_to_input_file>

2. Partitioned by Columns (Implementation 2):

   ./lcs_parallel_columns_2 --nThreads <number_of_threads> --inputFile <path_to_input_file>

3. Partitioned by Diagonals:

   ./lcs_parallel_diagonal --nThreads <number_of_threads> --inputFile <path_to_input_file>

### MPI Version:

mpirun -np <number_of_processes> ./lcs_mpi --inputFile <path_to_input_file>

---

## Examples:

1. Run the first parallel version (columns implementation) with 4 threads:

   ./lcs_parallel_columns_1 --nThreads 4 --inputFile inputs/rand_5000

2. Run the MPI version with 8 processes:

   mpirun -np 8 ./lcs_mpi --inputFile inputs/rand_5000
