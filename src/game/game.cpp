#include "game.hpp"

namespace game {
    Game::Game() = default;

    std::optional<engine::crash> Game::verify_system() const {
        return std::nullopt;
    }

    void Game::render_frame(const vk::raii::CommandBuffer &cmd, const engine::FrameInfo &frame_info) {
        engine::transition_image(
            cmd,
            frame_info.image,
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1),
            engine::ImageState{
                .layout = vk::ImageLayout::eUndefined,
                .access = vk::AccessFlagBits2::eNone,
                .stage  = vk::PipelineStageFlagBits2::eTopOfPipe,
                .owner  = VK_QUEUE_FAMILY_IGNORED,
            },
            engine::ImageState{
                .layout = vk::ImageLayout::eTransferDstOptimal,
                .access = vk::AccessFlagBits2::eTransferWrite,
                .stage  = vk::PipelineStageFlagBits2::eTransfer,
                .owner  = VK_QUEUE_FAMILY_IGNORED,
            }
        );

        cmd.clearColorImage(
            frame_info.image,
            vk::ImageLayout::eTransferDstOptimal,
            vk::ClearColorValue(1.0f, 0.0f, 0.0f, 1.0f),
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
        );

        engine::transition_image(
            cmd,
            frame_info.image,
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1),
            engine::ImageState{
                .layout = vk::ImageLayout::eTransferDstOptimal,
                .access = vk::AccessFlagBits2::eTransferWrite,
                .stage  = vk::PipelineStageFlagBits2::eTransfer,
                .owner  = VK_QUEUE_FAMILY_IGNORED,
            },
            engine::ImageState{
                .layout = vk::ImageLayout::ePresentSrcKHR,
                .access = vk::AccessFlagBits2::eNone,
                .stage  = vk::PipelineStageFlagBits2::eBottomOfPipe,
                .owner  = VK_QUEUE_FAMILY_IGNORED,
            }
        );
    }
} // namespace game
