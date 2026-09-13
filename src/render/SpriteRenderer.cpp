#include "SpriteRenderer.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>

namespace rtk {



    const std::vector<rtk::vec2> QUAD_VERTICES = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    const std::vector<uint16_t> QUAD_INDICES = {0, 1, 2, 2, 3, 0};

    SpriteRenderer::SpriteRenderer(VulkanContext& context, const TextureManager& textureManager)
        : _context(context), _textureManager(textureManager)
    {
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createBuffers();
        createCommandBuffers();
        createSyncObjects();
    }

    SpriteRenderer::~SpriteRenderer()
    {
        VkDevice device = _context.getDevice();

        vkDeviceWaitIdle(device);

        vkDestroyBuffer(device, _quadIndexBuffer, nullptr);
        vkFreeMemory(device, _quadIndexBufferMemory, nullptr);

        vkDestroyBuffer(device, _quadVertexBuffer, nullptr);
        vkFreeMemory(device, _quadVertexBufferMemory, nullptr);

        for (FrameResources& frame : _frames) {
            if (frame.mappedInstances != nullptr && frame.instanceMemory != VK_NULL_HANDLE) {
                vkUnmapMemory(device, frame.instanceMemory);
                frame.mappedInstances = nullptr;
            }

            if (frame.instanceBuffer != VK_NULL_HANDLE)
                vkDestroyBuffer(device, frame.instanceBuffer, nullptr);

            if (frame.instanceMemory != VK_NULL_HANDLE)
                vkFreeMemory(device, frame.instanceMemory, nullptr);

            if (frame.imageAvailable != VK_NULL_HANDLE)
                vkDestroySemaphore(device, frame.imageAvailable, nullptr);

            if (frame.inFlightFence != VK_NULL_HANDLE)
                vkDestroyFence(device, frame.inFlightFence, nullptr);
        }

        destroyRenderFinishedSemaphores();

        vkDestroyPipeline(device, _graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
        for (VkFramebuffer framebuffer : _swapChainFramebuffers)
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        vkDestroyRenderPass(device, _renderPass, nullptr);
    }

    void SpriteRenderer::createRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = _context.getSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(_context.getDevice(), &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS)
            throw std::runtime_error("Failed to create render pass");
    }

    void SpriteRenderer::createGraphicsPipeline()
    {
        VkDevice device = _context.getDevice();

        auto vertShaderCode = readFile("shaders/sprite.vert.spv");
        auto fragShaderCode = readFile("shaders/sprite.frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        auto vertexBinding = SpriteVertex::getBindingDescription();
        auto instanceBinding = getSpriteBindingDescription();
        std::vector<VkVertexInputBindingDescription> bindings = {vertexBinding, instanceBinding};

        auto vertexAttributes = SpriteVertex::getAttributeDescriptions();
        auto instanceAttributes = getSpriteAttributeDescriptions();

        std::vector<VkVertexInputAttributeDescription> attributes;
        attributes.insert(attributes.end(), vertexAttributes.begin(), vertexAttributes.end());
        attributes.insert(attributes.end(), instanceAttributes.begin(), instanceAttributes.end());

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
        vertexInputInfo.pVertexBindingDescriptions = bindings.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPushConstantRange pushConstant{};
        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstant.offset = 0;
        pushConstant.size = sizeof(SpritePushConstants);

        VkDescriptorSetLayout setLayout = _textureManager.getDescriptorSetLayout();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &setLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout");

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = _pipelineLayout;
        pipelineInfo.renderPass = _renderPass;
        pipelineInfo.subpass = 0;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS)
            throw std::runtime_error("Failed to create graphics pipeline");

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    void SpriteRenderer::createFramebuffers()
    {
        const auto& swapChainImageViews = _context.getSwapChainImageViews();
        auto swapChainExtent = _context.getSwapChainExtent();
        _swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            VkImageView attachments[] = { swapChainImageViews[i] };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = _renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(_context.getDevice(), &framebufferInfo, nullptr, &_swapChainFramebuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create framebuffer");
        }
    }

    void SpriteRenderer::createBuffers()
    {
        VkDeviceSize vertexBufferSize = sizeof(rtk::vec2) * QUAD_VERTICES.size();
        createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     _quadVertexBuffer, _quadVertexBufferMemory);

        void* vertexData;
        checkVkR(vkMapMemory(_context.getDevice(), _quadVertexBufferMemory, 0, vertexBufferSize, 0, &vertexData));
        memcpy(vertexData, QUAD_VERTICES.data(), vertexBufferSize);
        vkUnmapMemory(_context.getDevice(), _quadVertexBufferMemory);

        VkDeviceSize indexBufferSize = sizeof(uint16_t) * QUAD_INDICES.size();
        createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     _quadIndexBuffer, _quadIndexBufferMemory);

