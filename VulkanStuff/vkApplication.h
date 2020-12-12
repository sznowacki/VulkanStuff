#pragma once

class vkApplication
{
public:
    //Methods
    void Run();

private:
    //Window
    GLFWwindow* window = nullptr;

    const uint32_t WINDOW_WIDTH = 800;
    const uint32_t WINDOW_HEIGHT = 600;

    //VkMembers
    VkInstance vkInstance = nullptr;
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;

    VkPhysicalDevice vkPhysicalDevice = nullptr;
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        bool IsComplete()
        {
            return graphicsFamily.has_value();
        }
    };

    //Validation Layers
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    const bool validationLayersEnabled = false;
#else
    const bool validationLayersEnabled = true;
#endif

    //Methods
    bool CheckValidationLayersSupport();
    std::vector<const char*> GetRequiredExtensions();
    void SetupDebugMessenger();
    void GetPhysicalDevice();
    uint32_t GetDeviceScore(const VkPhysicalDevice& vkPhysicalDevice);
    QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& physicalDevice);
    bool IsDeviceSupportingRequirements(const VkPhysicalDevice& physicalDevice);
    void InitVulkan();
    void CreateInstance();
    void InitWindow();
    void MainLoop();
    void Cleanup();
};
