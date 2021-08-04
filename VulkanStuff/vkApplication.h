#pragma once

class vkApplication
{
public:
    void                                run();

private:
    //Window
    GLFWwindow*                         window                      = nullptr;
    const uint32_t                      WINDOW_WIDTH                = 800;
    const uint32_t                      WINDOW_HEIGHT               = 600;
    VkDebugUtilsMessengerEXT            vkDebugMessenger            = nullptr;

    //Validation Layers
    const std::vector<const char*>      vkValidationLayers          = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
    const bool                          vkValidationLayersEnabled   = false;
#else
    const bool                          vkValidationLayersEnabled   = true;
#endif

    //VkMembers
    VkInstance                          vkInstance                  = nullptr;
    VkPhysicalDevice                    vkPhysicalDevice            = nullptr;

    struct QueueFamilyIndices
    {
        std::optional<uint32_t>         graphicsFamily;
        std::optional<uint32_t>         presentFamily;

        const bool IsComplete() const
        {
            return (graphicsFamily.has_value() && presentFamily.has_value());
        }
    };

    VkDevice                            vkLogicalDevice             = nullptr;
    VkQueue                             vkGraphicsQueue             = nullptr;
    VkQueue                             vkPresentQueue              = nullptr;
    VkSurfaceKHR                        vkSurface                   = nullptr;

    const std::vector<const char*>      vkDeviceExtensions          = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        vkSurfaceCapabilities       = {};
        std::vector<VkSurfaceFormatKHR> vkFormats                   = {};
        std::vector<VkPresentModeKHR>   vkPresentModes              = {};
    };

    //Swapchain
    VkSwapchainKHR                      vkSwapchainKHR              = nullptr;
    std::vector<VkImage>                vkSwapchainImages           = {};
    VkFormat                            vkSwapchainImageFormat      = VK_FORMAT_UNDEFINED;
    VkExtent2D                          vkSwapchainExtent           = {0,0};

    //Image View
    std::vector<VkImageView>            vkSwapchainImageViews       = {};

    //Render Pass
    VkRenderPass                        vkRenderPass                = {};

    //Pipeline Layout
    VkPipelineLayout                    vkPipelineLayout            = {};

    //Graphics Pipeline
    VkPipeline                          vkGraphicsPipeline          = {};

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //Window
    void                                initWindow();
    bool                                checkValidationLayersSupport()                                                          const;
    const std::vector<const char*>      getRequiredExtensions()                                                                 const;
    void                                setupDebugMessenger();

    //Physical Device
    void                                findPhysicalDevice();
    const uint32_t                      getPhysicalDeviceScore(const VkPhysicalDevice& physicalDevice)                          const;
    const QueueFamilyIndices            getQueueFamilies(const VkPhysicalDevice& physicalDevice)                                const;
    bool                                isDeviceSupportingRequirements(const VkPhysicalDevice& physicalDevice)                  const;

    //Logical Device
    void                                createLogicalDevice();

    //Surface
    void                                createSurface();

    //Swapchain
    bool                                checkDeviceExtensionsSupport(const VkPhysicalDevice& physicalDevice)                    const;
    const SwapchainSupportDetails       querySwapchainSupport(VkPhysicalDevice physicalDevice)                                  const;
    const VkSurfaceFormatKHR            chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)        const;
    const VkPresentModeKHR              chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)       const;
    const VkExtent2D                    chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)                   const;
    void                                createSwapchain();

    //Image View
    void                                createImageViews();

    //Graphics Pipeline
    void                                createGraphicsPipeline();
    VkShaderModule                      createShaderModule(const std::vector<char>& code);

    //Render Pass
    void                                createRenderPass();

    //Base
    void                                initVulkan();
    void                                createInstance();
    void                                mainLoop();
    void                                cleanup();
};
