#include "header.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iterator>
#include <stdexcept>
#include <iostream>

// Timer class implementation
// Constructor: Initializes the timer by recording the current time
Timer::Timer() {
    start_time = std::chrono::high_resolution_clock::now();
}

// Stops the timer and calculates the elapsed time since the timer started
void Timer::stop(const std::string& processName) {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << processName << " took " << duration << "ms.\n"; // Prints the duration with the process name
}

// Read the file into a string
// Reads the entire content of a file specified by `filePath` into a single string
std::string readFile(const std::string& filePath) {
    // Check if the file exists, otherwise throw an error
    if (!std::filesystem::exists(filePath)) {
        throw std::runtime_error("File does not exist: " + filePath);
    }
    try {
        // Open the file in binary mode and move the cursor to the end to get the size
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::ios_base::failure("Failed to open file: " + filePath);
        }

        auto fileSize = file.tellg(); // Get the file size
        file.seekg(0, std::ios::beg); // Move the cursor back to the beginning

        // Allocate a string to hold the content and read it into the string
        std::string content(static_cast<size_t>(fileSize), '\0');
        file.read(content.data(), fileSize);

        return content; // Return the file content as a string
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to read file: " + std::string(e.what()));
    }
}

// Trim leading and trailing apostrophes from a word
std::string trimApostrophes(const std::string& word) {
    if (word.empty()) return ""; // If the word is empty, return an empty string

    // Find the first and last characters that are not apostrophes
    auto start = word.find_first_not_of("'");
    auto end = word.find_last_not_of("'");
    // If the word consists only of apostrophes, return an empty string
    return (start == std::string::npos || end == std::string::npos) ? "" : word.substr(start, end - start + 1);
}

// Tokenize the text into words
// Converts the input text to lowercase, removes punctuation, and splits it into words
std::vector<std::string> tokenize(const std::string& text) {
    // Preprocess the text: Replace non-alphanumeric characters with spaces and convert to lowercase
    auto preprocess = [](const std::string& input) {
        return std::accumulate(input.begin(), input.end(), std::string(), [](std::string acc, unsigned char c) {
            acc.push_back((std::isalnum(c) || c == '\'') ? std::tolower(c) : ' '); // Keep alphanumeric characters and apostrophes
            return acc;
        });
    };

    // Split the processed string into individual words
    auto splitIntoWords = [](const std::string& processed) {
        std::istringstream iss(processed); // Use stringstream for splitting
        return std::vector<std::string>{std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>()};
    };

    // Clean the tokens: Trim apostrophes and remove empty tokens
    auto cleanTokens = [](const std::vector<std::string>& tokens) {
        return std::accumulate(tokens.begin(), tokens.end(), std::vector<std::string>(), [](std::vector<std::string> acc, const std::string& token) {
            auto cleaned = trimApostrophes(token); // Trim apostrophes from each word
            if (!cleaned.empty()) acc.push_back(std::move(cleaned)); // Only add non-empty words
            return acc;
        });
    };

    return cleanTokens(splitIntoWords(preprocess(text))); // Apply all steps and return the final list of words
}

// Parallel tokenization of the text
// Splits the text into two parts, processes them in parallel, and merges the results
std::vector<std::string> parallelTokenize(const std::string& text) {
    if (text.empty()) return {}; // If the input text is empty, return an empty vector
    if (text.size() < 2) return tokenize(text); // If the text is very short, fallback to sequential tokenization

    // Find a suitable split point (avoid splitting inside a word)
    size_t mid = text.size() / 2;
    while (mid > 0 && (std::isalnum(text[mid]) || text[mid] == '\'')) --mid;

    // Launch parallel tasks to tokenize each half of the text
    auto future1 = std::async(std::launch::async, tokenize, text.substr(0, mid)); // Tokenize the first half
    auto future2 = std::async(std::launch::async, tokenize, text.substr(mid)); // Tokenize the second half

    // Wait for both tasks to complete and get the results
    auto words1 = future1.get();
    auto words2 = future2.get();

    // Merge tokens that might have been split across the split point
    if (!words1.empty() && !words2.empty() && (std::isalnum(text[mid]) || text[mid] == '\'')) {
        words1.back() += words2.front(); // Merge the last word of the first half with the first word of the second half
        words2.erase(words2.begin()); // Remove the merged word from the second list
    }

    // Combine the two lists of words into one
    return std::accumulate(
        words2.begin(), words2.end(), std::move(words1),
        [](std::vector<std::string> acc, const std::string& word) {
            acc.push_back(word);
            return acc;
        }
    );
}

// Write sorted words to a file
// Outputs the vector of words to a file, with one word per line
void writeToFile(const std::string& filePath, const std::vector<std::string>& words) {
    if (words.empty()) return; // If there are no words, do nothing

    std::ofstream file(filePath, std::ios::out | std::ios::trunc); // Open the file for writing
    if (!file.is_open()) {
        throw std::ios_base::failure("Failed to open file: " + filePath);
    }

    // Write each word to a new line
    for (const auto& word : words) {
        file << word << '\n';
    }
}

// Process the file and time each step
// Reads the input file, tokenizes its content, inserts words into a Red-Black Tree, and writes the sorted output
void processFileWithTiming(const std::string& inputPath, const std::string& outputPath, bool useParallel) {
    try {
        std::cout << "Processing file: " << inputPath << "\n";

        Timer totalTimer; // Timer to measure the total processing time

        // Step 1: Read the file
        Timer readTimer;
        std::string content = readFile(inputPath);
        readTimer.stop("Reading File");

        // Step 2: Tokenize the text (sequential or parallel)
        Timer tokenizeTimer;
        auto tokens = useParallel ? parallelTokenize(content) : tokenize(content);
        tokenizeTimer.stop("Tokenization");

        // Step 3: Insert tokens into a Red-Black Tree
        Timer treeTimer;
        auto tree = useParallel ? parallelInsert(tokens) : std::accumulate(tokens.begin(), tokens.end(), RBTree<std::string>(),
            [](const RBTree<std::string>& t, const std::string& s) { return t.insert(s); });
        treeTimer.stop("Tree Construction");

        // Step 4: Retrieve sorted words from the tree
        Timer sortTimer;
        auto sorted = tree.getSortedValues();
        sortTimer.stop("Sorting");

        // Step 5: Write sorted words to the output file
        Timer writeTimer;
        writeToFile(outputPath, sorted);
        writeTimer.stop("Writing File");

        totalTimer.stop("Total Processing"); // Stop the total timer and print the result
    } catch (const std::exception& e) {
        std::cerr << "Error processing file: " << e.what() << "\n"; // Handle errors gracefully
    }
}