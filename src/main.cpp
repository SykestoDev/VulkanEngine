#include <iostream>
#include <vulkan/vulkan.h>

using namespace std;

int main()
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
    
    VkInstance instance;

    VkResult result = vkCreateInstance(&instanceInfo, 0, &instance);
    if(result == VK_SUCCESS)
    {
        cout << "Successfully created Vulkan Instance." << endl;
    }

    cout << "Hello World!" << endl;
    return 0;
}