#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "engine/fwd.hpp"

namespace engine {

class VulkanContext {
public:
    explicit VulkanContext(const std::shared_ptr<EngineContext>& engineContext);
    virtual ~VulkanContext() = default;
    
private:
    std::shared_ptr<EngineContext> m_EngineContext;

    vk::raii::Context m_Context;
    std::unique_ptr<vk::raii::Instance> m_Instance;

    // Unlike things such as the VkInstance raii handle, there is no concept of ownership over a physical device, so we use an optional to prevent resource acquisition prior to the time when we actually get to enumerate and select physical devices.
    std::optional<vk::raii::PhysicalDevice> m_PhysicalDevice;


};

} // engine
