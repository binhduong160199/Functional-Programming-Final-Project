#include "doctest.h"
#include "header.h"
#include <filesystem>
#include <cctype>
#include <random>
#include <fstream>

// Helper function to generate a valid file with specific content
// This is used to create a temporary file for testing purposes
std::string generateValidFile(const std::string& content) {
    auto tempPath = std::filesystem::temp_directory_path() / ("test_file_" + std::to_string(std::rand()) + ".txt");
    std::ofstream file(tempPath);
    file << content;  // Write the given content to the file
    file.close();     // Close the file to flush the data
    return tempPath;  // Return the path of the created file
}

// Helper function to generate a non-existent file path for invalid path testing
std::string generateInvalidFilePath() {
    return "/invalid/path/to/file_" + std::to_string(std::rand()) + ".txt";
}

// Test cases for the `readFile` function
TEST_CASE("readFile Function - Property-Based Testing") {
    // Test with a valid file
    auto testValidFile = []() {
        std::string content = "This is a test file.";  // Define test content
        auto filePath = generateValidFile(content);   // Generate a temporary file with the content
        CHECK(readFile(filePath) == content);         // Verify the file content matches the expected value
        std::filesystem::remove(filePath);            // Clean up the temporary file
    };

    // Test with an invalid file path
    auto testInvalidFile = []() {
        auto invalidPath = generateInvalidFilePath();  // Generate an invalid path
        CHECK_THROWS_AS(readFile(invalidPath), std::runtime_error);  // Check if the correct exception is thrown
    };

    // Execute the test cases
    testValidFile();
    testInvalidFile();
}

// Helper function to generate random strings with apostrophes
std::string generateRandomStringWithApostrophes(size_t coreLength) {
    const std::string characters = "abcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, characters.size() - 1);

    // Generate the main core of the random string
    std::string core = std::accumulate(
        std::vector<size_t>(coreLength).begin(), std::vector<size_t>(coreLength).end(),
        std::string(),
        [&](std::string acc, size_t) {
            return acc + characters[dist(gen)];
        });

    // Add random leading and trailing apostrophes
    size_t leadingApostrophes = dist(gen) % 5; // Up to 4 leading apostrophes
    size_t trailingApostrophes = dist(gen) % 5; // Up to 4 trailing apostrophes

    return std::string(leadingApostrophes, '\'') + core + std::string(trailingApostrophes, '\'');
}

// Helper function to generate strings consisting only of apostrophes
std::string generateAllApostrophesString(size_t length) {
    return std::string(length, '\'');  // Generate a string of a specific length filled with apostrophes
}

// Test cases for the `trimApostrophes` function
TEST_CASE("trimApostrophes Function") {
    // Test random strings with apostrophes
    auto testRandomStringsWithApostrophes = []() {
        size_t coreLength = 10;  // Define the length of the random core string
        auto randomString = generateRandomStringWithApostrophes(coreLength);  // Generate a random test string
        auto trimmed = trimApostrophes(randomString);  // Trim the apostrophes

        // Check that the trimmed string has no leading or trailing apostrophes
        CHECK(trimmed.find_first_of("'") != 0);  // No leading apostrophes
        CHECK(trimmed.find_last_of("'") != trimmed.size() - 1);  // No trailing apostrophes

        // Check the trimmed string length is not longer than the original
        CHECK(trimmed.size() <= randomString.size());

        // Verify the core characters remain intact
        CHECK(trimmed.find_first_not_of("'") != std::string::npos);
    };

    // Test strings containing only apostrophes
    auto testAllApostrophesString = []() {
        size_t length = 10;  // Define the length of the test string
        auto allApostrophes = generateAllApostrophesString(length);  // Generate a string of only apostrophes
        CHECK(trimApostrophes(allApostrophes).empty());  // Verify trimming results in an empty string
    };

    // Test an empty input
    auto testEmptyInput = []() {
        CHECK(trimApostrophes("").empty());  // Check that trimming an empty string returns an empty string
    };

    // Execute the test cases multiple times for random input
    for (int i = 0; i < 5; ++i) {
        testRandomStringsWithApostrophes();
    }
    testAllApostrophesString();
    testEmptyInput();
}

// Helper function to generate random text with special characters and spaces
std::string generateRandomText(size_t length) {
    const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'.,!? ";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, characters.size() - 1);

    // Generate a random string of specified length
    return std::accumulate(
        std::vector<size_t>(length).begin(), std::vector<size_t>(length).end(),
        std::string(),
        [&](std::string acc, size_t) {
            return acc + characters[dist(gen)];
        });
}

// Test cases for the `tokenize` function
TEST_CASE("tokenize Function") {
    // Test with random text
    auto testRandomText = []() {
        size_t randomLength = 50;  // Define the length of the random text
        auto randomText = generateRandomText(randomLength);  // Generate a random test string
        auto tokens = tokenize(randomText);  // Tokenize the text

        // Verify that tokens are valid words
        CHECK(std::all_of(tokens.begin(), tokens.end(), [](const std::string& token) {
            return !token.empty() && std::all_of(token.begin(), token.end(), [](char c) {
                return std::isalnum(c) || c == '\'';  // Tokens must contain alphanumeric or apostrophe characters
            });
        }));
    };

    // Run the test multiple times for better coverage
    for (int i = 0; i < 5; ++i) {
        testRandomText();
    }
}

