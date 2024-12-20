#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <mpi.h>
#include "core/get_time.h"
#include "core/cxxopts.h"

void read_sequences(std::vector<char> &a, std::vector<char> &b, std::string input_file_path) {
    std::ifstream file(input_file_path);

    if (!file) {
        std::cerr << "Could not open the file!" << std::endl;
        return;
    }

    std::string line;

    // Read the first line
    if (std::getline(file, line)) {
        for (char ch : line) {
            if (ch != ',') {
                a.push_back(ch);
            }
        }
    }

    // Read the second line
    if (std::getline(file, line)) {
        for (char ch : line) {
            if (ch != ',') {
                b.push_back(ch);
            }
        }
    }
}


int main(int argc, char *argv[]) {
    // Initialize command line arguments
    cxxopts::Options options("Longest_common_subsequence",
                            "Calculate longest common subsequence using serial and parallel execution");
    options.add_options(
      "",
      {
           {"inputFile", "Input file path for random numbers file", 
           cxxopts::value<std::string>()->default_value("inputs/rand_5000")},
      });
    auto cl_options = options.parse(argc, argv);
    std::string input_file_path = cl_options["inputFile"].as<std::string>();

    // Initialize sequences a and b
    std::vector<char> a;
    std::vector<char> b;
    timer t;
    int rank, total_rank;
    int min_columns, excess_columns, start, end, row_size;

    // Read sequences from the file
    read_sequences(a, b, input_file_path);

    // Dimensions of the grid
    int m = a.size();
    int n = b.size();

    t.start();

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &total_rank);

    // Divided by columns for each process
    // This is same as the pseudocode provide in Assignment 6 heat transfer
    min_columns = n / total_rank;
    excess_columns = n % total_rank;
    if (rank < excess_columns) {
        start = rank * (min_columns + 1);
        end = start + min_columns;
    }
    else {
        start = (excess_columns * (min_columns + 1)) + ((rank-excess_columns) * min_columns);
        end = start + min_columns - 1;
    }

    // The first column in rank 0 is 0
    // and for other ranks, first column will used to store value from the last column of the previous rank
    // so we need to add 1 to the row size
    // We only need two rows at a time
    row_size = (end - start + 1) + 1;
    std::vector<int> old_row(row_size, 0);
    std::vector<int> new_row(row_size, 0);

    for (int i = 1; i < (m + 1); i++) {
        // Swap old_row and new_row
        old_row.swap(new_row);

        // Get message from process on the left, except for rank 0
        if (rank != 0) {
            int temp;
            // Tag is row number
            MPI_Recv(&temp, 1, MPI_INT, rank - 1, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Set the first column in the new row
            new_row[0] = temp;
        }

        // Calculate the new row
        // new_row corresponds to the next row of old_row
        for (int j = 1; j < row_size; j++) {
            if (a[i - 1] == b[start + j - 1]) {
                new_row[j] = old_row[j-1] + 1;
            } else {
                new_row[j] = std::max(old_row[j], new_row[j - 1]);
            }
        }

        // Send last column to process on the right, except for the last rank
        if (rank != total_rank - 1) {
            // Send the last column in current row to the process on the right
            // Tag is row number
            int temp = new_row[row_size - 1];
            MPI_Send(&temp, 1, MPI_INT, rank + 1, i, MPI_COMM_WORLD);
        }
    }


    // Print the length of LCS and time taken
    if (rank == total_rank - 1) {
        std:: cout << "Length of Longest Common Subsequence: " << new_row[row_size - 1] << std::endl;
        std:: cout << "Time taken: " << t.stop() << std::endl;
    }

    MPI_Finalize();
}
