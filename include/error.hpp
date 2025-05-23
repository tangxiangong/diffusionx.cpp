#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include <expected>
#include <optional>
#include <utility>

struct Error {
    std::string message;

    explicit Error(std::string msg) : message(std::move(msg)) {
    }

    static Error InvalidArgument(const std::string &msg) {
        return Error("Invalid argument: " + msg);
    }
};

template<typename T>
using Result = std::expected<T, Error>;

inline std::unexpected<Error> Err(const Error &e) {
    return std::unexpected<Error>(e);
}

template<typename T>
std::expected<T, Error> Ok(T &&value) {
    return std::expected<T, Error>(std::forward<T>(value));
}

template<typename T>
std::optional<T> Some(T &&value) {
    return std::optional<T>(std::forward<T>(value));
}

using None = std::nullopt_t;

template<typename T>
T unwrap(const Result<T> &result) {
    return *result;
}

#endif //ERROR_HPP
