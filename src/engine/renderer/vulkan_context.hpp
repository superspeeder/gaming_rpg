#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "engine/fwd.hpp"

namespace engine {

    struct QueueSet {
        vk::raii::Queue main = nullptr;
        std::optional<vk::raii::Queue> lowPriority;
    };

    struct Queues {
        QueueSet primary;
        vk::raii::Queue present = nullptr;

        std::optional<QueueSet> exclusiveTransfer;
        std::optional<QueueSet> exclusiveCompute;
    };

    /**
     * The Transfer and Compute queues will prefer exclusive but use primary if those aren't available (exclusive transfer queues get special hardware, and using seperated hardware for compute is nice sometimes).
     * Low priority versions of queues are suggestions (if the system targeted queue family has at least 2 queues available, it'll use one with a lower priority value for low priority things. Be warned, only use this for things that you actually don't need to be running at good speed, as the gpu may push the work off until later consistently. I would only use this for async, expensive work that isn't required for any frame, or executed at a regular period).
     *      A good low priority example might be a chunk of compute which is used to procedurally generate a world.
     */
    enum class DeviceQueue {
        Primary,
        PrimaryLowPriority,
        Present,
        Transfer,
        TransferLowPriority,
        Compute,
        ComputeLowPriority,
    };

    class VulkanContext {
      public:
        explicit VulkanContext(const std::shared_ptr<EngineContext> &engineContext);
        virtual ~VulkanContext() = default;

      private:
        std::shared_ptr<EngineContext> m_EngineContext;

        vk::raii::Context                   m_Context;
        std::unique_ptr<vk::raii::Instance> m_Instance;

        // Unlike things such as the VkInstance raii handle, there is no concept of ownership over a physical device, so we use an optional to prevent resource acquisition prior to
        // the time when we actually get to enumerate and select physical devices.
        vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;

        std::unique_ptr<vk::raii::Device> m_Device;

        uint32_t m_PrimaryQueueFamily;
        uint32_t m_PresentationQueueFamily;

        std::optional<uint32_t> m_ExclusiveTransferQueueFamily;
        std::optional<uint32_t> m_ExclusiveComputeQueueFamily;

        Queues m_Queues;


    };

} // namespace engine
