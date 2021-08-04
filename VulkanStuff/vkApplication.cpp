#include "pch.h"
#include "vkApplication.h"
#include "Debug.h"

void vkApplication::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

bool vkApplication::checkValidationLayersSupport() const
{
    uint32_t validationLayerCount;
    vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);

    std::vector<VkLayerProperties> availableValidationLayers(validationLayerCount);
    vkEnumerateInstanceLayerProperties(&validationLayerCount, availableValidationLayers.data());

    for(const char* validationLayer : vkValidationLayers)
    {
        bool layerFound = false;
        for(const auto& availableValidationLayer : availableValidationLayers)
        {
            if(strcmp(validationLayer, availableValidationLayer.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if(!layerFound)
        {
            return false;
        }
    }

    return true;
}

const std::vector<const char*> vkApplication::getRequiredExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if(vkValidationLayersEnabled)
    {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return requiredExtensions;
}

void vkApplication::setupDebugMessenger()
{
    if(!vkValidationLayersEnabled)
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfo;
    populateDebugMessengerCreateInfo(vkDebugUtilsMessengerCreateInfo);

    if(createDebugUtilsMessengerEXT(vkInstance, &vkDebugUtilsMessengerCreateInfo, nullptr, &vkDebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("DebugMessenger: Failed to set up debug messenger!");
    }
}

void vkApplication::findPhysicalDevice()
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);

    if(physicalDeviceCount == 0)
    {
        throw std::runtime_error("PhysicalDevice: Failed to find GPUs with Vulkan support");
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices.data());

    std::multimap<int, VkPhysicalDevice> candidates;

    for(const auto& physicalDevice : physicalDevices)
    {
        int candidateScore = getPhysicalDeviceScore(physicalDevice);
        candidates.insert(std::make_pair(candidateScore, physicalDevice));
    }

    //TODO: Allow user to choose which GPU to use. Print option list and scores.
    if(candidates.rbegin()->first > 0 && isDeviceSupportingRequirements(candidates.rbegin()->second))
    {
        vkPhysicalDevice = candidates.rbegin()->second;
    }
    else
    {
        throw std::runtime_error("PhysicalDevice: Failed to find GPU supporting requirements");
    }
}

const uint32_t vkApplication::getPhysicalDeviceScore(const VkPhysicalDevice& physicalDevice) const
{
    VkPhysicalDeviceProperties physicalDeviceProperties = {};
    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

    //TODO: Evaluate other parameters
    uint32_t score = 0;

    //Performance Parameters
    if(physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }

    //Important
    score += physicalDeviceProperties.limits.maxImageDimension2D;

    //Required Parameters - without them application can't work.
    if(!physicalDeviceFeatures.geometryShader)
    {
        return 0;
    }

    return score;
}

const vkApplication::QueueFamilyIndices vkApplication::getQueueFamilies(const VkPhysicalDevice& physicalDevice) const
{
    QueueFamilyIndices queueFamilyIndices = {};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    VkBool32 presentSupport = false;
    uint32_t i = 0;
    for(const auto& queueFamilyProperty : queueFamilyProperties)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, vkSurface, &presentSupport);

        if(queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFamilyIndices.graphicsFamily = i;
        }

        if(presentSupport)
        {
            queueFamilyIndices.presentFamily = i;
        }

        if(queueFamilyIndices.IsComplete())
        {
            break;
        }

        ++i;
    }

    return queueFamilyIndices;
}

bool vkApplication::isDeviceSupportingRequirements(const VkPhysicalDevice& physicalDevice) const
{
    const QueueFamilyIndices queueFamilyIndices = getQueueFamilies(physicalDevice);

    const bool extensionsSupported = checkDeviceExtensionsSupport(physicalDevice);

    bool swapchainSufficient = false;
    if(extensionsSupported)
    {
        const SwapchainSupportDetails swapChainSupportDetails = querySwapchainSupport(physicalDevice);
        swapchainSufficient = !swapChainSupportDetails.vkFormats.empty() && !swapChainSupportDetails.vkPresentModes.empty();
    }

    return queueFamilyIndices.IsComplete() && extensionsSupported && swapchainSufficient;
}

