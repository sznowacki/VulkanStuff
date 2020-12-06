#pragma once
#include <cstdint>

//GLFW automatically include Vulkan header
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> 

class vkApplication
{
public:
	//Members
    GLFWwindow* window = nullptr;

    const uint32_t WINDOW_WIDTH = 800;
	const uint32_t WINDOW_HEIGHT = 600;

	//Methods
	void Run();

private:
	//Members
	VkInstance vkInstance = nullptr;

	//Methods
	void InitVulkan();
    void EnumerateSupportedInstanceExtensions();
    void CreateInstance();
	void InitWindow();
	void MainLoop();
	void Cleanup();
};
