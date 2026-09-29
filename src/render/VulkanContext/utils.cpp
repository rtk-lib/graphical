#include "render/VulkanContext.hpp"

namespace rtk
{
    uint32_t VulkanContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        throw std::runtime_error("Failed to find suitable memory type!");
    }

    VkCommandBuffer VulkanContext::beginSingleTimeCommands() const
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = _commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        if (vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate command buffer");

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
            throw std::runtime_error("Failed to begin single time command buffer");
        }
        return commandBuffer;
    }

    void VulkanContext::endSingleTimeCommands(VkCommandBuffer commandBuffer) const
    {
        try {
            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
                throw std::runtime_error("Failed to end command buffer");

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
                throw std::runtime_error("Failed to submit queue");
            
            if (vkQueueWaitIdle(_graphicsQueue) != VK_SUCCESS)
                throw std::runtime_error("Failed to wait idle");
        } catch (...) {
            vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
            throw;
        }

        vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
    }
}