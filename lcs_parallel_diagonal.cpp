#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include "core/get_time.h"
#include "core/utils.h"
#include <thread>
#include <atomic>


using namespace std;

#define DEFAULT_NUMBER_OF_THREADS "1"

vector<string> splitString(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

inline void longestCommonSubsequence(uint threadID, const string& s1, const string& s2, vector<vector<int>>& dp, CustomBarrier* barrier,
                    uint number_of_threads) {
    int n = s1.size();
    int m = s2.size();
    int diagonal = 1;

    while (true) {
        // Get the current diagonal index
        if (diagonal > n + m - 1) {
            return;
        }

        // The process is we takeing diagonal which the threads will we working on
        // And base on that diagonal, we will divide the work to the thread
        // Because these elements in diagonal are not dependent
        // On any elemnt in same diagonal, we take that advantage to create
        // Parallel algorithm
        // The way we know that two elemnts (x1, y1) and (x2, y2) on the same diagonal 
        // If x1 + y1 == x2 + y2 and they will align on diagonal (x1 + y1 - 1)
        // For example: (1, 2) and (2, 1) on the same diagonal #2
        int size = min(diagonal, n) - max(1, diagonal - m + 1) + 1; // Size of current diagonal
        int chunkSize = (size + number_of_threads - 1) / number_of_threads; // Divide diagonal into chunks

        // Base start and end for the diagonal
        int base_start = max(1, diagonal - m + 1);
        int base_end = min(n, diagonal);

        // Calculate thread-specific start and end
        int start = base_start + threadID * chunkSize;
        int end = min(base_end + 1, start + chunkSize);

        // Compute the segment for this thread
        for (int idx = start; idx < end; ++idx) {
            int i = idx;            // Row index
            int j = diagonal - i + 1; // Column index

            if (i >= 1 && j >= 1 && i <= n && j <= m) { // Ensure valid indices
                if (s1[i - 1] == s2[j - 1]) {
                    dp[i][j] = dp[i - 1][j - 1] + 1; // Match
                } else {
                    dp[i][j] = max(dp[i - 1][j], dp[i][j - 1]); // No match
                }
            }
        }
        barrier->wait();
        diagonal++;
    }
}

pair<int, string> longestCommonSubsequenceParallel(const string& s1, const string& s2, int number_of_threads) {
  std::thread t[number_of_threads];
  CustomBarrier barrier(number_of_threads);

  int n = s1.size();
  int m = s2.size();

  vector<vector<int>> dp(n + 1, vector<int>(m + 1, 0));

  uint min_columns_for_each_thread = n / number_of_threads;
  uint excess_columns = n % number_of_threads;
  uint curr_column = 1;

  for (int i = 0; i < number_of_threads; i++) {
    t[i] = std::thread(longestCommonSubsequence, i, s1, s2, ref(dp), &barrier, number_of_threads);
  }

  for (int i = 0; i < number_of_threads; i++) {
    t[i].join();
  }

    string lcs = "";
    int i = n, j = m;
    while (i > 0 && j > 0) {
        if (s1[i - 1] == s2[j - 1]) {
            lcs += s1[i - 1]; 
            --i;
            --j;
        } else if (dp[i - 1][j] > dp[i][j - 1]) {
            --i;
        } else {
            --j;
        }
    }
    reverse(lcs.begin(), lcs.end());
    return {dp[n][m], lcs};
}

int main(int argc, char* argv[]) {
    // Initialize command line arguments
    cxxopts::Options options("Longest_common_subsequence",
                            "Calculate longest common subsequence using serial and parallel execution");
    options.add_options(
      "custom",
      {
          {"nThreads", "Number of threads",
           cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_THREADS)},
           {"inputFile", "Input file path for random numbers file", 
           cxxopts::value<std::string>()->default_value("inputs/rand_5000")},
      });
    auto cl_options = options.parse(argc, argv);
    int n_threads = cl_options["nThreads"].as<uint>();
    std::string input_file_path = cl_options["inputFile"].as<std::string>();

    ifstream inputFile(input_file_path);
    if (!inputFile) {
        cerr << "Error: Could not open file '" << input_file_path << "'." << endl;
        return 1;
    }

    string sequence1, sequence2;

    getline(inputFile, sequence1);
    getline(inputFile, sequence2);
    inputFile.close();

    vector<string> seq1Tokens = splitString(sequence1, ',');
    vector<string> seq2Tokens = splitString(sequence2, ',');

    string s1 = "";
    string s2 = "";
    for (const auto& ch : seq1Tokens) s1 += ch;
    for (const auto& ch : seq2Tokens) s2 += ch;

    timer timer;

    timer.start();
    pair<int, string> lcs = longestCommonSubsequenceParallel(s1, s2, n_threads);
    double time_taken = timer.stop();

    // Output the results
    cout << "Length of Longest Common Subsequence: " << lcs.first << endl;
    cout << "Longest Common Subsequence: " << lcs.second << endl;
    cout << "Time taken: " << time_taken << endl;

    return 0;
}
