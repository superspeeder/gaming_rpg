#include "vulkan_context.hpp"

#include "engine/engine_context.hpp"
#include "engine/tools.hpp"

#include <GLFW/glfw3.h>

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;
#else
#error "You must compile this program with Vulkan in dynamic dispatch mode (set the preprocessor define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
#endif

namespace engine {
    VulkanContext::VulkanContext(const std::shared_ptr<EngineContext> &engineContext) : m_EngineContext(engineContext) {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(glfwGetInstanceProcAddress);

        {
            vk::ApplicationInfo appInfo{};
            appInfo.apiVersion = vk::ApiVersion14;

            uint32_t     count;
            const char **requiredInstanceExtensions = glfwGetRequiredInstanceExtensions(&count);

            std::vector<const char *> instanceExtensions(requiredInstanceExtensions, requiredInstanceExtensions + count);
            std::vector<const char *> instanceLayers;

            if (engineContext->debugSettings().enableGraphicsApiValidation) {
                instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            if (engineContext->debugSettings().enableGraphicsApiCallDump) {
                instanceLayers.push_back("VK_LAYER_LUNARG_api_dump");
            }

            m_Instance = std::make_unique<vk::raii::Instance>(m_Context, vk::InstanceCreateInfo({}, &appInfo, instanceLayers, instanceExtensions));
        }

        auto physicalDevices = vk::raii::PhysicalDevices(*m_Instance);
        m_PhysicalDevice     = physicalDevices[0]; // TODO: non-naive physical device selection

        {
            std::vector<const char *> deviceExtensions = {
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

            bool     selectedGraphicsFamily = false;
            bool     selectedPresentFamily  = false;
            uint32_t index                  = 0;

            auto        dummyWindow = m_EngineContext->createDummyWindow();
            const auto &surface     = dummyWindow->createSurface(m_Instance);

            auto queueFamilyProperties = m_PhysicalDevice.getQueueFamilyProperties();
            for (const auto &props : queueFamilyProperties) {
                if (!selectedGraphicsFamily && props.queueFlags & vk::QueueFlagBits::eGraphics) {
                    selectedGraphicsFamily = true;
                    m_PrimaryQueueFamily   = index;
                }

                if (!selectedPresentFamily && m_PhysicalDevice.getSurfaceSupportKHR(index, **surface)) {
                    selectedPresentFamily     = true;
                    m_PresentationQueueFamily = index;
                }

                if (!m_ExclusiveTransferQueueFamily.has_value() && !(props.queueFlags & (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute)) &&
                    props.queueFlags & vk::QueueFlagBits::eTransfer) {
                    m_ExclusiveTransferQueueFamily = true;
                }

                if (!m_ExclusiveComputeQueueFamily.has_value() && !(props.queueFlags & (vk::QueueFlagBits::eGraphics)) && props.queueFlags & vk::QueueFlagBits::eCompute) {
                    m_ExclusiveComputeQueueFamily = true;
                }

                ++index;
            }

            if (!selectedGraphicsFamily) {
                throw crash(CrashReason::CriticalFailure, "No graphics queue family available (likely a broken vulkan driver).");
            }

            if (!selectedPresentFamily) {
                throw crash(CrashReason::CriticalFailure, "No queue family supports presentation.");
                /* TODO: mitigate the possibility of this occurring by not using a naive gpu selection method (we can
                 *       do this check for presentation support when selecting physical device instead which should
                 *       prevent us from ever seeing this error).
                 */
            }

            bool primaryDoesntPresent = m_PrimaryQueueFamily != m_PresentationQueueFamily;

            std::array<float, 2> queuePriorities = {1.0f, 0.5f};

            uint32_t graphicsQueueCount = std::min(queueFamilyProperties[m_PrimaryQueueFamily].queueCount, 2U);
            uint32_t transferQueueCount, computeQueueCount;

            std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

            queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags{}, m_PrimaryQueueFamily, graphicsQueueCount, queuePriorities.data());

            if (primaryDoesntPresent) {
                queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags{}, m_PresentationQueueFamily, 1, queuePriorities.data());
            }

            if (m_ExclusiveTransferQueueFamily.has_value()) {
                transferQueueCount = std::min(queueFamilyProperties[m_ExclusiveTransferQueueFamily.value()].queueCount, 2U);
                queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags{}, m_ExclusiveTransferQueueFamily.value(), transferQueueCount, queuePriorities.data());
            }

            if (m_ExclusiveComputeQueueFamily.has_value()) {
                computeQueueCount = std::min(queueFamilyProperties[m_ExclusiveComputeQueueFamily.value()].queueCount, 2U);
                queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags{}, m_ExclusiveComputeQueueFamily.value(), computeQueueCount, queuePriorities.data());
            }

            m_Device = std::make_unique<vk::raii::Device>(m_PhysicalDevice, vk::DeviceCreateInfo({}, queueCreateInfos, {}, deviceExtensions, nullptr, &f2));

            m_Queues.primary.main = m_Device->getQueue(m_PrimaryQueueFamily, 0);
            if (graphicsQueueCount == 2) {
                m_Queues.primary.lowPriority = m_Device->getQueue(m_PrimaryQueueFamily, 1);
            }

            m_Queues.present = m_Device->getQueue(m_PresentationQueueFamily, 0);

            if (m_ExclusiveTransferQueueFamily.has_value()) {
                m_Queues.exclusiveTransfer->main = m_Device->getQueue(m_ExclusiveTransferQueueFamily.value(), 0);
                if (transferQueueCount == 2) {
                    m_Queues.exclusiveTransfer->lowPriority = m_Device->getQueue(m_ExclusiveTransferQueueFamily.value(), 1);
                }
            }

            if (m_ExclusiveComputeQueueFamily.has_value()) {
                m_Queues.exclusiveCompute->main = m_Device->getQueue(m_ExclusiveComputeQueueFamily.value(), 0);
                if (transferQueueCount == 2) {
                    m_Queues.exclusiveCompute->lowPriority = m_Device->getQueue(m_ExclusiveComputeQueueFamily.value(), 1);
                }
            }
        }
    }
} // namespace engine
