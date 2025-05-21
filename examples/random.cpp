#include <print>
#include "random/normal.h"
#include "random/exponential.h"
#include <chrono>

#include "random/poisson.h"

int main() {
    {
        auto start_time = std::chrono::high_resolution_clock::now();

        auto normal = randn(3);

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        if (!normal) {
            std::println("error: {}", normal.error().message);
            return 1;
        }
        for (const auto &val: normal.value()) {
            std::println("{}", val);
        }

        // std::println("Time taken by randn: {} ms", duration.count());
    }

    {
        auto start_time = std::chrono::high_resolution_clock::now();

        auto exp = randexp(3);

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        if (!exp) {
            std::println("error: {}", exp.error().message);
            return 1;
        }

        for (const auto &val: exp.value()) {
            std::println("{}", val);
        }

        // std::println("Time taken by randexp: {} ms", duration.count());
    }

    {
        auto start_time = std::chrono::high_resolution_clock::now();

        auto poisson = rand_poisson<unsigned long>(3);

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        if (!poisson) {
            std::println("error: {}", poisson.error().message);
            return 1;
        }

        for (const auto &val: poisson.value()) {
            std::println("{}", val);
        }

        // std::println("Time taken by randexp: {} ms", duration.count());
    }
    return 0;
}
