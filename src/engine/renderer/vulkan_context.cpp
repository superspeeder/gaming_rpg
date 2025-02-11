#include "vulkan_context.hpp"

#include "engine/engine_context.hpp"

#include <GLFW/glfw3.h>

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC==1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;
#else
#error "You must compile this program with Vulkan in dynamic dispatch mode (set the preprocessor define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
#endif

namespace engine {
    VulkanContext::VulkanContext(const std::shared_ptr<EngineContext>& engineContext) : m_EngineContext(engineContext) {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(glfwGetInstanceProcAddress);

        {
            vk::ApplicationInfo appInfo{};
            appInfo.apiVersion = vk::ApiVersion14;

            uint32_t count;
            const char** requiredInstanceExtensions = glfwGetRequiredInstanceExtensions(&count);

            std::vector<const char*> instanceExtensions(requiredInstanceExtensions, requiredInstanceExtensions + count);
            std::vector<const char*> instanceLayers;

            if (engineContext->debug_settings())
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

    }
} // engine