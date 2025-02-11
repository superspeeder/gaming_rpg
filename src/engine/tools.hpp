#pragma once

#include <stdexcept>
#include <string_view>

namespace engine {
    void error_popup(std::string_view message);

    enum class CrashReason {
        UnsupportedSystem,
        OutOfMemory,
        OutOfVideoMemory,
        LoadFailed,
        CriticalFailure,
        UnknownError,
    };

    constexpr std::string_view to_string(const CrashReason reason) {
        switch (reason) {
        case CrashReason::UnsupportedSystem:
            return "UnsupportedSystem";
        case CrashReason::OutOfMemory:
            return "OutOfMemory";
        case CrashReason::OutOfVideoMemory:
            return "OutOfVideoMemory";
        case CrashReason::LoadFailed:
            return "LoadFailed";
        case CrashReason::CriticalFailure:
            return "CriticalFailure";

        case CrashReason::UnknownError:
        default:
            return "Error";
        }
    }

    class crash final : public std::exception {
      public:
        crash(CrashReason reason, std::string_view message);
        [[nodiscard]] const char *what() const override;

        const CrashReason reason;
        const std::string message;
        const std::string full_message;
    };
} // namespace engine
