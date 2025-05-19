#include <print>
#include "random/normal.h"

using normal::rands;

int main() {
    auto result = rands(10);
    if (!result) {
        std::println("error: {}", result.error().message);
        return 1;
    }
    for (auto&& item : *result) {
        std::println("{}", item);
    }
    return 0;
} 