#include "header.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iterator>
#include <stdexcept>
#include <iostream>

// Timer class implementation
Timer::Timer() {
    start_time = std::chrono::high_resolution_clock::now();
}

void Timer::stop(const std::string& processName) {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << processName << " took " << duration << "ms.\n";
}

// Read the file into a string
std::string readFile(const std::string& filePath) {
    if (!std::filesystem::exists(filePath)) {
        throw std::runtime_error("File does not exist: " + filePath);
    }
    try {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::ios_base::failure("Failed to open file: " + filePath);
        }

        auto fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string content(static_cast<size_t>(fileSize), '\0');
        file.read(content.data(), fileSize);

        return content;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to read file: " + std::string(e.what()));
    }
}

// Trim leading and trailing apostrophes
std::string trimApostrophes(const std::string& word) {
    if (word.empty()) return "";
    auto start = word.find_first_not_of("'");
    auto end = word.find_last_not_of("'");
    return (start == std::string::npos || end == std::string::npos) ? "" : word.substr(start, end - start + 1);
}

// Tokenize the text
std::vector<std::string> tokenize(const std::string& text) {
    auto preprocess = [](const std::string& input) {
        return std::accumulate(input.begin(), input.end(), std::string(), [](std::string acc, unsigned char c) {
            acc.push_back((std::isalnum(c) || c == '\'') ? std::tolower(c) : ' ');
            return acc;
        });
    };

    auto splitIntoWords = [](const std::string& processed) {
        std::istringstream iss(processed);
        return std::vector<std::string>{std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>()};
    };

    auto cleanTokens = [](const std::vector<std::string>& tokens) {
        return std::accumulate(tokens.begin(), tokens.end(), std::vector<std::string>(), [](std::vector<std::string> acc, const std::string& token) {
            auto cleaned = trimApostrophes(token);
            if (!cleaned.empty()) acc.push_back(std::move(cleaned));
            return acc;
        });
    };

    return cleanTokens(splitIntoWords(preprocess(text)));
}

// Parallel tokenization
std::vector<std::string> parallelTokenize(const std::string& text) {
    if (text.empty()) return {};
    if (text.size() < 2) return tokenize(text);

    size_t mid = text.size() / 2;
    while (mid > 0 && (std::isalnum(text[mid]) || text[mid] == '\'')) --mid;

    auto future1 = std::async(std::launch::async, tokenize, text.substr(0, mid));
    auto future2 = std::async(std::launch::async, tokenize, text.substr(mid));

    auto words1 = future1.get();
    auto words2 = future2.get();

    if (!words1.empty() && !words2.empty() && (std::isalnum(text[mid]) || text[mid] == '\'')) {
        words1.back() += words2.front();
        words2.erase(words2.begin());
    }

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
    if (words.empty()) return;

    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        throw std::ios_base::failure("Failed to open file: " + filePath);
    }

    for (const auto& word : words) {
        file << word << '\n';
    }
}

// Process file with timing
void processFileWithTiming(const std::string& inputPath, const std::string& outputPath, bool useParallel) {
    try {
        std::cout << "Processing file: " << inputPath << "\n";

        Timer totalTimer;

        Timer readTimer;
        std::string content = readFile(inputPath);
        readTimer.stop("Reading File");

        Timer tokenizeTimer;
        auto tokens = useParallel ? parallelTokenize(content) : tokenize(content);
        tokenizeTimer.stop("Tokenization");

        Timer treeTimer;
        auto tree = useParallel ? parallelInsert(tokens) : std::accumulate(tokens.begin(), tokens.end(), RBTree<std::string>(),
            [](const RBTree<std::string>& t, const std::string& s) { return t.insert(s); });
        treeTimer.stop("Tree Construction");

        Timer sortTimer;
        auto sorted = tree.getSortedValues();
        sortTimer.stop("Sorting");

        Timer writeTimer;
        writeToFile(outputPath, sorted);
        writeTimer.stop("Writing File");

        totalTimer.stop("Total Processing");
    } catch (const std::exception& e) {
        std::cerr << "Error processing file: " << e.what() << "\n";
    }
}