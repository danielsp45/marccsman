#ifndef KV_STATUS_H
#define KV_STATUS_H

#include <string>

class Result {
public:
    // Returns a success status.
    static Result OK() {
        return Result();
    }

    // Returns a status representing a "not found" error.
    static Result NotFound(const std::string& msg = "") {
        return Result("NotFound", msg);
    }

    // Returns a status representing a generic error.
    static Result Error(const std::string& msg = "") {
        return Result("Error", msg);
    }

    // Returns true if the status represents success.
    bool ok() const {
        return code_.empty();
    }

    // Getters for code and message.
    std::string code() const { return code_; }
    std::string message() const { return message_; }

private:
    std::string code_;
    std::string message_;

    // Private constructor for a successful status.
    Result() = default;

    // Private constructor for an error status.
    Result(const std::string& code, const std::string& message)
        : code_(code), message_(message) {}
};

#endif // KV_STATUS_H
