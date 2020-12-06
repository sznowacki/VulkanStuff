#include "vkApplication.h"

#include <stdexcept>
#include <vector>

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
}

void vkApplication::EnumerateSupportedInstanceExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    printf("Available Extensions: \n");
    for(const auto& extension : extensions)
    {
        printf("\t%s\n", extension.extensionName);
    }
}

void vkApplication::CreateInstance()
{
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

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = nullptr;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	VkInstanceCreateInfo vkInstanceCreateInfo
    {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		NULL,
		&vkApplicationInfo,
		0,
		nullptr,
		glfwExtensionCount,
		glfwExtensions
	};

	EnumerateSupportedInstanceExtensions();

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
	vkDestroyInstance(vkInstance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}
