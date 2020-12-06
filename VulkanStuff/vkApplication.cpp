#include "vkApplication.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> //GLFW automatically include Vulkan header

void vkApplication::Run()
{
	InitWindow();
    InitVulkan();
	MainLoop();
	Cleanup();
}

void vkApplication::InitVulkan()
{
	
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
	glfwDestroyWindow(window);

	glfwTerminate();
}
