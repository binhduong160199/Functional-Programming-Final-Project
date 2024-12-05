#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

// Simple function to test
int add(int a, int b) {
    return a + b;
}

// Test cases for the `add` function
TEST_CASE("Testing add function") {
    CHECK(add(1, 2) == 3);
    CHECK(add(0, 0) == 0);
    CHECK(add(-1, 1) == 0);
    CHECK(add(5, 5) == 10);
}
