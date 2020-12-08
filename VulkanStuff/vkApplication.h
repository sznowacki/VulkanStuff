#pragma once

class vkApplication
{
public:
	//Methods
	void Run();

private:
	//Members
	VkInstance vkInstance = nullptr;
	VkDebugUtilsMessengerEXT debugMessenger;
	GLFWwindow* window = nullptr;

	const uint32_t WINDOW_WIDTH = 800;
	const uint32_t WINDOW_HEIGHT = 600;

	const std::vector<const char*> validationLayers = {	"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool validationLayersEnabled = true;
#endif

	//Methods
	bool CheckValidationLayersSupport();
	std::vector<const char*> GetRequiredExtensions();
	void SetupDebugMessenger();
	void InitVulkan();
    void CreateInstance();
	void InitWindow();
	void MainLoop();
	void Cleanup();
};
