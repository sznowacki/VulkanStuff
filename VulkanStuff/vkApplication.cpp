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

const bool vkApplication::checkValidationLayersSupport() const
{
    uint32_t validationLayerCount;
    vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);

    std::vector<VkLayerProperties> availableValidationLayers(validationLayerCount);
    vkEnumerateInstanceLayerProperties(&validationLayerCount, availableValidationLayers.data());

    for(const char* validationLayer : validationLayers)
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

    if(validationLayersEnabled)
    {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return requiredExtensions;
}

void vkApplication::setupDebugMessenger()
{
    if(!validationLayersEnabled)
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfo;
    PopulateDebugMessengerCreateInfo(vkDebugUtilsMessengerCreateInfo);

    if(CreateDebugUtilsMessengerEXT(vkInstance, &vkDebugUtilsMessengerCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
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

    //TODO: Is multimap needed here?
    //Multimap is ordered map and it automatically sort candidates by increasing score
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

const vkApplication::QueueFamilyIndices vkApplication::GetQueueFamilies(const VkPhysicalDevice& physicalDevice) const
{
    QueueFamilyIndices queueFamilyIndices = {};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    uint32_t i = 0;
    for(const auto& queueFamilyProperty : queueFamilyProperties)
    {
        if(queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFamilyIndices.graphicsFamily = i;
        }

        if(queueFamilyIndices.IsComplete())
        {
            break;
        }

        ++i;
    }

    return queueFamilyIndices;
}

const bool vkApplication::isDeviceSupportingRequirements(const VkPhysicalDevice& physicalDevice) const
{
    const QueueFamilyIndices queueFamilyIndices = GetQueueFamilies(physicalDevice);

    return queueFamilyIndices.IsComplete();
}


void vkApplication::createLogicalDevice()
{
    QueueFamilyIndices queueFamilyIndices = GetQueueFamilies(vkPhysicalDevice);
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo deviceQueueCreateInfo
    {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        nullptr,
        NULL,
        queueFamilyIndices.graphicsFamily.value(),
        1,
        &queuePriority
    };

    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};

    VkDeviceCreateInfo deviceCreateInfo
    {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        nullptr,
        NULL,
        1,
        &deviceQueueCreateInfo,
        0,
        nullptr,
        0,
        nullptr,
        &physicalDeviceFeatures
    };

    if(validationLayersEnabled)
    {
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if(vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkLogicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Logical Device: Failed to create logical device");
    }

    vkGetDeviceQueue(vkLogicalDevice, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
}

void vkApplication::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    findPhysicalDevice();
    createLogicalDevice();
}

void vkApplication::createInstance()
{
    if(validationLayersEnabled && !checkValidationLayersSupport())
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

    if(validationLayersEnabled)
    {
        PopulateDebugMessengerCreateInfo(vkDebugUtilsMessengerCreateInfo);
        vkInstanceCreateInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&vkDebugUtilsMessengerCreateInfo);
        vkInstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        vkInstanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
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
    vkDestroyDevice(vkLogicalDevice, nullptr);

    if(validationLayersEnabled)
    {
        DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
    }

    vkDestroyInstance(vkInstance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}