        void* indexData;
        checkVkR(vkMapMemory(_context.getDevice(), _quadIndexBufferMemory, 0, indexBufferSize, 0, &indexData));
        memcpy(indexData, QUAD_INDICES.data(), indexBufferSize);
        vkUnmapMemory(_context.getDevice(), _quadIndexBufferMemory);

        for (FrameResources& frame : _frames)
            createInstanceBuffer(frame, SpritesPerPage);
    }

    void SpriteRenderer::createSyncObjects()
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkDevice device = _context.getDevice();

        for (FrameResources& frame : _frames) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frame.imageAvailable) != VK_SUCCESS)
                throw std::runtime_error("Failed to create image-available semaphore");

            if (vkCreateFence(device, &fenceInfo, nullptr, &frame.inFlightFence) != VK_SUCCESS)
                throw std::runtime_error("Failed to create in-flight fence");
        }

        createRenderFinishedSemaphores();
    }

    bool SpriteRenderer::beginFrame(const RGB& clearColorVal)
    {
        if (_isFrameStarted)
            throw std::runtime_error("Frame already started");

        FrameResources& frame = _frames[_currentFrame];
        frame.instanceCount = 0;
        frame.uploadedBytes = 0;
        VkDevice device = _context.getDevice();
        VkCommandBuffer commandBuffer = frame.commandBuffer;

        if (vkWaitForFences(device, 1, &frame.inFlightFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
            throw std::runtime_error("Failed to wait for frame fence");

        VkResult result = vkAcquireNextImageKHR(device, _context.getSwapChain(), UINT64_MAX, frame.imageAvailable, VK_NULL_HANDLE, &_imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return false;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swapchain image");
        }

        if (vkResetFences(device, 1, &frame.inFlightFence) != VK_SUCCESS)
            throw std::runtime_error("Failed to reset frame fence");

        if (vkResetCommandBuffer(frame.commandBuffer, 0) != VK_SUCCESS)
            throw std::runtime_error("Failed to reset command buffer");

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Failed to begin recording command buffer");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _renderPass;
        renderPassInfo.framebuffer = _swapChainFramebuffers[_imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = _context.getSwapChainExtent();

        VkClearValue clearColor = {{{clearColorVal.r / 255.0f, clearColorVal.g / 255.0f, clearColorVal.b / 255.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        _isFrameStarted = true;
        return true;
    }

    void SpriteRenderer::drawSprite(const rtk::vec2& position, const rtk::vec2& size, float rotation, uint32_t textureId)
    {
        const SpriteData sprite{position, size, rotation, textureId, 0xFFFFFFFF, 0};

        submit(sprite);
    }

    void SpriteRenderer::flush()
    {
        FrameResources& frame = _frames[_currentFrame];

        if (frame.instanceCount == 0)
            return;

        VkCommandBuffer commandBuffer = frame.commandBuffer;

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = (float)_context.getSwapChainExtent().height;
        viewport.width = (float)_context.getSwapChainExtent().width;
        viewport.height = -(float)_context.getSwapChainExtent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = _context.getSwapChainExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        auto extent = _context.getSwapChainExtent();
        glm::mat4 projView = glm::ortho(0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, -1.0f, 1.0f);

        SpritePushConstants push{};
        push.projectionView = projView;
        vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SpritePushConstants), &push);

        VkBuffer vertexBuffers[] = {_quadVertexBuffer,frame.instanceBuffer};
        VkDeviceSize offsets[] = {0, 0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, _quadIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

        VkDescriptorSet descriptorSet = _textureManager.getDescriptorSet();
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, 6, static_cast<uint32_t>(frame.instanceCount), 0, 0, 0 );
    }

    void SpriteRenderer::endFrame()
    {
        if (!_isFrameStarted)
            return;

        flush();

        FrameResources& frame = _frames[_currentFrame];
        VkCommandBuffer commandBuffer = frame.commandBuffer;

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffer");

        VkSemaphore waitSemaphores[] = {frame.imageAvailable};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = {_renderFinishedSemaphores[_imageIndex]};

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(_context.getGraphicsQueue(), 1, &submitInfo, frame.inFlightFence) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit frame");

        VkSwapchainKHR swapChains[] = {_context.getSwapChain()};

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &_imageIndex;

        const VkResult result = vkQueuePresentKHR(_context.getPresentQueue(), &presentInfo);

        const bool mustRecreate =
            result == VK_ERROR_OUT_OF_DATE_KHR ||
            result == VK_SUBOPTIMAL_KHR;

        if (result != VK_SUCCESS && !mustRecreate)
            throw std::runtime_error("Failed to present swapchain image");

        _isFrameStarted = false;
        _currentFrame =(_currentFrame + 1) % MaxFramesInFlight;

        if (mustRecreate)
            recreateSwapChain();
    }

    void SpriteRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(_context.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to create buffer");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(_context.getDevice(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = _context.findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(_context.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate buffer memory");

        if (vkBindBufferMemory(_context.getDevice(), buffer, bufferMemory, 0) != VK_SUCCESS)
            throw std::runtime_error("Failed to bind buffer memory");
    }

    VkShaderModule SpriteRenderer::createShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(_context.getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            throw std::runtime_error("Failed to create shader module");
        return shaderModule;
    }

    std::vector<char> SpriteRenderer::readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("Failed to open file");

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    void SpriteRenderer::recreateSwapChain()
    {
        VkDevice device = _context.getDevice();

        vkDeviceWaitIdle(device);

        for (VkFramebuffer framebuffer : _swapChainFramebuffers)
            if (framebuffer != VK_NULL_HANDLE)
                vkDestroyFramebuffer(device, framebuffer, nullptr);

        _swapChainFramebuffers.clear();
        destroyRenderFinishedSemaphores();

        const VkFormat oldFormat = _context.getSwapChainImageFormat();

        _context.recreateSwapChain();

        if (_context.getSwapChainImageFormat() != oldFormat) {
            vkDestroyPipeline(device, _graphicsPipeline, nullptr);
            vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
            vkDestroyRenderPass(device, _renderPass, nullptr);

            _graphicsPipeline = VK_NULL_HANDLE;
            _pipelineLayout = VK_NULL_HANDLE;
            _renderPass = VK_NULL_HANDLE;

            createRenderPass();
            createGraphicsPipeline();
        }

        createFramebuffers();
        createRenderFinishedSemaphores();
    }

    void SpriteRenderer::createInstanceBuffer(FrameResources& frame, std::size_t capacity) 
    {
        const VkDeviceSize bufferSize = capacity * sizeof(SpriteData);

        createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            frame.instanceBuffer, frame.instanceMemory);

        if (vkMapMemory(_context.getDevice(), frame.instanceMemory, 0, bufferSize, 0, &frame.mappedInstances) != VK_SUCCESS) {
            vkDestroyBuffer(_context.getDevice(), frame.instanceBuffer, nullptr);

            vkFreeMemory(_context.getDevice(),frame.instanceMemory,nullptr);

            frame.instanceBuffer = VK_NULL_HANDLE;
            frame.instanceMemory = VK_NULL_HANDLE;

            throw std::runtime_error("Failed to map instance buffer with capacity " + std::to_string(capacity));
        }

        frame.instanceCapacity = capacity;
    }

    void SpriteRenderer::createCommandBuffers()
    {
        std::array<VkCommandBuffer, MaxFramesInFlight> commandBuffers{};

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = _context.getCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        checkVkR(vkAllocateCommandBuffers(_context.getDevice(), &allocInfo, commandBuffers.data()));

        for (std::size_t i = 0; i < MaxFramesInFlight; i++)
            _frames[i].commandBuffer = commandBuffers[i];
    }

    void SpriteRenderer::submit(const SpriteData& sprite)
    {
        submit(std::span<const SpriteData>(&sprite, 1));
    }

    void SpriteRenderer::submit(std::span<const SpriteData> sprites)
    {
        if (!_isFrameStarted)
            throw std::runtime_error("submit() called outside a frame");

        if (sprites.empty())
            return;

        FrameResources& frame = _frames[_currentFrame];
        if (sprites.size() > std::numeric_limits<uint32_t>::max() - frame.instanceCount)
            throw std::runtime_error("Too many instances for one draw call");

        const std::size_t requiredCapacity = frame.instanceCount + sprites.size();

        ensureInstanceCapacity(frame, requiredCapacity);

        auto* destination = static_cast<SpriteData*>(frame.mappedInstances) + frame.instanceCount;

        std::memcpy(destination, sprites.data(), sprites.size_bytes());

        frame.instanceCount += sprites.size();
        frame.uploadedBytes = frame.instanceCount * sizeof(SpriteData);
    }

    void SpriteRenderer::ensureInstanceCapacity(FrameResources& frame, std::size_t requiredCapacity)
    {
        if (requiredCapacity <= frame.instanceCapacity)
            return;

        if (requiredCapacity > std::numeric_limits<std::size_t>::max() - (SpritesPerPage - 1))
            throw std::runtime_error("Instance capacity overflow: " + std::to_string(requiredCapacity));

        const std::size_t newCapacity =
            ((requiredCapacity + SpritesPerPage - 1) / SpritesPerPage) * SpritesPerPage;

        if (newCapacity > std::numeric_limits<std::size_t>::max() / sizeof(SpriteData) ||
            newCapacity > std::numeric_limits<VkDeviceSize>::max() / sizeof(SpriteData))
            throw std::runtime_error("Instance buffer size overflow: " + std::to_string(requiredCapacity));

        VkDevice device = _context.getDevice();
        FrameResources replacement{};

        try {
            createInstanceBuffer(replacement, newCapacity);
        } catch (const std::exception& error) {
            if (replacement.mappedInstances != nullptr)
                vkUnmapMemory(device, replacement.instanceMemory);

            if (replacement.instanceBuffer != VK_NULL_HANDLE)
                vkDestroyBuffer(device, replacement.instanceBuffer, nullptr);

            if (replacement.instanceMemory != VK_NULL_HANDLE)
                vkFreeMemory(device, replacement.instanceMemory, nullptr);

            throw std::runtime_error("Failed to grow instance buffer for capacity " +std::to_string(requiredCapacity) + ": " + error.what());
        }

        if (frame.instanceCount != 0)
            std::memcpy(replacement.mappedInstances, frame.mappedInstances,
                frame.instanceCount * sizeof(SpriteData));

        if (frame.mappedInstances != nullptr)
            vkUnmapMemory(device, frame.instanceMemory);

        if (frame.instanceBuffer != VK_NULL_HANDLE)
            vkDestroyBuffer(device, frame.instanceBuffer, nullptr);

        if (frame.instanceMemory != VK_NULL_HANDLE)
            vkFreeMemory(device, frame.instanceMemory, nullptr);

        frame.instanceBuffer = replacement.instanceBuffer;
        frame.instanceMemory = replacement.instanceMemory;
        frame.mappedInstances = replacement.mappedInstances;
        frame.instanceCapacity = replacement.instanceCapacity;
        frame.reallocationCount++;
    }

    void SpriteRenderer::createRenderFinishedSemaphores()
    {
        const std::size_t imageCount = _context.getSwapChainImageViews().size();

        _renderFinishedSemaphores.assign(imageCount, VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkDevice device = _context.getDevice();

        for (VkSemaphore& semaphore : _renderFinishedSemaphores) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
                destroyRenderFinishedSemaphores();
                throw std::runtime_error("Failed to create render-finished semaphores");
            }
        }
    }

    void SpriteRenderer::destroyRenderFinishedSemaphores()
    {
        VkDevice device = _context.getDevice();

        for (VkSemaphore semaphore : _renderFinishedSemaphores) {
            if (semaphore != VK_NULL_HANDLE)
                vkDestroySemaphore(device, semaphore, nullptr);
        }

        _renderFinishedSemaphores.clear();
    }
}
