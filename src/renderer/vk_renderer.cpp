#include <vulkan/vulkan.h>

struct VkContext
{
    VkInstance instance;
};

bool vk_Init(VkContext* vkContext)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan";
    appInfo.applicationVersion = 1024 * 56789 + 3; // major version, minor version
    appInfo.apiVersion = VK_API_VERSION_1_0;//VK_MAKE_VERSION(major,minor,patch);
    appInfo.pEngineName = "ChefeEngine";

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;

    VkResult result = vkCreateInstance(&instanceInfo, 0, &vkContext->instance);
    if (result == VK_SUCCESS)
    {
        // Add debug output to indicate Successfully created VkInstance
        OutputDebugStringA("VkInstance successfully created.");
        return true;
    }
    // Add debug output to indicate VkInstance failed to create
    OutputDebugStringA("VkInstance failed to create.");
    return false;
}