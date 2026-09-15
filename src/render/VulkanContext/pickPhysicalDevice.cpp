#include "render/VulkanContext.hpp"

namespace rtk
{

    bool VulkanContext::isDeviceSuitable(VkPhysicalDevice device, bool allowNonDiscrete)
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device, &properties);

        const bool isDiscrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

        if (!isDiscrete && !allowNonDiscrete) {
            _virtualGpuPool.push_back(device);
            return false;
        }

        const QueueFamilyIndices indices = findQueueFamilies(device);

        const bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;

        if (extensionsSupported) {
            const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);

            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

        VkPhysicalDeviceFeatures2 features{};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features.pNext = &indexingFeatures;

        vkGetPhysicalDeviceFeatures2(device, &features);

        const bool bindlessSupported = indexingFeatures.descriptorBindingPartiallyBound == VK_TRUE &&
            indexingFeatures.runtimeDescriptorArray == VK_TRUE &&
            indexingFeatures.shaderSampledImageArrayNonUniformIndexing == VK_TRUE;

        constexpr uint32_t RequiredTextures = 1000;

        const bool descriptorLimitsSupported =
            properties.limits.maxPerStageDescriptorSamplers >= RequiredTextures &&
            properties.limits.maxPerStageDescriptorSampledImages >= RequiredTextures &&
            properties.limits.maxDescriptorSetSamplers >= RequiredTextures &&
            properties.limits.maxDescriptorSetSampledImages >= RequiredTextures &&
            properties.limits.maxPerStageResources >= RequiredTextures;

        const bool vulkan12Supported = properties.apiVersion >= VK_API_VERSION_1_2;

#ifdef __APPLE__
        return indices.isComplete() && extensionsSupported && swapChainAdequate && bindlessSupported && vulkan12Supported;
#else
        return indices.isComplete() && extensionsSupported && swapChainAdequate && bindlessSupported && descriptorLimitsSupported && vulkan12Supported;
#endif
    }

    bool VulkanContext::checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;

        /*Get the number of extension on this device*/
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        /*Get all the extension possible for this device*/
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        /*Verify that every extension in _deviceExtensions is present by removing matches from the set*/
        std::set<std::string> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());
        for (const auto& extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);
        return requiredExtensions.empty();
    }


    void VulkanContext::pickPhysicalDevice()
    {
        /*Get the number of physical device on the computer*/
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");

        /*Get every physical device on the computer*/
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

        /* Select the first device that meets all application requirements (queue support, swapchain, features) */
        for (const auto& device : devices)
            if (isDeviceSuitable(device, false)){
                _physicalDevice = device;
                break;
            }

        if (_physicalDevice == VK_NULL_HANDLE){
            for (const auto& device : devices)
                if (isDeviceSuitable(device, true)){
                    _physicalDevice = device;
                    break;
                }
        }
        if (_physicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("Failed to find a suitable GPU!");
    }
}