#include "render/VulkanContext.hpp"

namespace rtk
{
    /**
     * @brief give info to how use the buffer of sprite data
    */
    void VulkanContext::createCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
            throw std::runtime_error("Failed to create command pool!");
    }
}