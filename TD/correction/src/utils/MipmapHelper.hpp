#pragma once

#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/Device.hpp>
#include <cmath>
#include <algorithm>

inline uint32_t mipLevelCount(uint32_t width, uint32_t height) {
    return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
}

inline void generateMipmaps(vk::Image image, uint32_t width, uint32_t height,
                             uint32_t mipLevels, LavaCake::Device& device) {
    LavaCake::CommandBuffer cmd(device.getDevice(), device.getCommandPool(), false);
    cmd.begin();

    for (uint32_t i = 1; i < mipLevels; i++) {
        // Transition level i-1: eTransferDstOptimal -> eTransferSrcOptimal
        vk::ImageMemoryBarrier barrierSrc{};
        barrierSrc.image                           = image;
        barrierSrc.srcAccessMask                   = vk::AccessFlagBits::eTransferWrite;
        barrierSrc.dstAccessMask                   = vk::AccessFlagBits::eTransferRead;
        barrierSrc.oldLayout                       = vk::ImageLayout::eTransferDstOptimal;
        barrierSrc.newLayout                       = vk::ImageLayout::eTransferSrcOptimal;
        barrierSrc.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrierSrc.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrierSrc.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        barrierSrc.subresourceRange.baseMipLevel   = i - 1;
        barrierSrc.subresourceRange.levelCount     = 1;
        barrierSrc.subresourceRange.baseArrayLayer = 0;
        barrierSrc.subresourceRange.layerCount     = 1;

        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            {},
            {},
            {},
            { barrierSrc }
        );

        // Blit from level i-1 to level i
        int32_t srcWidth  = static_cast<int32_t>(std::max(width  >> (i - 1), 1u));
        int32_t srcHeight = static_cast<int32_t>(std::max(height >> (i - 1), 1u));
        int32_t dstWidth  = std::max(srcWidth  / 2, 1);
        int32_t dstHeight = std::max(srcHeight / 2, 1);

        vk::ImageBlit blit{};
        blit.srcOffsets[0]                 = vk::Offset3D{0, 0, 0};
        blit.srcOffsets[1]                 = vk::Offset3D{srcWidth, srcHeight, 1};
        blit.srcSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
        blit.srcSubresource.mipLevel       = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount     = 1;
        blit.dstOffsets[0]                 = vk::Offset3D{0, 0, 0};
        blit.dstOffsets[1]                 = vk::Offset3D{dstWidth, dstHeight, 1};
        blit.dstSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
        blit.dstSubresource.mipLevel       = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount     = 1;

        vk::CommandBuffer vkCmd = cmd;
        vkCmd.blitImage(
            image, vk::ImageLayout::eTransferSrcOptimal,
            image, vk::ImageLayout::eTransferDstOptimal,
            { blit },
            vk::Filter::eLinear
        );

        // Transition level i-1: eTransferSrcOptimal -> eShaderReadOnlyOptimal
        vk::ImageMemoryBarrier barrierShader{};
        barrierShader.image                           = image;
        barrierShader.srcAccessMask                   = vk::AccessFlagBits::eTransferRead;
        barrierShader.dstAccessMask                   = vk::AccessFlagBits::eShaderRead;
        barrierShader.oldLayout                       = vk::ImageLayout::eTransferSrcOptimal;
        barrierShader.newLayout                       = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrierShader.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrierShader.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrierShader.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        barrierShader.subresourceRange.baseMipLevel   = i - 1;
        barrierShader.subresourceRange.levelCount     = 1;
        barrierShader.subresourceRange.baseArrayLayer = 0;
        barrierShader.subresourceRange.layerCount     = 1;

        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader,
            {},
            {},
            {},
            { barrierShader }
        );
    }

    // Transition last mip level: eTransferDstOptimal -> eShaderReadOnlyOptimal
    vk::ImageMemoryBarrier barrierLast{};
    barrierLast.image                           = image;
    barrierLast.srcAccessMask                   = vk::AccessFlagBits::eTransferWrite;
    barrierLast.dstAccessMask                   = vk::AccessFlagBits::eShaderRead;
    barrierLast.oldLayout                       = vk::ImageLayout::eTransferDstOptimal;
    barrierLast.newLayout                       = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrierLast.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrierLast.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrierLast.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    barrierLast.subresourceRange.baseMipLevel   = mipLevels - 1;
    barrierLast.subresourceRange.levelCount     = 1;
    barrierLast.subresourceRange.baseArrayLayer = 0;
    barrierLast.subresourceRange.layerCount     = 1;

    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eFragmentShader,
        {},
        {},
        {},
        { barrierLast }
    );

    cmd.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = cmd;
    device.getAnyQueue().submit(submitInfo, {});
    device.getAnyQueue().waitIdle();
}