// Generate random text with varying complexity
std::string generateComplexRandomText(size_t length) {
    const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'.,!? ";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, characters.size() - 1);

    return std::accumulate(
        std::vector<size_t>(length).begin(), std::vector<size_t>(length).end(),
        std::string(),
        [&](std::string acc, size_t) {
            return acc + characters[dist(gen)];
        });
}

// Generate structured text for testing parallelTokenize
std::string generateKnownStructuredText() {
    return "Parallel tokenization should match single-threaded tokenization exactly.";
}

// Generate edge case inputs for parallel tokenization testing
std::vector<std::string> generateParallelEdgeCases() {
    return {
        "",                             // Empty string
        " ",                            // String with spaces only
        "!!!???",                       // Special characters only
        "Parallel123Test'Example'",     // Mixed alphanumeric and apostrophes
        "VeryVeryVeryLongSingleWord"    // A single very long word
    };
}

// Test cases for the `parallelTokenize` function
TEST_CASE("parallelTokenize Function") {
    // Test with structured input
    auto testStructuredText = []() {
        std::string text = generateKnownStructuredText();  // Generate structured test string
        auto parallelTokens = parallelTokenize(text);      // Tokenize using parallel method
        auto sequentialTokens = tokenize(text);           // Tokenize using sequential method
        CHECK(parallelTokens == sequentialTokens);         // Ensure results match
    };

    // Test with complex random input
    auto testComplexRandomText = []() {
        size_t randomLength = 100;  // Moderate size for random input
        auto randomText = generateComplexRandomText(randomLength);  // Generate random text
        auto parallelTokens = parallelTokenize(randomText);         // Parallel tokenization
        auto sequentialTokens = tokenize(randomText);               // Sequential tokenization
        CHECK(parallelTokens == sequentialTokens);                  // Verify consistency
    };

    // Test edge cases for parallel tokenization
    auto testEdgeCases = []() {
        auto edgeCases = generateParallelEdgeCases();  // Generate edge case inputs
        for (const auto& text : edgeCases) {
            auto parallelTokens = parallelTokenize(text);  // Tokenize in parallel
            auto sequentialTokens = tokenize(text);        // Tokenize sequentially
            CHECK(parallelTokens == sequentialTokens);      // Ensure results are consistent
        }
    };

    // Test with large input to evaluate performance
    auto testLargeInput = []() {
        size_t largeLength = 1000;  // Define large input size
        auto largeText = generateComplexRandomText(largeLength);  // Generate large random text
        auto parallelTokens = parallelTokenize(largeText);        // Tokenize in parallel
        auto sequentialTokens = tokenize(largeText);              // Tokenize sequentially
        CHECK(parallelTokens == sequentialTokens);                // Verify tokenization results
    };

    // Execute all test cases
    testStructuredText();
    for (int i = 0; i < 5; ++i) {  // Repeat random tests for better coverage
        testComplexRandomText();
    }
    testEdgeCases();
    testLargeInput();
}

// Helper function to generate random integers for RBTree testing
std::vector<int> generateRandomIntegers(size_t count, int minValue = 0, int maxValue = 1000) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(minValue, maxValue);

    // Generate a vector of random integers
    std::vector<int> values(count);
    std::generate(values.begin(), values.end(), [&]() { return dist(gen); });
    return values;
}

// Test cases for the `RBTree` class
TEST_CASE("RBTree") {
    // Test insertion of random values and verify sorted order
    SUBCASE("Insert Random Values and Verify Sorted Order") {
        size_t count = 100;  // Number of random values to insert
        auto randomValues = generateRandomIntegers(count);  // Generate random integers

        RBTree<int> tree;  // Initialize an empty RBTree
        for (const auto& value : randomValues) {
            tree = tree.insert(value);  // Insert each value into the tree
        }

        // Verify the tree's sorted order
        auto sortedValues = tree.getSortedValues();  // Retrieve sorted values from the tree
        auto expectedValues = randomValues;          // Copy the random values
        std::sort(expectedValues.begin(), expectedValues.end());  // Sort the values
        expectedValues.erase(std::unique(expectedValues.begin(), expectedValues.end()), expectedValues.end());  // Remove duplicates
        CHECK(sortedValues == expectedValues);  // Check if the tree's values match the expected sorted values
    }

    // Test insertion of a large dataset into the RBTree
    SUBCASE("Test Large Input with Random Data") {
        size_t count = 1000;  // Number of random values for large input test
        auto randomValues = generateRandomIntegers(count);  // Generate random integers

        RBTree<int> tree;  // Initialize an empty RBTree
        for (const auto& value : randomValues) {
            tree = tree.insert(value);  // Insert each value
        }

        // Verify the sorted order
        auto sortedValues = tree.getSortedValues();
        auto expectedValues = randomValues;
        std::sort(expectedValues.begin(), expectedValues.end());
        expectedValues.erase(std::unique(expectedValues.begin(), expectedValues.end()), expectedValues.end());
        CHECK(sortedValues == expectedValues);  // Validate against expected values
    }

    // Test an empty tree
    SUBCASE("Edge Case: Empty Tree") {
        RBTree<int> tree;  // Create an empty tree

        auto sortedValues = tree.getSortedValues();  // Retrieve values from the tree
        CHECK(sortedValues.empty());  // Ensure the tree is empty
    }

    // Test insertion of a single element
    SUBCASE("Edge Case: Single Element") {
        RBTree<int> tree;  // Initialize an empty RBTree
        tree = tree.insert(42);  // Insert a single value

        // Verify the sorted order
        auto sortedValues = tree.getSortedValues();
        CHECK(sortedValues == std::vector<int>{42});  // Ensure the tree contains only the inserted value
    }
}