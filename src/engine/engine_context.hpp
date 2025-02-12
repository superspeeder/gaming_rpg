#pragma once

#include "engine/window.hpp"

#include "engine/renderer/vulkan_context.hpp"

#include <memory>

namespace engine {

    /**
     * Configuration for Engine debugging.
     *
     * @note All debugging code should be considered as removed during preprocessing in the distribution form of the engine for performance reasons. The only exception to this is `enableGraphicsApiValidation` which is left on due to lack of any overhead if it's left in.
     */
    struct DebugSettings {
        /**
         * Enable validation layers for Vulkan
         */
        bool enableGraphicsApiValidation = false;

        /**
         * Enable the API dump for Vulkan. NEVER use this unless you have no other option (and are willing to sift through possibly hundreds of thousands of lines of text output).
         */
        bool enableGraphicsApiCallDump = false;

        /**
         * Enable continuity, sanity, and ordering checks on render graphs. This is disabled by default for performance reasons (generally, if you are in development, you probably want this on, otherwise any checking won't actually do anything useful so it should just be off).
         */
        bool enableRenderGraphChecking = false;
    };

    class EngineContext {
      public:
        EngineContext();
        virtual ~EngineContext() = default;

        /**
         * Create a dummy window which is configured the same as any other window, but is invisible to the user and is only used to query system support for features (for example,
         * querying for presentation support on a queue family).
         *
         * @see {@link engine::Window}
         *
         * @return A unique_ptr to the dummy window. A unique_ptr is used instead of the standard weak_ptr used by the window management system because a dummy window is not
         * intended to exist for long, and further will not be included in any tracking (if you maintain the windows lifetime, the system will not know and may deinitialize before
         * you clean it up).
         */
        [[nodiscard]] std::unique_ptr<Window> createDummyWindow() const;

        [[nodiscard]] const DebugSettings& debugSettings() const;

      private:
        DebugSettings m_DebugSettings;


        std::shared_ptr<VulkanContext> m_VulkanContext;
    };

} // namespace engine