void vkApplication::createLogicalDevice()
{
    QueueFamilyIndices queueFamilyIndices = getQueueFamilies(vkPhysicalDevice);
    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos = {};

    std::set<uint32_t> uniqueQueueFamilies = {
        queueFamilyIndices.graphicsFamily.value(),
        queueFamilyIndices.presentFamily.value()
    };

    float queuePriority = 1.0f;

    for (uint32_t uniqueQueueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo deviceQueueCreateInfo
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            nullptr,
            NULL,
            uniqueQueueFamily,
            1,
            &queuePriority
        };
        deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
    }

    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};

    VkDeviceCreateInfo deviceCreateInfo
    {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        nullptr,
        NULL,
        static_cast<uint32_t>(deviceQueueCreateInfos.size()),
        deviceQueueCreateInfos.data(),
        0,
        nullptr,
        static_cast<uint32_t>(vkDeviceExtensions.size()),
        vkDeviceExtensions.data(),
        &physicalDeviceFeatures
    };

    if(vkValidationLayersEnabled)
    {
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(vkValidationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = vkValidationLayers.data();
    }

    if(vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkLogicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Logical Device: Failed to create logical device");
    }

    vkGetDeviceQueue(vkLogicalDevice, queueFamilyIndices.graphicsFamily.value(), 0, &vkGraphicsQueue);
    vkGetDeviceQueue(vkLogicalDevice, queueFamilyIndices.presentFamily.value(), 0, &vkPresentQueue);
}

void vkApplication::createSurface()
{
    if(glfwCreateWindowSurface(vkInstance, window, nullptr, &vkSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("Surface: Failed to create window surface!");
    }
}


bool vkApplication::checkDeviceExtensionsSupport(const VkPhysicalDevice& physicalDevice) const
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(vkDeviceExtensions.begin(), vkDeviceExtensions.end());

    for (const auto& availableExtension : availableExtensions)
    {
        requiredExtensions.erase(availableExtension.extensionName);
    }

    return requiredExtensions.empty();
}


const vkApplication::SwapchainSupportDetails vkApplication::querySwapchainSupport(VkPhysicalDevice physicalDevice) const
{
    SwapchainSupportDetails swapchainSupportDetails = {};

    //Check basic surface Capabilities like min/max number of images in swapchain min/max width and size of images
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vkSurface, &swapchainSupportDetails.vkSurfaceCapabilities);

    //Check Formats Support
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vkSurface, &formatCount, nullptr);

    if(formatCount != 0)
    {
        swapchainSupportDetails.vkFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vkSurface, &formatCount, swapchainSupportDetails.vkFormats.data());
    }

    //Check Present Modes Support
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vkSurface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        swapchainSupportDetails.vkPresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vkSurface, &presentModeCount, swapchainSupportDetails.vkPresentModes.data());
    }

    return swapchainSupportDetails;
}


const VkSurfaceFormatKHR vkApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const
{
    for (const auto& availableFormat : availableFormats)
    {
        if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

const VkPresentModeKHR vkApplication::chooseSwapPresentMode( const std::vector<VkPresentModeKHR>& availablePresentModes) const
{
    for(const auto& availablePresentMode : availablePresentModes)
    {
        if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

const VkExtent2D vkApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) const
{
    if(surfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        return surfaceCapabilities.currentExtent;
    }
    else
    {
        int width = 0;
        int height = 0;

        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D currentExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        currentExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, currentExtent.width));
        currentExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.minImageExtent.height, currentExtent.height));

        return currentExtent;
    }
}

