#include <print>
#include "random/stable.h"

int main() {
    size_t len = 10;
    auto result = rand_stable(len, 0.8, 1.0);
    if (!result) {
        std::println("error: {}", result.error().message);
        return 1;
    }
    auto rnd = result.value();
    std::println("{}", rnd);
}
