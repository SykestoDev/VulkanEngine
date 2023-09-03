#include <vulkan/vulkan.h>
#ifdef WINDOWS_BUILD
#include <vulkan/vulkan_win32.h>
#elif LINUX_BUILD // Other OS
#endif //WINDOWS_BUILD
#include <iostream>
using namespace std;

#define ArraySize(arr) sizeof((arr))/ sizeof((arr[0]))

#define VK_CHECK(result)                        \
if (result != VK_SUCCESS)                       \
{                                               \
    cout << "Vulkan Error: " << result << endl; \
    __debugbreak();                             \
    return false;                               \
}                                               \
else                                            \
{                                               \
    cout << "Success" << endl;                  \
}                                               \

static VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT msgFlags,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
    {
        cout << "Validation Error: " << pCallbackData->pMessage << endl;
        return false;
    }

struct VkContext
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkQueue graphicsQueue;
    VkSwapchainKHR swapChain;
    VkCommandPool commandPool;
    
    VkSemaphore aquireSemaphore;
    VkSemaphore submitSemaphore;

    uint32_t scImageCount;
    //TODO: Suballoaction from Main Memory
    VkImage scImages[5];
    int graphicsIndex;
};

bool vk_Init(VkContext* vkContext, void* window)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 241);; // major version, minor version
    appInfo.apiVersion = VK_API_VERSION_1_3;//VK_MAKE_VERSION(major,minor,patch);
    appInfo.pEngineName = "ChefeEngine";

    uint32_t apiVersion;
    if (vkEnumerateInstanceVersion(&apiVersion) == VK_SUCCESS) {
        std::cout << "Vulkan API Version: " << VK_VERSION_MAJOR(apiVersion) << "."
                  << VK_VERSION_MINOR(apiVersion) << "."
                  << VK_VERSION_PATCH(apiVersion) << std::endl;
    } else {
        std::cout << "Unable to query Vulkan API version." << std::endl;
    }

    const char* extensions[] = {
#ifdef WINDOWS_BUILD
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif LINUX_BUILD
#endif
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME
    };

    const char* layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.ppEnabledExtensionNames = extensions;
    instanceInfo.enabledExtensionCount = ArraySize(extensions);
    instanceInfo.ppEnabledLayerNames = layers;
    instanceInfo.enabledLayerCount = ArraySize(layers);

    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &vkContext->instance));

    auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkContext->instance, "vkCreateDebugUtilsMessengerEXT");

    if (vkCreateDebugUtilsMessengerEXT)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugInfo.pfnUserCallback = vkDebugCallback;
        vkCreateDebugUtilsMessengerEXT(vkContext->instance, &debugInfo, 0, &vkContext->debugMessenger);
    }
    else 
    {
        return false;
    }

    /* Create Surface */
    {
#ifdef WINDOWS_BUILD
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hwnd = (HWND)window;//GetActiveWindow();//glfwGetWin32Window((GLFWwindow*)window);
    surfaceInfo.hinstance = GetModuleHandle(NULL);
    VK_CHECK(vkCreateWin32SurfaceKHR(vkContext->instance, &surfaceInfo, 0, &vkContext->surface));
#elif LINUX_BUILD
#endif
    }

    /* Choose GPU */
    {
        vkContext->graphicsIndex = -1;

        uint32_t gpuCount = 0;
        // TODO: Suballocation fro Main Function
        VkPhysicalDevice gpus[10];
        VK_CHECK(vkEnumeratePhysicalDevices(vkContext->instance, &gpuCount, 0));
        VK_CHECK(vkEnumeratePhysicalDevices(vkContext->instance, &gpuCount, gpus));

        for (uint32_t i = 0; i < gpuCount; i++)
        {
            VkPhysicalDevice gpu = gpus[i];

            uint32_t queueFamilyCount = 0;
            // TODO: Suballocation fro Main Function
            VkQueueFamilyProperties queueProps[10];
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, 0);
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueProps);

            for (uint32_t j = 0; j < queueFamilyCount; j++)
            {
                if (queueProps[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    VkBool32 surfaceSupport = VK_FALSE;
                    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, j, vkContext->surface, &surfaceSupport));

                    if (surfaceSupport)
                    {
                        vkContext->graphicsIndex = j;
                        vkContext->gpu = gpu;
                        break;
                    }
                    
                }
                
            }
            
        }

        if (vkContext->graphicsIndex < 0)
        {
            return false;
        }
    }

    /* Logical Device */
    {
        float queuePriority = 0.0f;

        VkDeviceQueueCreateInfo queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = vkContext->graphicsIndex;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;

        const char* extensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.pQueueCreateInfos = &queueInfo;
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.ppEnabledExtensionNames = extensions;
        deviceInfo.enabledExtensionCount = ArraySize(extensions);

        
        VK_CHECK(vkCreateDevice(vkContext->gpu, &deviceInfo, 0, &vkContext->device));

        vkGetDeviceQueue(vkContext->device, vkContext->graphicsIndex, 0, &vkContext->graphicsQueue);
    }

    /* Swapchain */
    {
        uint32_t formatCounts;
        // TODO: Suballocation from Mian Function
        VkSurfaceFormatKHR surfaceFormats[10];
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkContext->gpu, vkContext->surface, &formatCounts, nullptr));
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkContext->gpu, vkContext->surface, &formatCounts, surfaceFormats));

        for (uint32_t i = 0; i < formatCounts; i++)
        {
            VkSurfaceFormatKHR format = surfaceFormats[i];

            if (format.format == VK_FORMAT_B8G8R8A8_SRGB)
            {
                vkContext->surfaceFormat = format;
                break;
            }
        }   
        

        uint32_t imgCount = 0;
        VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkContext->gpu, vkContext->surface, &surfaceCapabilities));
        
        imgCount = surfaceCapabilities.minImageCount + 1;
        imgCount = imgCount > surfaceCapabilities.maxImageCount ? imgCount - 1 : imgCount;

        VkSwapchainCreateInfoKHR scInfo = {};
        scInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        scInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        scInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        scInfo.surface = vkContext->surface;
        scInfo.imageFormat = vkContext->surfaceFormat.format;
        scInfo.preTransform = surfaceCapabilities.currentTransform;
        scInfo.imageExtent = surfaceCapabilities.currentExtent;
        scInfo.minImageCount = imgCount;
        scInfo.imageArrayLayers = 1;

        VK_CHECK(vkCreateSwapchainKHR(vkContext->device, &scInfo, 0, &vkContext->swapChain));

        VK_CHECK(vkGetSwapchainImagesKHR(vkContext->device, vkContext->swapChain, &vkContext->scImageCount, VK_NULL_HANDLE));
        VK_CHECK(vkGetSwapchainImagesKHR(vkContext->device, vkContext->swapChain, &vkContext->scImageCount, vkContext->scImages));
    }

    /* Command Pool*/
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = vkContext->graphicsIndex;
        VK_CHECK(vkCreateCommandPool(vkContext->device, &poolInfo, 0, &vkContext->commandPool));
    }

    /* Syn Objects*/
    {
        VkSemaphoreCreateInfo semaInfo = {};
        semaInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CHECK(vkCreateSemaphore(vkContext->device, &semaInfo, 0, &vkContext->aquireSemaphore));
        VK_CHECK(vkCreateSemaphore(vkContext->device, &semaInfo, 0, &vkContext->submitSemaphore));
    }

    return true;
}