void vkApplication::createSwapchain()
{
    const SwapchainSupportDetails swapChainSupportDetails = querySwapchainSupport(vkPhysicalDevice);

    const VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.vkFormats);
    const VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupportDetails.vkPresentModes);
    const VkExtent2D extent = chooseSwapExtent(swapChainSupportDetails.vkSurfaceCapabilities);

    // Its recommended to have 1 more than the minimum in case we have to wait for
    // driver to complete internal operation before we can acquire another image to render
    uint32_t imageCount = swapChainSupportDetails.vkSurfaceCapabilities.minImageCount + 1;

    if(swapChainSupportDetails.vkSurfaceCapabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.vkSurfaceCapabilities.maxImageCount)
    {
        imageCount = swapChainSupportDetails.vkSurfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfoKhr = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        nullptr,
        NULL,
        vkSurface,
        imageCount,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        extent,
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        swapChainSupportDetails.vkSurfaceCapabilities.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        presentMode,
        VK_TRUE,
        nullptr,
    };

    QueueFamilyIndices queueFamilyIndices = getQueueFamilies(vkPhysicalDevice);
    uint32_t uniqueQueueFamilies[] = {
        queueFamilyIndices.graphicsFamily.value(),
        queueFamilyIndices.presentFamily.value()
    };
    if(queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
    {
        swapchainCreateInfoKhr.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfoKhr.queueFamilyIndexCount = 2;
        swapchainCreateInfoKhr.pQueueFamilyIndices = uniqueQueueFamilies;
    }

    if(vkCreateSwapchainKHR(vkLogicalDevice, &swapchainCreateInfoKhr, nullptr, &vkSwapchainKHR) != VK_SUCCESS)
    {
        throw std::runtime_error("Swapchain: Failed to create swap chain");
    }

    vkGetSwapchainImagesKHR(vkLogicalDevice, vkSwapchainKHR, &imageCount, nullptr);
    vkSwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(vkLogicalDevice, vkSwapchainKHR, &imageCount, vkSwapchainImages.data());

    vkSwapchainImageFormat = surfaceFormat.format;
    vkSwapchainExtent = extent;
}

void vkApplication::createImageViews()
{
    vkSwapchainImageViews.resize(vkSwapchainImages.size());

    for(size_t i = 0; i < vkSwapchainImages.size(); ++i)
    {
        VkImageViewCreateInfo imageViewCreateInfo = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            nullptr,
            NULL,
            vkSwapchainImages[i],
            VK_IMAGE_VIEW_TYPE_2D,
            vkSwapchainImageFormat,
            {VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0,1,0,1}
        };

        if(vkCreateImageView(vkLogicalDevice, &imageViewCreateInfo, nullptr, &vkSwapchainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Image Views: Failed to create image views!");
        }
    }
}

static std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule vkApplication::createShaderModule(const std::vector<char>& code)
{
    // Pass the pointer to the buffer with bytecode and the length of it
    VkShaderModuleCreateInfo createInfo
    {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        NULL,
        code.size(),
        reinterpret_cast<const uint32_t*>(code.data())
    };

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(vkLogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Shader Module: failed to create shader module!");
    }

    return shaderModule;
}


