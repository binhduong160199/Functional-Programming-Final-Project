#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#include "header.h"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <cctype>
#include <iterator>

// Read the file into a string
std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

// Trim leading and trailing apostrophes from a word
std::string trimApostrophes(const std::string& word) {
    size_t start = word.find_first_not_of("'");
    size_t end = word.find_last_not_of("'");
    return (start == std::string::npos) ? "" : word.substr(start, end - start + 1);
}

// Tokenize the text
std::vector<std::string> tokenize(const std::string& text) {
    // Step 1: Convert the text to lowercase and replace unwanted characters
    auto preprocess = [](const std::string& input) {
        return std::accumulate(input.begin(), input.end(), std::string(), [](std::string acc, unsigned char c) {
            acc.push_back((std::isalnum(c) || c == '\'') ? std::tolower(c) : ' ');
            return acc;
        });
    };

    // Step 2: Split the processed text into words
    auto splitIntoWords = [](const std::string& processed) {
        std::istringstream iss(processed);
        return std::vector<std::string>{std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>()};
    };

    // Step 3: Clean up leading and trailing apostrophes
    auto cleanTokens = [](const std::vector<std::string>& tokens) {
        return std::accumulate(tokens.begin(), tokens.end(), std::vector<std::string>(), [](std::vector<std::string> acc, const std::string& token) {
            std::string cleaned = trimApostrophes(token);
            if (!cleaned.empty()) acc.push_back(cleaned);
            return acc;
        });
    };

    // Compose the steps
    return cleanTokens(splitIntoWords(preprocess(text)));
}

// Parallel tokenization
std::vector<std::string> parallelTokenize(const std::string& text) {
    // Helper function to adjust the midpoint
    auto adjustMid = [](size_t mid, const std::string& txt) {
        while (mid > 0 && std::isalnum(txt[mid])) --mid;
        return mid;
    };

    size_t mid = adjustMid(text.size() / 2, text);

    // Launch parallel tasks for tokenizing the text
    auto future1 = std::async(std::launch::async, tokenize, text.substr(0, mid));
    auto future2 = std::async(std::launch::async, tokenize, text.substr(mid));

    auto words1 = future1.get();
    auto words2 = future2.get();

    // Use std::accumulate to combine the vectors without requiring mutable references
    return std::accumulate(
        words2.begin(), words2.end(), std::move(words1),
        [](std::vector<std::string> acc, const std::string& word) {
            acc.push_back(word);
            return acc;
        }
    );
}

// Write sorted words to a file
void writeToFile(const std::string& filePath, const std::vector<std::string>& words) {
    try {
        // Open the file for writing
        std::ofstream file(filePath, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            throw std::ios_base::failure("Failed to open file: " + filePath);
        }

        // Combine all words into a single string, separating them with newlines, and add a trailing newline
        const std::string content = std::accumulate(
            words.begin(), words.end(), std::string(),
            [](const std::string& acc, const std::string& word) {
                return acc + (acc.empty() ? "" : "\n") + word;
            }
        ) + "\n"; // Add a final newline for consistency

        // Write the combined string to the file
        file << content;

    } catch (const std::exception& e) {
        std::cerr << "Error while writing to file: " << e.what() << "\n";
        throw; // Rethrow the exception for the caller to handle
    }
}

// **TEST CASES**
TEST_CASE("Test File Reading") {
    std::ofstream testFile("test_file.txt");
    testFile << "Hello world! Functional programming in C++.";
    testFile.close();

    CHECK(readFile("test_file.txt") == "Hello world! Functional programming in C++.");
    std::filesystem::remove("test_file.txt");
}

TEST_CASE("Test Tokenization") {
    std::string text = "Hello, world! Functional-programming in C++.";
    auto words = tokenize(text);

    std::vector<std::string> expected{"hello", "world", "functional", "programming", "in", "c"};
    REQUIRE(words == expected);
}

