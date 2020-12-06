#pragma once
#include <cstdint>

class GLFWwindow;

class vkApplication
{
public:
	void Run();

    GLFWwindow* window = nullptr;

    const uint32_t WINDOW_WIDTH = 800;
	const uint32_t WINDOW_HEIGHT = 600;

private:
	void InitVulkan();
	void InitWindow();
	void MainLoop();
	void Cleanup();
};
