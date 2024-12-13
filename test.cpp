#include "doctest.h"
#include "header.h"
#include <filesystem>
#include <cctype>
#include <random>
#include <fstream>

// Test for Read file
std::string generateValidFile(const std::string& content) {
    auto tempPath = std::filesystem::temp_directory_path() / ("test_file_" + std::to_string(std::rand()) + ".txt");
    std::ofstream file(tempPath);
    file << content;
    file.close();
    return tempPath;
}

std::string generateInvalidFilePath() {
    return "/invalid/path/to/file_" + std::to_string(std::rand()) + ".txt";
}

TEST_CASE("readFile Function - Property-Based Testing") {
    // Test with a valid file
    auto testValidFile = []() {
        std::string content = "This is a test file.";
        auto filePath = generateValidFile(content);
        CHECK(readFile(filePath) == content);
        std::filesystem::remove(filePath);
    };

    // Test with an invalid file path
    auto testInvalidFile = []() {
        auto invalidPath = generateInvalidFilePath();
        CHECK_THROWS_AS(readFile(invalidPath), std::runtime_error);
    };

    testValidFile();
    testInvalidFile();
}

// Generate random strings with apostrophes around the core text
std::string generateRandomStringWithApostrophes(size_t coreLength) {
    const std::string characters = "abcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, characters.size() - 1);

    // Generate core text
    std::string core = std::accumulate(
        std::vector<size_t>(coreLength).begin(), std::vector<size_t>(coreLength).end(),
        std::string(),
        [&](std::string acc, size_t) {
            return acc + characters[dist(gen)];
        });

    // Add leading and trailing apostrophes
    size_t leadingApostrophes = dist(gen) % 5; // Up to 4 leading apostrophes
    size_t trailingApostrophes = dist(gen) % 5; // Up to 4 trailing apostrophes

    return std::string(leadingApostrophes, '\'') + core + std::string(trailingApostrophes, '\'');
}

// Generate strings with only apostrophes
std::string generateAllApostrophesString(size_t length) {
    return std::string(length, '\'');
}

TEST_CASE("trimApostrophes Function") {
    // Test random strings with apostrophes
    auto testRandomStringsWithApostrophes = []() {
        size_t coreLength = 10; // Length of the core random string
        auto randomString = generateRandomStringWithApostrophes(coreLength);
        auto trimmed = trimApostrophes(randomString);

        // The trimmed string should not contain leading or trailing apostrophes
        CHECK(trimmed.find_first_of("'") != 0);
        CHECK(trimmed.find_last_of("'") != trimmed.size() - 1);

        // The trimmed string length should be no longer than the original string
        CHECK(trimmed.size() <= randomString.size());

        // If there are non-apostrophe characters, they should remain intact
        CHECK(trimmed.find_first_not_of("'") != std::string::npos);
    };

    // Test strings with only apostrophes
    auto testAllApostrophesString = []() {
        size_t length = 10; // Total number of apostrophes
        auto allApostrophes = generateAllApostrophesString(length);
        CHECK(trimApostrophes(allApostrophes).empty());
    };

    // Test empty input
    auto testEmptyInput = []() {
        CHECK(trimApostrophes("").empty());
    };

    // Run tests
    for (int i = 0; i < 5; ++i) { // Run the random test multiple times for better coverage
        testRandomStringsWithApostrophes();
    }
    testAllApostrophesString();
    testEmptyInput();
}