TEST_CASE("Test Parallel Tokenization") {
    // Define the input text for testing
    std::string text = "Hello, world! Functional-programming in C. Let's test this!";

    // Call the function to tokenize the input text in parallel
    auto words = parallelTokenize(text);

    // Define the expected output after tokenization
    std::vector<std::string> expected{"hello", "world", "functional", "programming", "in", "c", "let's", "test", "this"};

    // Compare the result using functional programming style
    bool match = std::equal(words.begin(), words.end(), expected.begin());

    // If the result doesn't match, print debugging information
    if (!match) {
        std::cerr << "\nParallel Tokenization Failed!\n"
                  << "Expected: " << std::accumulate(expected.begin(), expected.end(), std::string(),
                                                    [](const std::string& acc, const std::string& word) {
                                                        return acc + (acc.empty() ? "" : " ") + word;
                                                    })
                  << "\nActual:   " << std::accumulate(words.begin(), words.end(), std::string(),
                                                       [](const std::string& acc, const std::string& word) {
                                                           return acc + (acc.empty() ? "" : " ") + word;
                                                       })
                  << "\n";
    }

    // Use a functional approach for the assertion
    REQUIRE(match);
}

TEST_CASE("Test Red-Black Tree Insertion") {
    ImmutableRedBlackTree<std::string> tree;

    tree = tree.insert("functional");
    tree = tree.insert("programming");
    tree = tree.insert("in");
    tree = tree.insert("c");
    tree = tree.insert("functional"); // Duplicate

    auto sorted = tree.getSortedValues();
    std::vector<std::string> expected{"c", "functional", "in", "programming"};
    CHECK(sorted == expected);
}

TEST_CASE("Test Writing to File") {
    std::string filePath = "output_test.txt";
    std::vector<std::string> words{"apple", "banana", "cherry"};

    writeToFile(filePath, words);

    std::ifstream file(filePath);
    std::ostringstream content;
    content << file.rdbuf();

    std::string expected = "apple\nbanana\ncherry\n";
    CHECK(content.str() == expected);

    std::filesystem::remove(filePath);
}

//processFile function to use parallel insertion with timing
void processFileWithTiming(const std::string& inputPath, const std::string& outputPath, bool useParallel = false) {
    try {
        std::cout << "Processing file: " << inputPath << "\n";

        // Step 1: Read the file
        Timer fileReadTimer;
        std::string text = readFile(inputPath);
        fileReadTimer.stop("File reading");

        // Step 2: Tokenize the text
        Timer tokenizeTimer;
        std::vector<std::string> words = useParallel ? parallelTokenize(text) : tokenize(text);
        tokenizeTimer.stop("Tokenizing");

        // Step 3: Insert into Red-Black Tree
        Timer treeConstructionTimer;
        ImmutableRedBlackTree<std::string> tree = useParallel ? parallelInsert(words) : std::accumulate(
            words.begin(), words.end(), ImmutableRedBlackTree<std::string>(),
            [](const ImmutableRedBlackTree<std::string>& tree, const std::string& word) { return tree.insert(word); });
        treeConstructionTimer.stop("Tree construction");

        // Step 4: Retrieve sorted words
        Timer traversalTimer;
        std::vector<std::string> sortedWords = tree.getSortedValues();
        traversalTimer.stop("Tree traversal");

        // Step 5: Write to output file
        Timer fileWriteTimer;
        writeToFile(outputPath, sortedWords);
        fileWriteTimer.stop("File writing");

        std::cout << "Processing completed. Results written to " << outputPath << ".\n";
    } catch (const std::exception& e) {
        std::cerr << "Error during file processing: " << e.what() << "\n";
    }
}

int main(int argc, char** argv) {
    // Run tests
    doctest::Context context;

    std::cout << "\nRunning tests...\n";
    int testResult = context.run(); // Run doctest
    if (testResult != 0) {
        std::cerr << "\nSome test cases failed. Check the details above.\n";
        return testResult;
    }
    std::cout << "\nAll test cases passed!\n";

    // Performance comparison
    try {
        // Prompt user for input file path
        std::cout << "\nEnter the path to the input file: ";
        std::string inputPath;
        std::getline(std::cin, inputPath);

        const std::string outputPath = "output.txt";

        // Sequential processing
        std::cout << "\n=== Sequential Processing ===\n";
        processFileWithTiming(inputPath, outputPath, false);

        // Parallel processing
        std::cout << "\n=== Parallel Processing ===\n";
        processFileWithTiming(inputPath, outputPath, true);

    } catch (const std::exception& e) {
        std::cerr << "Error in main: " << e.what() << "\n";
        return 1;
    }

    return 0;
}