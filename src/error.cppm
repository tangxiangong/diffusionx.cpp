module;

#include <expected>
#include <optional>
#include <string>
#include <utility>

export module diffusionx.error;

/**
 * @brief Error structure representing various error conditions in the diffusionx library
 * 
 * This structure encapsulates error information with a descriptive message.
 * It provides static factory methods for creating common error types.
 */
export struct Error {
    std::string message; ///< The error message describing what went wrong

    /**
     * @brief Constructs an Error with the given message
     * @param msg The error message
     */
    explicit Error(std::string msg) : message(std::move(msg)) {
    }

    /**
     * @brief Creates an invalid argument error
     * @param msg Additional context about the invalid argument
     * @return Error instance representing an invalid argument error
     */
    static auto InvalidArgument(const std::string &msg) -> Error {
        return Error("Invalid argument: " + msg);
    }

    /**
     * @brief Creates a not implemented error
     * @param msg Additional context about what is not implemented
     * @return Error instance representing a not implemented error
     */
    static auto NotImplemented(const std::string &msg) -> Error {
        return Error("Not implemented: " + msg);
    }

    /**
     * @brief Creates an I/O error
     * @param msg Additional context about the I/O error
     * @return Error instance representing an I/O error
     */
    static auto IoError(const std::string &msg) -> Error {
        return Error("I/O error: " + msg);
    }

    /**
     * @brief Creates a simulation error
     * @param msg Additional context about the simulation error
     * @return Error instance representing a simulation error
     */
    static auto SimulationFailed(const std::string &msg) -> Error {
        return Error("Simulation failed: " + msg);
    }
};

/**
 * @brief Type alias for std::expected with Error as the error type
 * @tparam T The value type when the operation succeeds
 * 
 * This represents a computation that can either succeed with a value of type T
 * or fail with an Error.
 */
export template<typename T>
using Result = std::expected<T, Error>;

/**
 * @brief Type alias for std::optional
 * @tparam T The value type that may or may not be present
 * 
 * This represents a value that may or may not be present.
 */
export template<typename T>
using Option = std::optional<T>;

/**
 * @brief Creates an unexpected error result
 * @param e The error to wrap
 * @return std::unexpected containing the error
 * 
 * This function is used to create error results in a more ergonomic way.
 */
export inline auto Err(const Error &e) -> std::unexpected<Error> {
    return std::unexpected<Error>(e);
}

/**
 * @brief Creates a successful result
 * @tparam T The type of the value
 * @param value The value to wrap in a successful result
 * @return std::expected containing the value
 * 
 * This function is used to create successful results in a more ergonomic way.
 */
export template<typename T>
auto Ok(T &&value) -> std::expected<std::remove_reference_t<T>, Error> {
    return std::expected<std::remove_reference_t<T>, Error>(std::forward<T>(value));
}

/**
 * @brief Creates a successful result with void value
 * @return std::expected<void, Error> representing success
 * 
 * This function is used to create successful results for operations that don't return a value.
 */
export inline auto Ok() -> std::expected<void, Error> {
    return std::expected<void, Error>();
}

/**
 * @brief Creates an optional with a value
 * @tparam T The type of the value
 * @param value The value to wrap in an optional
 * @return std::optional containing the value
 * 
 * This function is used to create optional values in a more ergonomic way.
 */
export template<typename T>
auto Some(T &&value) -> std::optional<std::remove_reference_t<T>> {
    return std::optional<std::remove_reference_t<T>>(std::forward<T>(value));
}

/**
 * @brief Type alias for std::nullopt_t representing no value
 * 
 * This represents the absence of a value in an optional.
 */
export using None = std::nullopt_t;

/**
 * @brief Unwraps a Result, returning the contained value
 * @tparam T The type of the value in the Result
 * @param result The Result to unwrap
 * @return The contained value
 * @warning This function assumes the Result contains a value and will cause
 *          undefined behavior if called on an error Result. Use with caution.
 */
export template<typename T>
auto unwrap(const Result<T> &result) -> T {
    return *result;
}
