#include <iostream>
#include <chrono>
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <string>

std::unordered_map<std::string, long> durations;

void writeExecutionTime(const char* functionName, const std::chrono::high_resolution_clock::time_point& start) {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    durations[functionName] += duration;
}

void finalizeExecutionTimes() {
    FILE* file;
    errno_t err = fopen_s(&file, "execution_times.csv", "a");
    if (err == 0 && file) {
        fprintf(file, "FunctionName,fibonacciDP,fibonacciRecursive,\n");

        // Check if both functions are measured
        bool dpFound = false;
        bool recursiveFound = false;

        // Write durations
        for (const auto& pair : durations) {
            if (pair.first == "fibonacciDP") {
                fprintf(file, "Duration(us),%ld,", pair.second);
                dpFound = true;
            }
            else if (pair.first == "fibonacciRecursive") {
                fprintf(file, "%ld\n", pair.second);
                recursiveFound = true;
            }
        }

        // Handle case where one or both functions were not found
        if (!dpFound) {
            fprintf(file, "Duration(us),,\n");
        }
        if (!recursiveFound) {
            fprintf(file, ",%ld\n", 0); // Assuming 0 if not found
        }

        fclose(file);
    }
    else {
        std::cerr << "Error opening file for writing." << std::endl;
    }
}

int fibonacciRecursive(int n) {
    if (n <= 1) return n;
    return fibonacciRecursive(n - 1) + fibonacciRecursive(n - 2);
}

int fibonacciDP(int n) {
    if (n <= 1) return n;
    std::vector<int> dp(n + 1);
    dp[0] = 0;
    dp[1] = 1;
    for (int i = 2; i <= n; ++i) {
        dp[i] = dp[i - 1] + dp[i - 2];
    }
    return dp[n];
}

int main() {
    int number = 30;

    // Run and measure fibonacciDP
    auto startDP = std::chrono::high_resolution_clock::now();
    fibonacciDP(number);
    writeExecutionTime("fibonacciDP", startDP);

    // Run and measure fibonacciRecursive
    auto startRecursive = std::chrono::high_resolution_clock::now();
    fibonacciRecursive(number);
    writeExecutionTime("fibonacciRecursive", startRecursive);

    // Finalize and write execution times to CSV
    finalizeExecutionTimes();

    return 0;
}
