//#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

// Validation layers
#include <vector>
#include <cstring> //for strcmp, to compare C strings
const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    const bool enableValidationLayers {false};
#else
    const bool enableValidationLayers {true};
#endif

#include <optional>

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

#include <set>

// Swap chains
const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

#include <cstdint> // for uint32_t
#include <limits> // for std::numeric_limits
#include <algorithm> // for std::clamp

class HelloTriangleApplication
{
private:
    // GLFWwindow
    GLFWwindow * window;
    const uint32_t WIDTH {800};
    const uint32_t HEIGHT {600};

    // Vulkan instance
    VkInstance vkInstance;

    // Window surface
    VkSurfaceKHR surface;

    // Physical device
    VkPhysicalDevice vkPhysicalDevice {VK_NULL_HANDLE};

    // Queue families
    QueueFamilyIndices indices;

    // Logical device, to interface with the physical device
    VkDevice vkDevice;

    // Graphics queue
    VkQueue graphicsQueue;

    // Presentation queue
    VkQueue presentQueue;

    // Swap chain
    VkSwapchainKHR swapChain;

public:
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initWindow()
    {
        glfwInit();

        // Don't create context for OpenGL, since we are using Vulkan
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // Disable window resizing
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        // Create a window
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan()
    {
        createVkInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
    }

    void createVkInstance()
    {
        if(enableValidationLayers)
        {
            bool hasValidationSupport = checkValidationLayerSupport();
            if(!hasValidationSupport)
            {
                throw std::runtime_error("Validation layers requested, but not available");
            }
        }

        VkApplicationInfo appInfo {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
        appInfo.pEngineName = "No engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceCreateInfo {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount {0};
        const char ** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
        instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;

        if(enableValidationLayers)
        {
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            instanceCreateInfo.enabledLayerCount = 0;
        }

        VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance);
        if(result != VK_SUCCESS)
        {
            std::cout << "Failed to create Vulkan instance" << std::endl;
        }
    }

    bool checkValidationLayerSupport()
    {
        // Count the number of validation layers available
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        // Initialize a vector of VkLayerProperties with layerCount entries
        std::vector<VkLayerProperties> availableLayers(layerCount);
        // Store the validation layer properties in the vector
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        std::cout << "available layers:\n";
        for(const VkLayerProperties& layerProperties : availableLayers)
        {
            std::cout << layerProperties.layerName << "\n";
        }
        std::cout << std::endl;

        // Check if the validation layers are available
        for(const char * layerName : validationLayers)
        {
            bool foundLayer = false;
            // search for layer in availableLayers
            for(const VkLayerProperties& layerProperties : availableLayers)
            {
                // if found layer, break and search for the next layer
                if(strcmp(layerName,layerProperties.layerName) == 0)
                {
                    foundLayer = true;
                    break;
                }
            }
            if(!foundLayer)
            {
                return false;
            }
        }

        return true;
    }

    void createSurface()
    {
        // nullptr refers to the custom allocator
        VkResult result = glfwCreateWindowSurface(vkInstance, window, nullptr, &surface);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create window surface");
        }
    }

    void pickPhysicalDevice()
    {
        // Count the devices with Vulkan support
        uint32_t deviceCount {0};
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
        if(deviceCount == 0)
        {
            throw std::runtime_error("Failed to find GPUs with Vulkan support");
        }
        
        // Initialize a vector of VkPhysicalDevice with deviceCount entries
        std::vector<VkPhysicalDevice> devices(deviceCount);
        // Store the devices with Vulkan support into the devices vector
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

        // select the first suitable device
        for(VkPhysicalDevice device : devices)
        {
            if(isDeviceSuitable(device))
            {
                vkPhysicalDevice = device;
                break;
            }
        }

        if(vkPhysicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("Failed to find a suitable GPU");
        }
    }

    bool isDeviceSuitable(VkPhysicalDevice physicalDevice)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

        //bool isDiscreteGPU = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        bool supportsGeometryShaders = deviceFeatures.geometryShader;
        
        std::cout << std::boolalpha;
        //std::cout << "is discrete gpu: " << isDiscreteGPU << std::endl;
        std::cout << "has geometry shaders: " << supportsGeometryShaders << std::endl;

        // Queue families
        indices = findQueueFamilies(physicalDevice);

        std::cout << "has queue families: " << indices.isComplete() << std::endl;

