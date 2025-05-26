module;

#include <string>
#include <expected>
#include <optional>
#include <utility>

export module diffusionx.error;

export struct Error {
    std::string message;

    explicit Error(std::string msg) : message(std::move(msg)) {
    }

    static Error InvalidArgument(const std::string &msg) {
        return Error("Invalid argument: " + msg);
    }
};

export template<typename T>
using Result = std::expected<T, Error>;

export inline std::unexpected<Error> Err(const Error &e) {
    return std::unexpected<Error>(e);
}

export template<typename T>
std::expected<T, Error> Ok(T &&value) {
    return std::expected<T, Error>(std::forward<T>(value));
}

export template<typename T>
std::optional<T> Some(T &&value) {
    return std::optional<T>(std::forward<T>(value));
}

export using None = std::nullopt_t;

export template<typename T>
T unwrap(const Result<T> &result) {
    return *result;
}
