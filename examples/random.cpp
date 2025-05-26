#include <print>
#include <chrono>

import diffusionx;

int main() {
    size_t len = 10;
    {
        auto normal = randn<float>(len);
        if (!normal) {
            std::println("error: {}", normal.error().message);
            return 1;
        }
        for (const auto &val: normal.value()) {
            std::println("{}", val);
        }
    }

    {
        auto exp = randexp<float>(len);
        if (!exp) {
            std::println("error: {}", exp.error().message);
            return 1;
        }

        for (const auto &val: exp.value()) {
            std::println("{}", val);
        }
    }

    {
        auto poisson = rand_poisson<unsigned long>(len);
        if (!poisson) {
            std::println("error: {}", poisson.error().message);
            return 1;
        }

        for (const auto &val: poisson.value()) {
            std::println("{}", val);
        }
    }
    return 0;
}