bool vk_render(VkContext* vkContext)
{
    uint32_t imgIndex;
    VK_CHECK(vkAcquireNextImageKHR(vkContext->device, vkContext->swapChain, 0, vkContext->aquireSemaphore, 0, &imgIndex)); 

    VkCommandBuffer cmd;
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = vkContext->commandPool;
    VK_CHECK(vkAllocateCommandBuffers(vkContext->device, &allocInfo, &cmd));
    
    VkCommandBufferBeginInfo cmdBuffBeginInfo = {};
    cmdBuffBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBuffBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; 
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBuffBeginInfo));

    /* Rendering Commands */
    {
        VkClearColorValue color = {1, 0, 0, 1};
        VkImageSubresourceRange range = {};
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.layerCount = 1;
        range.levelCount = 1;
        vkCmdClearColorImage(cmd, vkContext->scImages[imgIndex], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &color, 1, &range);
    }

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;


    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.pSignalSemaphores = &vkContext->submitSemaphore;
    submitInfo.signalSemaphoreCount =  1;
    submitInfo.pWaitSemaphores = &vkContext->aquireSemaphore;
    submitInfo.waitSemaphoreCount = 1;
    VK_CHECK(vkQueueSubmit(vkContext->graphicsQueue, 1, &submitInfo, 0));

    VkPresentInfoKHR vkPresentInfo = {};
    vkPresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    vkPresentInfo.pSwapchains = &vkContext->swapChain;
    vkPresentInfo.swapchainCount = 1;
    vkPresentInfo.pImageIndices = &imgIndex;
    vkPresentInfo.pWaitSemaphores = &vkContext->submitSemaphore;
    vkPresentInfo.waitSemaphoreCount = 1;
    VK_CHECK(vkQueuePresentKHR(vkContext->graphicsQueue, &vkPresentInfo));  
    
    vkFreeCommandBuffers(vkContext->device, vkContext->commandPool, 1, &cmd);

    return true;
}