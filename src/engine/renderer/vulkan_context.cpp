#include "vulkan_context.hpp"

#include "engine/engine_context.hpp"
#include "engine/tools.hpp"

#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;
#else
#error "You must compile this program with Vulkan in dynamic dispatch mode (set the preprocessor define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
#endif

namespace engine {
    VulkanContext::VulkanContext(const std::shared_ptr<EngineContext> &engine_context) : m_EngineContext(engine_context) {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(glfwGetInstanceProcAddress);
        {
            vk::ApplicationInfo appInfo{};
            appInfo.apiVersion = vk::ApiVersion14;

            uint32_t     count;
            const char **required_instance_extensions = glfwGetRequiredInstanceExtensions(&count);

            std::vector<const char *> instance_extensions(required_instance_extensions, required_instance_extensions + count);
            std::vector<const char *> instance_layers;

            if (engine_context->debug_settings().enable_graphics_api_validation) {
                instance_layers.push_back("VK_LAYER_KHRONOS_validation");
                instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            if (engine_context->debug_settings().enable_graphics_api_call_dump) {
                instance_layers.push_back("VK_LAYER_LUNARG_api_dump");
            }

            m_Instance = vk::raii::Instance(m_Context, vk::InstanceCreateInfo({}, &appInfo, instance_layers, instance_extensions));
            VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_Instance);
        }

        auto physical_devices = vk::raii::PhysicalDevices(m_Instance);
        m_PhysicalDevice      = physical_devices[0]; // TODO: non-naive physical device selection

        {
            std::vector<const char *> device_extensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            };

            vk::PhysicalDeviceFeatures2        f2{};
            vk::PhysicalDeviceVulkan11Features v11f{};
            vk::PhysicalDeviceVulkan12Features v12f{};
            vk::PhysicalDeviceVulkan13Features v13f{};
            vk::PhysicalDeviceVulkan14Features v14f{};

            f2.pNext   = &v11f;
            v11f.pNext = &v12f;
            v12f.pNext = &v13f;
            v13f.pNext = &v14f;

            f2.features.geometryShader     = true;
            f2.features.tessellationShader = true;
            f2.features.multiDrawIndirect  = true;
            f2.features.fillModeNonSolid   = true;
            f2.features.largePoints        = true;
            f2.features.wideLines          = true;

            v12f.drawIndirectCount      = true;
            v12f.runtimeDescriptorArray = true;

            v13f.synchronization2 = true;
            v13f.dynamicRendering = true;

            v14f.pushDescriptor = true;

            bool     selected_graphics_family = false;
            bool     selected_present_family  = false;
            uint32_t index                    = 0;

            auto dummy_window = m_EngineContext->create_dummy_window();
            auto surface      = dummy_window->create_surface_raw(m_Instance);

            auto queueFamilyProperties = m_PhysicalDevice.getQueueFamilyProperties();
            for (const auto &props : queueFamilyProperties) {
                if (!selected_graphics_family && props.queueFlags & vk::QueueFlagBits::eGraphics) {
                    selected_graphics_family = true;
                    m_PrimaryQueueFamily     = index;
                }

                if (!selected_present_family && m_PhysicalDevice.getSurfaceSupportKHR(index, *surface)) {
                    selected_present_family   = true;
                    m_PresentationQueueFamily = index;
                }

                if (!m_ExclusiveTransferQueueFamily.has_value() && !(props.queueFlags & vk::QueueFlagBits::eGraphics) && !(props.queueFlags & vk::QueueFlagBits::eCompute) &&
                    props.queueFlags & vk::QueueFlagBits::eTransfer) {
                    m_ExclusiveTransferQueueFamily = index;
                }

                if (!m_ExclusiveComputeQueueFamily.has_value() && !(props.queueFlags & vk::QueueFlagBits::eGraphics) && props.queueFlags & vk::QueueFlagBits::eCompute) {
                    m_ExclusiveComputeQueueFamily = index;
                }

                ++index;
            }

            if (!selected_graphics_family) {
                throw crash(CrashReason::CriticalFailure, "No graphics queue family available (likely a broken vulkan driver).");
            }

            if (!selected_present_family) {
                throw crash(CrashReason::CriticalFailure, "No queue family supports presentation.");
                /* TODO: mitigate the possibility of this occurring by not using a naive gpu selection method (we can
                 *       do this check for presentation support when selecting physical device instead which should
                 *       prevent us from ever seeing this error).
                 */
            }

            bool primary_doesnt_present = m_PrimaryQueueFamily != m_PresentationQueueFamily;

            std::array<float, 2> queue_priorities = {1.0f, 0.5f};

            uint32_t graphics_queue_count = std::min(queueFamilyProperties[m_PrimaryQueueFamily].queueCount, 2U);
            uint32_t transfer_queue_count, compute_queue_count;

            std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;

            queue_create_infos.emplace_back(vk::DeviceQueueCreateFlags{}, m_PrimaryQueueFamily, graphics_queue_count, queue_priorities.data());

            if (primary_doesnt_present) {
                queue_create_infos.emplace_back(vk::DeviceQueueCreateFlags{}, m_PresentationQueueFamily, 1, queue_priorities.data());
            }

            if (m_ExclusiveTransferQueueFamily.has_value()) {
                transfer_queue_count = std::min(queueFamilyProperties[m_ExclusiveTransferQueueFamily.value()].queueCount, 2U);
                queue_create_infos.emplace_back(vk::DeviceQueueCreateFlags{}, m_ExclusiveTransferQueueFamily.value(), transfer_queue_count, queue_priorities.data());
            }

            if (m_ExclusiveComputeQueueFamily.has_value()) {
                compute_queue_count = std::min(queueFamilyProperties[m_ExclusiveComputeQueueFamily.value()].queueCount, 2U);
                queue_create_infos.emplace_back(vk::DeviceQueueCreateFlags{}, m_ExclusiveComputeQueueFamily.value(), compute_queue_count, queue_priorities.data());
            }

            m_Device = vk::raii::Device(m_PhysicalDevice, vk::DeviceCreateInfo({}, queue_create_infos, {}, device_extensions, nullptr, &f2));
            VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_Device);

            m_Queues.primary.main = m_Device.getQueue(m_PrimaryQueueFamily, 0);
            if (graphics_queue_count == 2) {
                m_Queues.primary.lowPriority = m_Device.getQueue(m_PrimaryQueueFamily, 1);
            }

            m_Queues.present = m_Device.getQueue(m_PresentationQueueFamily, 0);

            if (m_ExclusiveTransferQueueFamily.has_value()) {
                m_Queues.exclusiveTransfer = QueueSet{m_Device.getQueue(m_ExclusiveTransferQueueFamily.value(), 0)};
                if (transfer_queue_count == 2) {
                    m_Queues.exclusiveTransfer->lowPriority = m_Device.getQueue(m_ExclusiveTransferQueueFamily.value(), 1);
                }
            }

            if (m_ExclusiveComputeQueueFamily.has_value()) {
                m_Queues.exclusiveCompute = QueueSet{m_Device.getQueue(m_ExclusiveComputeQueueFamily.value(), 0)};
                if (compute_queue_count == 2) {
                    m_Queues.exclusiveCompute->lowPriority = m_Device.getQueue(m_ExclusiveComputeQueueFamily.value(), 1);
                }
            }
        }
    }
} // namespace engine
