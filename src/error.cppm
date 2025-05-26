module;

#include <expected>
#include <optional>
#include <string>
#include <utility>

export module diffusionx.error;

export struct Error {
    std::string message;

    explicit Error(std::string msg) : message(std::move(msg)) {}

    static auto InvalidArgument(const std::string &msg) -> Error {
        return Error("Invalid argument: " + msg);
    }
};

export template <typename T>
using Result = std::expected<T, Error>;

export template <typename T>
using Option = std::optional<T>;

export inline auto Err(const Error &e) -> std::unexpected<Error> {
    return std::unexpected<Error>(e);
}

export template <typename T>
auto Ok(T &&value) -> std::expected<T, Error> {
    return std::expected<T, Error>(std::forward<T>(value));
}

export template <typename T>
auto Some(T &&value) -> std::optional<T> {
    return std::optional<T>(std::forward<T>(value));
}

export using None = std::nullopt_t;

export template <typename T>
auto unwrap(const Result<T> &result) -> T {
    return *result;
}
