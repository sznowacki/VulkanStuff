#include "pch.h"
#include "vkApplication.h"
#include "Debug.h"

void vkApplication::Run()
{
	InitWindow();
    InitVulkan();
	MainLoop();
	Cleanup();
}

void vkApplication::InitVulkan()
{
	CreateInstance();
	SetupDebugMessenger();
}

bool vkApplication::CheckValidationLayersSupport()
{
	uint32_t validationLayerCount;
	vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);

	std::vector<VkLayerProperties> availableValidationLayers(validationLayerCount);
	vkEnumerateInstanceLayerProperties(&validationLayerCount, availableValidationLayers.data());

	for(const char* layerName : validationLayers)
	{
		bool layerFound = false;
		for(const auto& layerProperties : availableValidationLayers)
		{
		    if(strcmp(layerName, layerProperties.layerName) == 0)
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

std::vector<const char*> vkApplication::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (validationLayersEnabled)
	{
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return requiredExtensions;
}

void vkApplication::SetupDebugMessenger()
{
	if(!validationLayersEnabled)
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfo;
	PopulateDebugMessengerCreateInfo(vkDebugUtilsMessengerCreateInfo);

	if (CreateDebugUtilsMessengerEXT(vkInstance, &vkDebugUtilsMessengerCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void vkApplication::CreateInstance()
{
	if(validationLayersEnabled && !CheckValidationLayersSupport())
	{
		throw std::runtime_error("Requested validation layers not available!");
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

	auto requiredExtensions = GetRequiredExtensions();

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

	if (validationLayersEnabled) {
		PopulateDebugMessengerCreateInfo(vkDebugUtilsMessengerCreateInfo);
		vkInstanceCreateInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&vkDebugUtilsMessengerCreateInfo);
		vkInstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		vkInstanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}

	if(vkCreateInstance(&vkInstanceCreateInfo, nullptr, &vkInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance!");
	}
}

void vkApplication::InitWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);
}

void vkApplication::MainLoop()
{
	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}


void vkApplication::Cleanup()
{
	if(validationLayersEnabled)
	{
		DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
	}

	vkDestroyInstance(vkInstance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}