// Generate random strings with alphanumeric characters, spaces, and special characters
std::string generateRandomText(size_t length) {
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

TEST_CASE("tokenize Function") {
    // Test with random text
    auto testRandomText = []() {
        size_t randomLength = 50; // Reasonable size to avoid excessive processing
        auto randomText = generateRandomText(randomLength);
        auto tokens = tokenize(randomText);

        // Ensure tokens are valid (no empty tokens and contain alphanumeric characters)
        CHECK(std::all_of(tokens.begin(), tokens.end(), [](const std::string& token) {
            return !token.empty() && std::all_of(token.begin(), token.end(), [](char c) {
                return std::isalnum(c) || c == '\'';
            });
        }));
    };
    for (int i = 0; i < 5; ++i) { // Run random tests multiple times for better coverage
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

// Generate text with known structure
std::string generateKnownStructuredText() {
    return "Parallel tokenization should match single-threaded tokenization exactly.";
}

// Generate edge case inputs for parallel tokenization
std::vector<std::string> generateParallelEdgeCases() {
    return {
        "",                             // Empty string
        " ",                            // Space-only
        "!!!???",                       // Special characters only
        "Parallel123Test'Example'",     // Mixed alphanumeric and apostrophes
        "VeryVeryVeryLongSingleWord"    // Single long word
    };
}

TEST_CASE("parallelTokenize Function") {
    // Test with structured input
    auto testStructuredText = []() {
        std::string text = "Parallel tokenization must match the sequential tokenization.";
        auto parallelTokens = parallelTokenize(text);
        auto sequentialTokens = tokenize(text);
        CHECK(parallelTokens == sequentialTokens);
    };

    // Test with complex random input
    auto testComplexRandomText = []() {
        size_t randomLength = 100; // Moderate size
        auto randomText = generateComplexRandomText(randomLength);
        auto parallelTokens = parallelTokenize(randomText);
        auto sequentialTokens = tokenize(randomText);
        CHECK(parallelTokens == sequentialTokens);
    };

    // Test with edge case inputs
    auto testEdgeCases = []() {
        auto edgeCases = generateParallelEdgeCases();
        for (const auto& text : edgeCases) {
            auto parallelTokens = parallelTokenize(text);
            auto sequentialTokens = tokenize(text);
            CHECK(parallelTokens == sequentialTokens);
        }
    };

    // Test with large input
    auto testLargeInput = []() {
        size_t largeLength = 1000; // Large input for parallel efficiency
        auto largeText = generateComplexRandomText(largeLength);
        auto parallelTokens = parallelTokenize(largeText);
        auto sequentialTokens = tokenize(largeText);
        CHECK(parallelTokens == sequentialTokens);
    };

    // Execute tests
    testStructuredText();
    for (int i = 0; i < 5; ++i) { // Repeat for better coverage
        testComplexRandomText();
    }
    testEdgeCases();
    testLargeInput();
}

// Data generator for random integers
std::vector<int> generateRandomIntegers(size_t count, int minValue = 0, int maxValue = 1000) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(minValue, maxValue);

    std::vector<int> values(count);
    std::generate(values.begin(), values.end(), [&]() { return dist(gen); });
    return values;
}

TEST_CASE("RBTree") {
    SUBCASE("Insert Random Values and Verify Sorted Order") {
        // Generate random integers
        size_t count = 100; // Adjust for testing size
        auto randomValues = generateRandomIntegers(count);

        // Insert values into the tree
        RBTree<int> tree;
        for (const auto& value : randomValues) {
            tree = tree.insert(value);
        }

        // Verify the output is sorted
        auto sortedValues = tree.getSortedValues();
        auto expectedValues = randomValues;
        std::sort(expectedValues.begin(), expectedValues.end());
        expectedValues.erase(std::unique(expectedValues.begin(), expectedValues.end()), expectedValues.end());
        CHECK(sortedValues == expectedValues);
    }

    SUBCASE("Test Large Input with Random Data") {
        // Generate a large dataset
        size_t count = 1000;
        auto randomValues = generateRandomIntegers(count);

        // Insert values into the tree
        RBTree<int> tree;
        for (const auto& value : randomValues) {
            tree = tree.insert(value);
        }

        // Verify sorted order
        auto sortedValues = tree.getSortedValues();
        auto expectedValues = randomValues;
        std::sort(expectedValues.begin(), expectedValues.end());
        expectedValues.erase(std::unique(expectedValues.begin(), expectedValues.end()), expectedValues.end());
        CHECK(sortedValues == expectedValues);
    }

    SUBCASE("Edge Case: Empty Tree") {
        // Create an empty tree
        RBTree<int> tree;

        // Check that the tree is empty
        auto sortedValues = tree.getSortedValues();
        CHECK(sortedValues.empty());
    }

    SUBCASE("Edge Case: Single Element") {
        // Insert a single value
        RBTree<int> tree;
        tree = tree.insert(42);

        // Verify the sorted output
        auto sortedValues = tree.getSortedValues();
        CHECK(sortedValues == std::vector<int>{42});
    }
}