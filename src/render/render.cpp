#include "render.hpp"
#include "../Logger/Logger.hpp"
#include <stdexcept>
#include <set>
#include <cstring>

namespace rtk
{
    Render::Render(Window& window) : _window(window)
    {
        LOG_INFO("Initializing Vulkan for Deferred + Path Tracing");
        createInstance();
        _window.createSurface(_instance);
        _surface = (VkSurfaceKHR)_window.getSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createRenderPass();
        createGBuffer();
        LOG_INFO("Vulkan initialization complete");
    }

    Render::~Render()
    {
        if (_device) {
            vkDeviceWaitIdle(_device);

            vkDestroyImageView(_device, _gBuffer.positionView, nullptr);
            vkDestroyImage(_device, _gBuffer.positionImage, nullptr);
            vkFreeMemory(_device, _gBuffer.positionMemory, nullptr);

            vkDestroyImageView(_device, _gBuffer.normalView, nullptr);
            vkDestroyImage(_device, _gBuffer.normalImage, nullptr);
            vkFreeMemory(_device, _gBuffer.normalMemory, nullptr);

            vkDestroyImageView(_device, _gBuffer.albedoView, nullptr);
            vkDestroyImage(_device, _gBuffer.albedoImage, nullptr);
            vkFreeMemory(_device, _gBuffer.albedoMemory, nullptr);

            vkDestroyImageView(_device, _gBuffer.depthView, nullptr);
            vkDestroyImage(_device, _gBuffer.depthImage, nullptr);
            vkFreeMemory(_device, _gBuffer.depthMemory, nullptr);

            vkDestroyRenderPass(_device, _renderPass, nullptr);
            vkDestroyDevice(_device, nullptr);
        }
        if (_instance && _surface) {
            vkDestroySurfaceKHR(_instance, _surface, nullptr);
            vkDestroyInstance(_instance, nullptr);
        }
        LOG_INFO("Render destroyed");
    }

    void Render::createInstance()
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "R-Type";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "RTK Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        auto extensions = _window.getRequiredExtensions();

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
            LOG_FATAL("Failed to create Vulkan instance");
            throw std::runtime_error("Vulkan failure");
        }
    }

    void Render::pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
        if (deviceCount == 0) throw std::runtime_error("No Vulkan support");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());
        _physicalDevice = devices[0];
        LOG_INFO("Physical device selected");
    }

    void Render::createLogicalDevice()
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilies.data());

        uint32_t graphicsQueueIndex = 0;
        bool found = false;
        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsQueueIndex = i;
                found = true;
                break;
            }
        }
        if (!found) throw std::runtime_error("No graphics queue found");

        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        uint32_t extCount;
        vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, nullptr);
        std::vector<VkExtensionProperties> availableExts(extCount);
        vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, availableExts.data());

        auto isExtSupported = [&](const char* extName) {
            for (const auto& ext : availableExts) {
                if (strcmp(ext.extensionName, extName) == 0) return true;
            }
            return false;
        };

        bool supportRT = isExtSupported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) &&
                         isExtSupported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) &&
                         isExtSupported(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

        std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        if (supportRT) {
            LOG_INFO("Ray Tracing supported, enabling RT extensions");
            deviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
            deviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
            deviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            deviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        } else {
            LOG_WARN("Ray Tracing NOT supported, falling back to pure Rasterization (Deferred Shading)");
        }

        VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures{};
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures{};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelStructureFeatures{};
        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        
        if (supportRT) {
            bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
            bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;

            rayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
            rayTracingFeatures.rayTracingPipeline = VK_TRUE;
            rayTracingFeatures.pNext = &bufferDeviceAddressFeatures;

            accelStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
            accelStructureFeatures.accelerationStructure = VK_TRUE;
            accelStructureFeatures.pNext = &rayTracingFeatures;

            deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            deviceFeatures2.pNext = &accelStructureFeatures;
        }

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = supportRT ? &deviceFeatures2 : nullptr;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        VkResult res = vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device);
        if (res != VK_SUCCESS) {
            LOG_FATAL("Failed to create logical device. Error code: " + std::to_string(res));
            throw std::runtime_error("Logical device creation failed");
        }
        vkGetDeviceQueue(_device, graphicsQueueIndex, 0, &_graphicsQueue);
        _presentQueue = _graphicsQueue;
    }

    void Render::createSwapChain() {}

    uint32_t Render::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type!");
    }

    void Render::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(_device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(_device, image, imageMemory, 0);
    }

    VkImageView Render::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
        return imageView;
    }

    VkFormat Render::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat Render::findDepthFormat() {
        return findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    void Render::createRenderPass()
    {
        std::vector<VkAttachmentDescription> attachments(4);

        attachments[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        attachments[1].format = VK_FORMAT_R16G16B16A16_SFLOAT;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        attachments[2].format = VK_FORMAT_R8G8B8A8_UNORM;
        attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        attachments[3].format = findDepthFormat();
        attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        std::vector<VkAttachmentReference> colorReferences = {
            {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
            {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
            {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
        };

        VkAttachmentReference depthReference{};
        depthReference.attachment = 3;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
        subpass.pColorAttachments = colorReferences.data();
        subpass.pDepthStencilAttachment = &depthReference;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }

        LOG_INFO("Deferred RenderPass configured with G-Buffer attachments");
    }

    void Render::createGBuffer()
    {
        uint32_t width = 800;
        uint32_t height = 600;

        VkImageUsageFlags colorUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        createImage(width, height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, colorUsage, properties, _gBuffer.positionImage, _gBuffer.positionMemory);
        _gBuffer.positionView = createImageView(_gBuffer.positionImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

        createImage(width, height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, colorUsage, properties, _gBuffer.normalImage, _gBuffer.normalMemory);
        _gBuffer.normalView = createImageView(_gBuffer.normalImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

        createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, colorUsage, properties, _gBuffer.albedoImage, _gBuffer.albedoMemory);
        _gBuffer.albedoView = createImageView(_gBuffer.albedoImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

        VkFormat depthFormat = findDepthFormat();
        createImage(width, height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, properties, _gBuffer.depthImage, _gBuffer.depthMemory);
        _gBuffer.depthView = createImageView(_gBuffer.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

        LOG_INFO("G-Buffer created (Position, Normal, Albedo, Depth)");
    }

    void Render::draw()
    {
        _window.display({0,0,0});
    }
}
