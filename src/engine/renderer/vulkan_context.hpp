#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "engine/fwd.hpp"

namespace engine {
    struct QueueSet {
        vk::raii::Queue                main = nullptr;
        std::optional<vk::raii::Queue> lowPriority;

        QueueSet(const QueueSet &other)                = delete;
        QueueSet(QueueSet &&other) noexcept            = default;
        QueueSet &operator=(const QueueSet &other)     = delete;
        QueueSet &operator=(QueueSet &&other) noexcept = default;

        QueueSet() = default;

        inline explicit QueueSet(vk::raii::Queue queue) : main(std::move(queue)) {}
    };

    struct Queues {
        QueueSet        primary;
        vk::raii::Queue present = nullptr;

        std::optional<QueueSet> exclusiveTransfer;
        std::optional<QueueSet> exclusiveCompute;

        Queues(const Queues &other)                = delete;
        Queues(Queues &&other) noexcept            = default;
        Queues &operator=(const Queues &other)     = delete;
        Queues &operator=(Queues &&other) noexcept = default;

        Queues() = default;
    };

    /**
     * The Transfer and Compute queues will prefer exclusive but use primary if those aren't available (exclusive transfer queues get special hardware, and using seperated hardware
     * for compute is nice sometimes). Low priority versions of queues are suggestions (if the system targeted queue family has at least 2 queues available, it'll use one with a
     * lower priority value for low priority things. Be warned, only use this for things that you actually don't need to be running at good speed, as the gpu may push the work off
     * until later consistently. I would only use this for async, expensive work that isn't required for any frame, or executed at a regular period). A good low priority example
     * might be a chunk of compute which is used to procedurally generate a world.
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

    class VulkanContext : public std::enable_shared_from_this<VulkanContext> {
        explicit VulkanContext(const std::shared_ptr<EngineContext> &engine_context);

      public:
        static inline std::shared_ptr<VulkanContext> create(const std::shared_ptr<EngineContext> &engineContext) {
            return std::shared_ptr<VulkanContext>(new VulkanContext(engineContext));
        }

        virtual ~VulkanContext() = default;

        VulkanContext(const VulkanContext &other)                = delete;
        VulkanContext(VulkanContext &&other) noexcept            = default;
        VulkanContext &operator=(const VulkanContext &other)     = delete;
        VulkanContext &operator=(VulkanContext &&other) noexcept = default;

        inline const vk::raii::Instance &instance() const { return m_Instance; }

        inline const vk::raii::PhysicalDevice &physical_device() const { return m_PhysicalDevice; }

        inline const vk::raii::Device &device() const { return m_Device; }

        inline const Queues &queues() const { return m_Queues; }

        inline bool are_present_render_shared() const { return m_PresentationQueueFamily == m_PrimaryQueueFamily; }

        inline uint32_t present_queue_family() const { return m_PresentationQueueFamily; }

        inline uint32_t primary_queue_family() const { return m_PrimaryQueueFamily; }

      private:
        std::weak_ptr<EngineContext> m_EngineContext;

        vk::raii::Context        m_Context;
        vk::raii::Instance       m_Instance       = nullptr;
        vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;
        vk::raii::Device         m_Device         = nullptr;

        uint32_t m_PrimaryQueueFamily;
        uint32_t m_PresentationQueueFamily;

        std::optional<uint32_t> m_ExclusiveTransferQueueFamily;
        std::optional<uint32_t> m_ExclusiveComputeQueueFamily;

        Queues m_Queues;
    };

    struct ImageState {
        vk::ImageLayout  layout;
        vk::AccessFlags2 access;
        vk::PipelineStageFlags2 stage;
        uint32_t owner = 0;
    };

    void transition_image(const vk::raii::CommandBuffer &cmd, vk::Image image, const vk::ImageSubresourceRange &isr, const ImageState &src, const ImageState &dst);
} // namespace engine
