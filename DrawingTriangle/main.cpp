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

#include <fstream>

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

    // Images
    std::vector<VkImage> swapChainImages;

    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    // Image views
    std::vector<VkImageView> swapChainImageViews;

    // Render pass
    VkRenderPass renderPass;

    // Pipeline layout
    VkPipelineLayout pipelineLayout;

    // Pipeline
    VkPipeline pipeline;

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
        std::cout << "create instance" << std::endl;
        createVkInstance();
        std::cout << "create surface" << std::endl;
        createSurface();
        std::cout << "create physical device" << std::endl;
        pickPhysicalDevice();
        std::cout << "create logical device" << std::endl;
        createLogicalDevice();
        std::cout << "create swap chain" << std::endl;
        createSwapChain();
        std::cout << "create image views" << std::endl;
        createImageViews();
        std::cout << "create render pass" << std::endl;
        createRenderPass();
        std::cout << "create graphics pipeline" << std::endl;
        createGraphicsPipeline();
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
        //std::cout << "-- is discrete gpu: " << isDiscreteGPU << std::endl;
        std::cout << "-- has geometry shaders: " << supportsGeometryShaders << std::endl;

        // Queue families
        indices = findQueueFamilies(physicalDevice);

        std::cout << "-- has queue families: " << indices.isComplete() << std::endl;

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
        VkExtent2D swapChainExtent = chooseSwapExtent(swapChainSupportDetails.capabilities);

        swapChainImageFormat = surfaceFormat.format;

        // Choose image count as minimum + 1
        uint32_t minImageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
        // But if it exceeds the maximum, use the maximum
        if(
            swapChainSupportDetails.capabilities.maxImageCount > 0 &&
            minImageCount > swapChainSupportDetails.capabilities.maxImageCount
        )
        {
            minImageCount = swapChainSupportDetails.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchainCreateInfo {};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = surface;
        swapchainCreateInfo.minImageCount = minImageCount;
        swapchainCreateInfo.imageFormat = surfaceFormat.format;
        swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent = swapChainExtent;
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

        // Get the swap chain images
        uint32_t imageCount;
        vkGetSwapchainImagesKHR(vkDevice, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(vkDevice, swapChain, &imageCount, swapChainImages.data());
    }

    
    void createImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());

        for(size_t i {0}; i < swapChainImages.size(); i++)
        {
            std::cout << "-- creating image view " << i << std::endl;
            VkImageViewCreateInfo imageViewCreateInfo {};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = swapChainImages[i];
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = swapChainImageFormat;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            VkResult result = vkCreateImageView(vkDevice, &imageViewCreateInfo, nullptr, &swapChainImageViews[i]);
            if(result != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create image views");
            }
        }
    }

    static std::vector<char> readFile(const std::string & filename)
    {
        // open file and seek the end, to find the file size
        std::ifstream file {filename, std::ios::ate | std::ios::binary};

        if(!file.is_open())
        {
            throw std::runtime_error("Failed to open file");
        }

        // Get the file size from the head position, which is at the end of the file
        size_t file_size {(size_t) file.tellg()};
        // Make a buffer to store the content of the file
        std::vector<char> buffer(file_size);
        // Go back to the beggining of the file
        file.seekg(0); 
        // Store the content of the file in the buffer
        file.read(buffer.data(), file_size);
        // Close the file, since we already have its content
        file.close();
        // Return the buffer
        return buffer;
    }

    VkShaderModule createShaderModule(const std::vector<char> & shader_code)
    {
        //std::cout << "createShaderModule" << std::endl;
        //std::cout << "Shader code size = " << shader_code.size() << std::endl;

        VkShaderModuleCreateInfo shaderModuleCreateInfo {};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = shader_code.size();
        shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(shader_code.data());

        VkShaderModule shaderModule;
        VkResult result = vkCreateShaderModule(vkDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create shader module.");
        }
        return shaderModule;
    }

    void createRenderPass()
    {
        VkAttachmentDescription attachmentDescription {};
        attachmentDescription.format = swapChainImageFormat;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT; // we're not using multisampling
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentReference {};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorAttachmentReference;

        VkRenderPassCreateInfo renderPassCreateInfo {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &attachmentDescription;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;

        VkResult result = vkCreateRenderPass(vkDevice, &renderPassCreateInfo, nullptr, &renderPass);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create render pass.");
        }
    }
    
    void createGraphicsPipeline()
    {
        std::vector<char> vertShaderCode = readFile("shaders/vert.spv");
        std::vector<char> fragShaderCode = readFile("shaders/frag.spv");
        std::cout << "vert shader code size: " << vertShaderCode.size() << " bytes" << std::endl;
        std::cout << "frag shader code size: " << fragShaderCode.size() << " bytes" << std::endl;

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo {};
        vertPipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertPipelineShaderStageCreateInfo.module = vertShaderModule;
        vertPipelineShaderStageCreateInfo.pName = "main"; // function to call

        VkPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo {};
        fragPipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragPipelineShaderStageCreateInfo.module = fragShaderModule;
        fragPipelineShaderStageCreateInfo.pName = "main"; // function to call

        VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] =
        {
            vertPipelineShaderStageCreateInfo,
            fragPipelineShaderStageCreateInfo
        };

        // Use dynamic states, so don't have to configure the viewport size and etc in the pipeline
        std::vector<VkDynamicState> dynamicStates = 
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

        // No vertex data to load, because it is already in the shaders
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo {};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr; // optional
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr; // optional

        // Configure pipeline to draw triangles
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo {};
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        // Configure viewport
        VkViewport viewport {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) swapChainExtent.width;
        viewport.height = (float) swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        // Scissor
        VkRect2D scissor {};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo {};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.viewportCount = 1;
        //viewportStateCreateInfo.pViewports = &viewport; // uncomment with not using dynamic states
        viewportStateCreateInfo.scissorCount = 1;
        //viewportStateCreateInfo.pScissors = &scissor; // uncomment with not using dynamic states

        VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo {};
        rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationCreateInfo.depthClampEnable = VK_FALSE;
        rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationCreateInfo.lineWidth = 1.0f;
        rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
        rasterizationCreateInfo.depthBiasConstantFactor = 0.0f; // optional
        rasterizationCreateInfo.depthBiasClamp = 0.0f; // optional
        rasterizationCreateInfo.depthBiasSlopeFactor = 0.0f; // optional

        // Multisampling
        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo {};
        multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateCreateInfo.minSampleShading = 1.0f; // optional
        multisampleStateCreateInfo.pSampleMask = nullptr; // optional
        multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // optional
        multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // optional

        // Color blending
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState {};
        colorBlendAttachmentState.colorWriteMask = 
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachmentState.blendEnable = VK_FALSE;
        colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // optional
        colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // optional
        colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD; // optional
        colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // optional
        colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // optional
        colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD; // optional

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo {};
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY; // optional
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
        for(size_t i {0}; i < 4; i++)
        {
            colorBlendStateCreateInfo.blendConstants[i] = 0.0f; // optional
        }

        // Pipeline layout
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 0; // optional
        pipelineLayoutCreateInfo.pSetLayouts = nullptr; // optional
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0; // optional
        pipelineLayoutCreateInfo.pPushConstantRanges = nullptr; // optional

        VkResult result = vkCreatePipelineLayout(vkDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout.");
        }

        // Create the graphics pipeline

        VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo {};
        graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineCreateInfo.stageCount = 2;
        graphicsPipelineCreateInfo.pStages = shaderStageCreateInfos;
        graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        graphicsPipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
        graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
        graphicsPipelineCreateInfo.pDepthStencilState = nullptr; // optional
        graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
        graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        graphicsPipelineCreateInfo.layout = pipelineLayout;
        graphicsPipelineCreateInfo.renderPass = renderPass;
        graphicsPipelineCreateInfo.subpass = 0;
        graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // optional
        graphicsPipelineCreateInfo.basePipelineIndex = -1; // optional

        result = vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create graphics pipeline.");
        }
        
        // cleanup
        vkDestroyShaderModule(vkDevice, vertShaderModule, nullptr);
        vkDestroyShaderModule(vkDevice, fragShaderModule, nullptr);
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
        // Destroy the graphics pipeline
        vkDestroyPipeline(vkDevice, pipeline, nullptr);

        // Destroy the pipeline layout
        vkDestroyPipelineLayout(vkDevice, pipelineLayout, nullptr);

        // Destroy the render pass
        vkDestroyRenderPass(vkDevice, renderPass, nullptr);

        // Destroy the swap chain image views
        for(VkImageView& imageView : swapChainImageViews)
        {
            vkDestroyImageView(vkDevice, imageView, nullptr);
        }

        // Don't need to cleanup the swap chain images

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
