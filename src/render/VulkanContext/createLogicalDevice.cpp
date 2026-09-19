#include "render/VulkanContext.hpp"

namespace rtk
{
    void VulkanContext::createLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        /*Disabeld Anisotropy for less complexity*/
        deviceFeatures.samplerAnisotropy = VK_FALSE;

        VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

        indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
        indexingFeatures.runtimeDescriptorArray = VK_TRUE;
        /*Enables non-uniform indexing in shaders. Prevents texture bleeding/glitches when adjacent pixels in a subgroup sample from different textures.*/
        indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &indexingFeatures;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(_deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = _deviceExtensions.data();

        if (_enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
            createInfo.ppEnabledLayerNames = _validationLayers.data();
        } else
            createInfo.enabledLayerCount = 0;

        if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS)
            throw std::runtime_error("Failed to create logical device!");

        vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
        vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);
    }
}