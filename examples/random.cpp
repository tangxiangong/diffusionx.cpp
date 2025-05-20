#include <print>
#include "random/normal.h"
#include <chrono>

using normal::rands;

int main() {
    
    auto start_time = std::chrono::high_resolution_clock::now();

    auto result = rands(1000000000);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    if (!result) {
        std::println("error: {}", result.error().message);
        return 1;
    }

    std::println("Time taken by rands: {} ms", duration.count());

    return 0;
}
