#pragma once
#include <memory>

namespace engine {
    class Application;
}

namespace engine::imgui {
    /**
     * ImGUI backend built for this engine. I've done this because of a variety of weird incompatibilities between my engine and the pre-made glfw and vulkan integration. Additionally, this allows me to use engine primitives and logic for ImGui rendering without feeling some level of pain (outside of this implementation at least).
     *
     * For example, we can use our own event system dispatching to handle inputs still, and not have to do weird things to make sure if ImGui will handle the input.
     * Also, I can make sure that the rendering methods I use will be followed correctly, which means that I can easily track the resource usage of the imgui layer (also, I can shuffle imgui onto its own layer easier by controlling how it actually renders. In this case, you just enable the ImGui layer in the application and it'll automatically take the rendered draw data and output it to that layer's output texture, which gets drawn onto the screen same as any other layer.
     */
    class ImGuiBackend {
    public:
        ImGuiBackend();
        ~ImGuiBackend() = default;

        void new_frame();

    private:

    };
}
