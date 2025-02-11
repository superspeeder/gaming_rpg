#include "tools.hpp"

#include <spdlog/spdlog.h>

#ifdef WIN32
#ifdef UNICODE
#undef UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace engine {
    void errorPopup(const std::string_view message) {
        spdlog::critical("Application crashed: {}", message);

#ifdef WIN32
        MessageBoxA(nullptr, message.data(), "Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_TASKMODAL);
#else
        // TODO: error popups on other operating systems
#endif
    }

    crash::crash(const CrashReason reason, const std::string_view message)
        : reason(reason), message(message), full_message(std::string(to_string(reason)) + ": " + std::string(message)) {}

    const char *crash::what() const {
        return full_message.c_str();
    }

} // namespace engine
