#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <expected>

struct Error {
    std::string message;
    Error(const std::string& msg) : message(msg) {}
    static Error InvalidArgument(const std::string& msg) {
        return Error("Invalid argument: " + msg);
    }
};

template <typename T>
using Result = std::expected<T, Error>;

inline std::unexpected<Error> Err(const Error& e) {
    return std::unexpected<Error>(e);
}

template <typename T>
inline std::expected<T, Error> Ok(T&& value) {
    return std::expected<T, Error>(std::forward<T>(value));
}

#endif //ERROR_H
