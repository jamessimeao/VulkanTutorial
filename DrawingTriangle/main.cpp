//#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

//Validation layers
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

    bool isComplete()
    {
        return graphicsFamily.has_value();
    }
};

class HelloTriangleApplication
{
private:
    // GLFWwindow
    GLFWwindow * window;
    const uint32_t WIDTH {800};
    const uint32_t HEIGHT {600};

    // Vulkan instance
    VkInstance vkInstance;

    // Physical device
    VkPhysicalDevice vkPhysicalDevice {VK_NULL_HANDLE};

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
        pickPhysicalDevice();
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

        VkInstanceCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount {0};
        const char ** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        if(enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &vkInstance);
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

    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        //bool isDiscreteGPU = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        bool supportsGeometryShaders = deviceFeatures.geometryShader;

        
        std::cout << std::boolalpha;
        //std::cout << "discrete gpu: " << isDiscreteGPU << std::endl;
        std::cout << "geometry shaders: " << supportsGeometryShaders << std::endl;


        // Queue families
        QueueFamilyIndices indices = findQueueFamilies(device);

        std::cout << "has queue families: " << indices.isComplete() << std::endl;

        bool isSuitable =  supportsGeometryShaders && indices.isComplete();

        return isSuitable;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount {0};
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i {0};
        for(const VkQueueFamilyProperties& queueFamily : queueFamilies)
        {
            if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
                break;
            }
            i++;
        }

        return indices;
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
        // Don't need to cleanup the physical device,
        // it is already destroyed together with the Vulkan instance

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
