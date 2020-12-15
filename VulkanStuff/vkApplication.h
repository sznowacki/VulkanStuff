#pragma once

class vkApplication
{
public:
    void run();

private:
    //Window
    GLFWwindow* window = nullptr;
    const uint32_t WINDOW_WIDTH = 800;
    const uint32_t WINDOW_HEIGHT = 600;
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;

    //Validation Layers
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
    const bool validationLayersEnabled = false;
#else
    const bool validationLayersEnabled = true;
#endif

    //VkMembers
    VkInstance vkInstance = nullptr;
    VkPhysicalDevice vkPhysicalDevice = nullptr;

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        const bool IsComplete() const
        {
            return (graphicsFamily.has_value() && presentFamily.has_value());
        }
    };

    VkDevice vkLogicalDevice = nullptr;
    VkQueue graphicsQueue = nullptr;
    VkSurfaceKHR vkSurfaceKHR = nullptr;
    VkQueue presentQueue = nullptr;

    //Window
    void initWindow();
    const bool checkValidationLayersSupport() const;
    const std::vector<const char*> getRequiredExtensions() const;
    void setupDebugMessenger();

    //Physical Device
    void findPhysicalDevice();
    const uint32_t getPhysicalDeviceScore(const VkPhysicalDevice& physicalDevice) const;
    const QueueFamilyIndices GetQueueFamilies(const VkPhysicalDevice& physicalDevice) const;
    const bool isDeviceSupportingRequirements(const VkPhysicalDevice& physicalDevice) const;
    

    //Logical Device
    void createLogicalDevice();

    //Surface
    void createSurface();

    //Base
    void initVulkan();
    void createInstance();
    void mainLoop();
    void cleanup();
};
