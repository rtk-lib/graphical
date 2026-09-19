#include "render/VulkanContext.hpp"

namespace rtk
{

    std::vector<const char*> VulkanContext::getRequiredExtensions() {
        return _window.getRequiredExtensions();
    }

    bool VulkanContext::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : _validationLayers) {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound)
                return false;
        }
        return true;
    }

    void VulkanContext::createInstance()
    {
        /*Check Validation like VK_LAYER_KHRONOS_validation aka KHRONOS*/
        if (_enableValidationLayers && !checkValidationLayerSupport()) {
            LOG_WARN("Validation layers requested, but not available! Disabling them.");
            _enableValidationLayers = false;
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "RTK Vulkan Context";

        /*Depracted*/
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

        appInfo.pEngineName = "RTK Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        /*Get VKH SWAPCHAIN extension*/
        auto extensions = getRequiredExtensions();

        #ifdef __APPLE__
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (_enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
            createInfo.ppEnabledLayerNames = _validationLayers.data();
        } else
            createInfo.enabledLayerCount = 0;

        if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create Vulkan instance!");
    }
}