#!/bin/bash

# Ensure the script is run from its own directory
cd "$(dirname "$0")"

# Define paths
BUILD_DIR="build"
EXECUTABLE="./final" # The name of your executable file
INPUT_FILE="war_and_peace.txt" # Input file name
OUTPUT_FILE="output.txt"

# Step 1: Create a build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
  mkdir "$BUILD_DIR"
fi

# Step 2: Navigate to the build directory
cd "$BUILD_DIR"

# Step 3: Run cmake to configure the project
cmake .. -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" || {
    echo "CMake configuration failed";
    exit 1;
}

# Step 4: Build the project
cmake --build . || {
    echo "Build failed";
    exit 1;
}

# Step 5: Run the executable from the project root directory
cd ..
if [ -f "$BUILD_DIR/final" ]; then
  echo "Running the executable..."
  "$BUILD_DIR/final" "$INPUT_FILE" "$OUTPUT_FILE" || {
      echo "Execution failed";
      exit 1;
  }
else
  echo "Executable not found: $BUILD_DIR/final"
  exit 1
fi

echo "Test run completed successfully."