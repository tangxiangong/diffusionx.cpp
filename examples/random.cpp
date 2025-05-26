#include <print>
#include <chrono>

import diffusionx.random;

int main() {
    size_t len = 3;
    {
        auto normal = randn<float>(len);
        if (!normal) {
            std::println("error: {}", normal.error().message);
            return 1;
        }
        std::println("randn: {}", normal.value());
    }

    {
        auto exp = randexp<float>(len);
        if (!exp) {
            std::println("error: {}", exp.error().message);
            return 1;
        }
        std::println("randexp: {}", exp.value());
    }

    {
        auto poisson = rand_poisson<unsigned long>(len);
        if (!poisson) {
            std::println("error: {}", poisson.error().message);
            return 1;
        }
        std::println("rand_poisson: {}", poisson.value());
    }
    return 0;
}