        // Swap chains
        bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);
        bool swapChainAdequate {false};
        if(extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(physicalDevice);
            swapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
        }

        bool isSuitable =  supportsGeometryShaders && indices.isComplete() && swapChainAdequate;

        return isSuitable;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount {0};
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        int i {0};
        for(const VkQueueFamilyProperties& queueFamily : queueFamilies)
        {
            // Graphics family
            if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            // Presentation family
            VkBool32 presentSupport {false};
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if(presentSupport)
            {
                indices.presentFamily = i;
            }

            // Early stop
            if(indices.isComplete())
            {
                break;
            }

            i++;
        }

        return indices;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        // copy the deviceExtensions vector to a new requiredExtensions set
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        // remove from the set the extensions which are available
        for(const VkExtensionProperties& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        // if all extensions were removed, then all required extensions are available
        return requiredExtensions.empty();
    }

    void createLogicalDevice()
    {
        // Create the queues (graphics and presentation)
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        const float queuePriority = 1.0f;
        for(uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }
        
        // again?
        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if(enableValidationLayers)
        {
            deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            deviceCreateInfo.enabledLayerCount = 0;
        }

        VkResult result = vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkDevice);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create logical device");
        }

        // Get a queue handle
        vkGetDeviceQueue(vkDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(vkDevice, indices.presentFamily.value(), 0, &presentQueue);
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice)
    {
        SwapChainSupportDetails details;

        // Capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

        // Formats
        uint32_t formatCount {0};
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        if(formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
        }

        // Present mode
        uint32_t presentModeCount {0};
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

        if(presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());

        }

        return details;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> & availableFormats)
    {
        // First check for preferable format
        for(const VkSurfaceFormatKHR & availableFormat : availableFormats)
        {
            if(
                availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
            )
            {
                return availableFormat;
            }
        }

        // If the preferable format isn't avaible, pick the first one available
        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> & availablePresentModes)
    {
        for(const VkPresentModeKHR & availablePresentMode : availablePresentModes)
        {
            if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities)
    {
        if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent =
            {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
            };

            actualExtent.width = std::clamp(
                                            actualExtent.width,
                                            capabilities.minImageExtent.width,
                                            capabilities.maxImageExtent.width
                                        );
            actualExtent.width = std::clamp(
                                            actualExtent.height,
                                            capabilities.minImageExtent.height,
                                            capabilities.maxImageExtent.height
                                        );

            return actualExtent;
        }
    }

    void createSwapChain()
    {
        SwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(vkPhysicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupportDetails.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupportDetails.capabilities);

        // Choose image count as minimum + 1
        uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
        // But if it exceeds the maximum, use the maximum
        if(
            swapChainSupportDetails.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupportDetails.capabilities.maxImageCount
        )
        {
            imageCount = swapChainSupportDetails.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchainCreateInfo {};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = surface;
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageFormat = surfaceFormat.format;
        swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent = extent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        if(indices.graphicsFamily != indices.presentFamily)
        {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainCreateInfo.queueFamilyIndexCount = 2;
            swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchainCreateInfo.queueFamilyIndexCount = 0; // optional
            swapchainCreateInfo.pQueueFamilyIndices = nullptr; // optional
        }

        // Don't apply transformation to image, like rotation
        swapchainCreateInfo.preTransform = swapChainSupportDetails.capabilities.currentTransform;
        // Don't blend the alpha channel with other windows in the window system
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.presentMode = presentMode;
        // Don't care about pixels behind another window, it is better for performance
        swapchainCreateInfo.clipped = VK_TRUE;
        // Don't create a new swap chain if it becomes invalid
        swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        // Create the swap chain
        VkResult result = vkCreateSwapchainKHR(vkDevice, &swapchainCreateInfo, nullptr, &swapChain);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create swap chain");
        }
    }

    void mainLoop()
    {
        // Keep the window open
        while(!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        }
    }

    void cleanup()
    {
        // Destroy the swap chain
        vkDestroySwapchainKHR(vkDevice, swapChain, nullptr);

        // Don't need to cleanup the graphics queue.
        // it is destroyed when the (logical?) device is destroyed

        // Destroy the logical device
        vkDestroyDevice(vkDevice, nullptr);

        // Don't need to cleanup the physical device,
        // it is already destroyed together with the Vulkan instance

        // Destroy the window surface
        vkDestroySurfaceKHR(vkInstance, surface, nullptr);

        // Destroy the Vulkan instance
        // The nullptr refers to the callback allocator
        vkDestroyInstance(vkInstance, nullptr);

        // Destroy the GLFW window
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main()
{
    HelloTriangleApplication app;

    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
