# Persistent Red-Black Tree with Functional Programming

This project implements a persistent red-black tree to read a large text file, tokenize it, and store unique words in a balanced tree. The sorted words are written to an output file (`output.txt`). The implementation follows functional programming principles, inspired by Bartosz Milewski's article: [Functional Data Structures in C++: Trees](https://bartoszmilewski.com/2013/11/25/functional-data-structures-in-c-trees/).

## Features

- **Red-Black Tree:** Persistent and immutable implementation to ensure data consistency and balance.
- **Tokenization:** Efficiently tokenize text while handling punctuation and case sensitivity.
- **Parallel Processing:** Optional parallel execution for tokenization and tree insertion for better performance.
- **Timing Utilities:** Measure and report the time taken for each step of the process.
- **Functional Principles:** The code avoids mutation and ensures immutability in tree operations.

---

## Getting Started

### Prerequisites

- **C++20** compiler
- **CMake** (minimum version 3.29)
- **Standard Libraries:** `<string>`, `<vector>`, `<future>`, `<numeric>`, `<filesystem>`, `<chrono>`

---

### Installation and Build

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/binhduong160199/Functional-Programming-Final-Project
   cd red-black-tree-project
2. **Run project:**
   ```bash
   ./run.sh
   type "war_and_peace.txt"		 	
