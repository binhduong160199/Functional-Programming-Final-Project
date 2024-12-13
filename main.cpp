#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#include "header.h"
#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
    doctest::Context context;

    std::cout << "\nRunning tests...\n";
    int testResult = context.run();
    if (testResult != 0) {
        std::cerr << "\nSome test cases failed. Check the details above.\n";
        return testResult;
    }
    std::cout << "\nAll test cases passed!\n";

    try {
        std::cout << "\nEnter the path to the input file: ";
        std::string inputPath;
        std::getline(std::cin, inputPath);

        const std::string outputPath = "output.txt";

        std::cout << "\n=== Sequential Processing ===\n";
        processFileWithTiming(inputPath, outputPath, false);

        std::cout << "\n=== Parallel Processing ===\n";
        processFileWithTiming(inputPath, outputPath, true);

    } catch (const std::exception& e) {
        std::cerr << "Error in main: " << e.what() << "\n";
        return 1;
    }
    return 0;
}