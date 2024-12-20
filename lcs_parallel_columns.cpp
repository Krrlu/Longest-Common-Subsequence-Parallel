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
std::thread *t;
std::atomic<int> activeThreadID(0);

vector<string> splitString(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

inline void longestCommonSubsequence(int threadID, const string& s1, const string& s2, vector<vector<int>>& dp, 
                              std::vector<int>& startx, std::vector<int>& endx, int number_of_threads, CustomBarrier* barrier) {
    int n = s1.size();
    int m = s2.size();
    const int gap = m/number_of_threads;
    int start = startx[threadID];
    int end = endx[threadID];

    // The idea behind this improvement columns-wise method 
    // That we imagine the woek like a ladder
    // Thread 1 will process row j, Thread 2 process row (j - 1) and so on
    for (int i = 1; i <= m; i++) {
        for(int j = start; j <= end; j++) {
            if (s1[j - 1] == s2[i - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = max(dp[i - 1][j], dp[i][j - 1]);
            }
        }

        // Each thread wait after processing gap of rows
        if (i % gap == 0) {
            barrier->wait();

            // Create new thread if the current thread is the last active thread
            if (activeThreadID.load() < number_of_threads - 1 && threadID == activeThreadID.load()){
                activeThreadID.fetch_add(1);
                barrier->increment_workers();
                t[activeThreadID.load()] = std::thread(longestCommonSubsequence, activeThreadID.load(), std::ref(s1), 
                std::ref(s2), std::ref(dp), std::ref(startx), 
                std::ref(endx), number_of_threads, barrier);
            }
        }
    }

    // decrement the number of workers before exiting
    barrier->decrement_workers();
 }


pair<int, string> longestCommonSubsequenceParallel(const string& s1, const string& s2, int number_of_threads) {
  timer parallel_timer;
  double time_taken = 0.0;
  std::vector<int> startx(number_of_threads);
  std::vector<int> endx(number_of_threads);
  t = new std::thread[number_of_threads];
  CustomBarrier barrier(1);

  int n = s1.size();
  int m = s2.size();

  vector<vector<int>> dp(n + 1, vector<int>(m + 1, 0));

  uint min_columns_for_each_thread = n / number_of_threads;
  uint excess_columns = n % number_of_threads;
  uint curr_column = 1;

  for (uint i = 0; i < number_of_threads; i++) {
    startx[i] = curr_column;
    if (excess_columns > 0) {
      endx[i] = curr_column + min_columns_for_each_thread;
      excess_columns--;
      } 
    else {
           endx[i] = curr_column + min_columns_for_each_thread - 1;
      }
    curr_column = endx[i]+1;
  } 
  
  parallel_timer.start();

  // Create the first thread
  t[0] = std::thread(longestCommonSubsequence, 0, s1, s2, ref(dp), ref(startx), ref(endx), number_of_threads, &barrier);


  for (int i = 0; i < number_of_threads; i++) {
    t[i].join();
  }

    string lcs = "";
    int i = m, j = n;
    while (i > 0 && j > 0) {
        if (s1[j - 1] == s2[i - 1]) {
            lcs += s1[j - 1];
            --i;
            --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            --i;
        } else {
            --j;
        }
    }

    delete[] t;
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