/// <summary>
/// The graphics pipeline is the sequence of operations that take the vertices and textures of meshes all the way to the pixels in the render targets.
/// 
/// Consist of multiple stages:
///    *Input assembler - collects the raw vertex data from the buffers
///     Vertex shader - run for every vertex and applies transformations to turn vertex positions from model space to screen space.
///     Tessellation shader - allow to subdivide geometry based on certain rules to increase the mesh quality.
///     Geometry shader - run on every primitive and can discard it or output more primitives than came in.
///    *Rasterization stage - discretizes the primitives into fragments. Discard every fragment which is outside the screen and also object that are covered by any other fragment in front of it.
///     Fragment shader - invoked for every fragment that survives and determines which framebuffer(s) the fragments are written to and with which color and depth values.
///    *Color blending - stage applies operations to mix different fragments that map to the same pixel in the framebuffer.
/// 
/// * - fixed-function stages which allows to tweak operations using parameters, but the way they work is predefined.
/// 
/// </summary>
void vkApplication::createGraphicsPipeline()
{
    auto vertShaderCode = readFile("Shaders/vert.spv");
    auto fragShaderCode = readFile("Shaders/frag.spv");

    // Wrap shader code into VkShaderModule objects to send it to the pipeline
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    // Fill vertex shader structure to define in which pipeline stage the vertex shaders is going to be used.
    VkPipelineShaderStageCreateInfo vertShaderStageInfo
    {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        NULL,
        VK_SHADER_STAGE_VERTEX_BIT,
        vertShaderModule,
        "main",
        nullptr
    };
    
    // Fill fragment shader structure to define in which pipeline stage the fragment shaders is going to be used.
    VkPipelineShaderStageCreateInfo fragShaderStageInfo
    {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        NULL,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        fragShaderModule,
        "main",
        nullptr
    };

    VkPipelineShaderStageCreateInfo shaderStages[] =
    {
        vertShaderStageInfo,
        fragShaderStageInfo
    };

    // Describe the format of the vertex data that will be passed to the vertex shader
    // Binding description: spacing between data and wheather the data is per-vertex or per-instance
    // Attribute description: type of the atributes passed to the vertex shader
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr,
        NULL,
        0,
        nullptr,
        0,
        nullptr
    };

    // Describe what kind of geometry will be drawn from the vertices and if primitive restart should be enabled
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        nullptr,
        NULL,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE
    };

    // A viewport describes the region of the framebuffer that the output will be rendered to.
    // The minDepth and maxDepth values specify the range of depth values to use for the framebuffer.
    VkViewport viewport{
        0.0f,
        0.0f,
        static_cast<float>(vkSwapchainExtent.width),
        static_cast<float>(vkSwapchainExtent.height),
        0.0f,
        1.0f
    };

    // Scissor rectangles define in which regions pixels will actually be stored.
    // Any pixels outside the scissor rectangles will be discarded by the rasterizer.
    VkRect2D scissor{
        { 0, 0 },
        vkSwapchainExtent
    };

    // Combine viewport and scissor rectangle.
    VkPipelineViewportStateCreateInfo viewportState{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        nullptr,
        NULL,
        1,
        &viewport,
        1,
        &scissor,
    };

    // The rasterizer takes the geometry shaped by the vertices from the vertex shader and turns it into fragments to be colored by the fragment shader.
    VkPipelineRasterizationStateCreateInfo rasterizer{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        NULL,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT,
        VK_FRONT_FACE_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };

    // Configure multisampling to perform anti-aliasing.
    VkPipelineMultisampleStateCreateInfo multisampling{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr,
        NULL,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        1.0f,
        nullptr,
        VK_FALSE,
        VK_FALSE 
    };

    // Combine color returned from fragment shader with the color that is already in the framebuffer.
    // Configure settings per attached framebuffer
    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        VK_FALSE,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    // Configure global color blending settings.
    VkPipelineColorBlendStateCreateInfo colorBlending{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        nullptr,
        NULL,
        VK_FALSE,
        VK_LOGIC_OP_COPY,
        1,
        &colorBlendAttachment,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    // Specify states which can changed without recreating the pipeline.
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState{
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        nullptr,
        NULL,
        2,
        dynamicStates
    };

    // Create VkPipelineLayout object to store uniform values which can be used to pass
    // transformation matrix to the vertex shader, or to create texture samplers in the fragment shader
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        nullptr,
        NULL,
        0,
        nullptr,
        0,
        nullptr
    };

    if (vkCreatePipelineLayout(vkLogicalDevice, &pipelineLayoutInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // Having all of the above: shader stages, fixed-function states, pipeline layout, render pass
    // we can combine them to create the graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,
        NULL,
        2,
        shaderStages,
        &vertexInputInfo,
        &inputAssembly,
        nullptr,
        &viewportState,
        &rasterizer,
        &multisampling,
        nullptr,
        &colorBlending,
        nullptr,
        vkPipelineLayout,
        vkRenderPass,
        0,
        // Vulkan allows to create a new graphics pipeline by deriving from an existing pipeline.
        // It can be done by specifing the handle of an existing pipeline with basePipelineHandle
        // or reference another pipeline that is about to be created by index with basePipelineIndex.
        VK_NULL_HANDLE,
        -1
    };

    if (vkCreateGraphicsPipelines(vkLogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkGraphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(vkLogicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(vkLogicalDevice, vertShaderModule, nullptr);
}


/// <summary>
/// Render pass object is a wrapper for framebuffer attachments that will be used while rendering.
/// Allows to specify color and depth buffers, samples used for each of them and how their contents should be handled throughout the rendering operations.
/// </summary>
void vkApplication::createRenderPass()
{
    VkAttachmentDescription colorAttachment{
        NULL,
        vkSwapchainImageFormat,
        VK_SAMPLE_COUNT_1_BIT,
        // loadOp and storeOp determine what to do with the color/depth data
        // in the attachment before rendering and after rendering.
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        // stencilLoadOp / stencilStoreOp apply to stencil data
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        // initialLayout specifies which layout the image will have before the render pass begins.
        VK_IMAGE_LAYOUT_UNDEFINED,
        // finalLayout specifies the layout to automatically transition to when the render pass finishes. 
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef{
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    // A single render pass can consist of multiple subpasses which are subsequent rendering
    // operations that depend on the contents of framebuffers in previous passes
    VkSubpassDescription subpass{
        NULL,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        nullptr,
        1,
        &colorAttachmentRef,
        nullptr,
        nullptr,
        0,
        nullptr
    };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(vkLogicalDevice, &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}


void vkApplication::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    findPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
}

void vkApplication::createInstance()
{
    if(vkValidationLayersEnabled && !checkValidationLayersSupport())
    {
        throw std::runtime_error("CreateInstance: Requested validation layers not available!");
    }

    VkApplicationInfo vkApplicationInfo
    {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        nullptr,
        "vkTriangle",
        VK_MAKE_VERSION(1,0,0),
        "No Engine",
        VK_MAKE_VERSION(1,0,0),
        VK_API_VERSION_1_2
    };

    auto requiredExtensions = getRequiredExtensions();

    VkInstanceCreateInfo vkInstanceCreateInfo
    {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr,
        NULL,
        &vkApplicationInfo,
        0,
        nullptr,
        static_cast<uint32_t>(requiredExtensions.size()),
        requiredExtensions.data()
    };

    VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfo;

    if(vkValidationLayersEnabled)
    {
        populateDebugMessengerCreateInfo(vkDebugUtilsMessengerCreateInfo);
        vkInstanceCreateInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&vkDebugUtilsMessengerCreateInfo);
        vkInstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(vkValidationLayers.size());
        vkInstanceCreateInfo.ppEnabledLayerNames = vkValidationLayers.data();
    }

    if(vkCreateInstance(&vkInstanceCreateInfo, nullptr, &vkInstance) != VK_SUCCESS)
    {
        throw std::runtime_error("CreateInstance: Failed to create instance!");
    }
}

void vkApplication::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);
}

void vkApplication::mainLoop()
{
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}


void vkApplication::cleanup()
{
    vkDestroyPipeline(vkLogicalDevice, vkGraphicsPipeline, nullptr);

    vkDestroyPipelineLayout(vkLogicalDevice, vkPipelineLayout, nullptr);

    vkDestroyRenderPass(vkLogicalDevice, vkRenderPass, nullptr);

    for(auto swapchainImageView : vkSwapchainImageViews)
    {
        vkDestroyImageView(vkLogicalDevice, swapchainImageView, nullptr);
    }

    vkDestroySwapchainKHR(vkLogicalDevice, vkSwapchainKHR, nullptr);

    vkDestroyDevice(vkLogicalDevice, nullptr);

    if(vkValidationLayersEnabled)
    {
        destroyDebugUtilsMessengerEXT(vkInstance, vkDebugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);

    vkDestroyInstance(vkInstance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}
