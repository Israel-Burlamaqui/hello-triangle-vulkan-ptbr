#include "hello_triangle_application.hpp"
#include "load_shader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <stdexcept>
#include <set>
#include <limits>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <chrono>

VkVertexInputBindingDescription Vertex::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    /*
        Todos os nossos dados por vértice estão compactados em um único array, então 
        teremos apenas uma vinculação. O parâmetro `binding` especifica o índice da 
        vinculação no array de vinculações. O parâmetro `stride` especifica o número 
        de bytes de uma entrada para a próxima, e o parâmetro `inputRate` pode ter um 
        dos seguintes valores:

        -`VK_VERTEX_INPUT_RATE_VERTEX`: Move para a próxima entrada de dados após 
            cada vértice.

        -`VK_VERTEX_INPUT_RATE_INSTANCE`: Move para a próxima entrada de dados após 
        cada instância.

        Como não usaremos renderização instanciada, então manteremos os dados por vértice.
    */
    
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    /*
        Como o protótipo da função indica, haverá duas dessas estruturas. Uma estrutura 
        de descrição de atributo descreve como extrair um atributo de vértice de um bloco 
        de dados de vértice originado de uma descrição de vinculação. Temos dois atributos, 
        posição e cor, então precisamos de duas estruturas de descrição de atributo.
    */

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);


    return attributeDescriptions;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) 
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
        {
            indices.graphicsFamily = i;
        }
    }

    return indices;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
        const VkAllocationCallbacks* pAllocator, 
        VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) 
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } 
    else 
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
        VkDebugUtilsMessengerEXT debugMessenger, 
        const VkAllocationCallbacks* pAllocator) 
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) 
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void HelloTriangleApplication::run() 
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void HelloTriangleApplication::initWindow()
{
    std::cout << "Iniciando GLFW..." << std::endl;

    if (!glfwInit()) 
    {
        throw std::runtime_error("Falha ao inicializar GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    if (!window) 
    {
        const char* description = nullptr;
        int code = glfwGetError(&description);
        throw std::runtime_error(
        std::string("Falha ao criar janela GLFW: ") +
        (description ? description : "erro desconhecido"));
    }
}

void HelloTriangleApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void HelloTriangleApplication::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}

void HelloTriangleApplication::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport()) 
    {
        throw std::runtime_error("Camadas de validação requisitadas não estão disponíveis!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    if (enableValidationLayers) 
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } 
    else 
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    /* Para checar se a instância (ou outra struct do Vulkan)
       foi criada com sucesso, não precisamos armazenar o resultado,
       mas sim checar o valor de retorno 
    */
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) 
    {
        throw std::runtime_error("Falha ao criar instância Vulkan");
    }
}

bool HelloTriangleApplication::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) 
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) 
        {
            if (strcmp(layerName, layerProperties.layerName) == 0) 
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) 
        {
            std::cout << "Camada de validação requisitada não encontrada: " << layerName << std::endl;
            return false;
        }
    }

    std::cout << "Camadas de validação requisitadas estão disponíveis:" << std::endl;
    for (const char* layerName : validationLayers) 
    {
        std::cout << "  - " << layerName << std::endl;
    }
    return true;
}

std::vector<const char*> HelloTriangleApplication::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) 
    {
        /* Usar a macro VK_EXT_DEBUG_UTILS_EXTENSION_NAME evita que
           o nome da extensão precise ser digitado manualmente
        */
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) 
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void HelloTriangleApplication::setupDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) 
    {
        throw std::runtime_error("Falha ao configurar o debug messenger!");
    }

}

void HelloTriangleApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    
    // Configurações do debug messenger, sobre a severidade das mensagens que queremos receber
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    
    // Configurações do debug messenger, sobre o tipo de mensagem que queremos receber
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    
    createInfo.pfnUserCallback = debugCallback;
}

void HelloTriangleApplication::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) 
    {
        throw std::runtime_error("Falha ao encontrar GPUs com suporte a Vulkan!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) 
    {
        if (isDeviceSuitable(device)) 
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) 
    {
        throw std::runtime_error("Falha ao encontrar um dispositivo físico adequado!");
    }
}

bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device)
{
    /*
        O que fazer quando uma família não estiver disponível? Poderíamos
        lançar uma exceção, mas esse não é o melhor método para decidir se
        um dispositivo é adequado ou não. Em vez disso, podemos preferir
        dispositivos que tenham todas as famílias de filas necessárias, mas
        não exigir que todas as famílias estejam presentes. Isso nos permite 
        usar dispositivos que não possuem todas as famílias de filas, mas ainda 
        podem ser adequados para o nosso aplicativo.
    */
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);
    bool swapChainAdequate = false;

    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && 
                            !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 
                                            queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) 
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport)
        {
            indices.presentFamily = i;
        }
    }

    return indices;
}

void HelloTriangleApplication::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), 
                                            indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily: uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    /*
        A maioria dos drivers gráficos recentes permitirão apenas um número
        pequeno de filas para cada Queue Family e você não precisa de mais 
        de uma. Isso porque você pode criar todos os command buffers em 
        múltiplas threads e depois submetê-las na thread principal numa única
        chamada de baixa sobrecarga.
    */

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("Falaha ao criar o dispositivo lógico!");
    }

    // Como estamos criando apenas uma fila para cada família, especificamos
    // o índice 0.
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void HelloTriangleApplication::createSurface()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao criar a Window Surface!");
    }
}

bool HelloTriangleApplication::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;

    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, 
                                        nullptr);

    std::vector<VkExtensionProperties> avaiableExtensions(extensionCount);
    
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, 
                                        avaiableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), 
                                            deviceExtensions.end());

    for (const auto& extension : avaiableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;
    uint32_t formatCount;
    uint32_t presentModeCount;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, 
                                                    details.formats.data());
    }

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, 
                                                details.presentModes.data());
    }



    return details;
}

VkSurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    /*
        Formatos da Swap Chain

        Cada registro VkSurfaceFormatKHR contém um membro 'format' e um 'colorSpace'. O
        primeiro especifica os canais de cor e os tipos, como por exemplo, o formato 
        BGRA8 solicitado abaixo - cada canal de cor usa 8 unsigned bits, dando um total de 32
        bits por pixel. O segundo indica se o espaço de cor SRGB (dentre outros espaços de cor)
        está presente ou não. 

        Em versões antigas da API Vulkan, era usado VK_COLORSPACE_SRGB_NONLINEAR_KHR ao invés da
        macro atual VK_COLOR_SPACE_SRGB_NONLINEAR_KHR.
    */

    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    /*
        Modos de apresentação da Swap Chain

        Essa é a configuração mais importante para a swap chain, porque ela representa as 
        condições para exibir as imagens na tela. Existem quatro modos possíveis no Vulkan:

        - VK_PRESENT_MODE_IMMEDIATE_KHR: O famoso "V-sync off". Produz tearing;

        - VK_PRESENT_MODE_FIFO_KHR: O aplicativo Vulkan espera um determinado tempo para 
                                    pegar uma das imagens da swap chain - que é uma fila -
                                    para exibir na tela. A fila pode conter mais de uma
                                    imagem, e é o único modo de apresentação garantido em 
                                    todas as GPUs Vulkan;
                                    
        - VK_PRESENT_MODE_FIFO_RELAXED: Uma variação do segundo modo, mas difere em uma coisa:
                                    Se o tempo de espera do próximo quadro acabou, e a fila da
                                    swap chain estiver completamente vazia, ele apresenta o 
                                    próximo quadro no mesmo instante que ele for concluído pela
                                    GPU, introduzindo tearing visível;

        - VK_PRESENT_MODE_MAILBOX_KHR: Outra variação do segundo, mas a fila da swap chain só 
                                    possui uma imagem - que não é a exibida atualmente. Esse é 
                                    o único modo que reduz drasticamente a latência introduzida 
                                    pelo V-sync, mas aumenta o consumo de recursos, e não é reco-
                                    mendada para dispositivos móveis por causa do thermal throtling.
    */
    
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }
    
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    } else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = 
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, 
            capabilities.minImageExtent.width, capabilities.maxImageExtent.width);

        actualExtent.height = std::clamp(actualExtent.height, 
            capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        
        return actualExtent;
    }
}

void HelloTriangleApplication::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && 
        imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), 
                                    indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    
    // 'compositeAlpha' pergunta se você quer que a janela seja transparente.
    // Na maioria das vezes, nós precisaremos de uma janela opaca. 
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao criar a swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void HelloTriangleApplication::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat);
    }
}

void HelloTriangleApplication::createGraphicsPipeline()
{
    /*
        O Vulkan não possui uma linguagem de shader em alto nível como GLSL ou o HLSL.
        Em vez disso, ele utiliza um bytecode chamado SPIR-V para compilar shaders e, por 
        causa disso, ele não se destina a ser legível para humanos. 

        A escolha entre um bytecode universal e uma linguagem de shader de alto nível veio
        de um problema antigo do OpenGL: dado a complexidade dos drivers das marcas de GPU,
        se você quisesse escrever shaders não-triviais (ou exóticos, como nos queridos emula-
        dores!) em GLSL, poderia acontecer de que ele funcionasse corretamente em uma GPU de
        uma empresa específica, mas quebrasse totalmente em outras por erros de sintaxe do
        compilador ou desenhasse uma cena 3d incorretamente por bugs do driver.

        Como o SPIR-V é um bytecode, a escrita de compiladores e drivers fica bem menos complexa,
        evitando (pelo menos!) esses bugs de drivers...
    */

    auto vertShaderCode = readFile("src/shaders/spirv/triangle_vert.spv");
    auto fragShaderCode = readFile("src/shaders/spirv/triangle_frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    /*
        Para usar os shaders propriamente ditos, precisamos associá-los a um estágio específico
        do pipeline através da estrutura 'VkPipelineShaderStageCreateInfo' como parte da criação
        do Graphics Pipeline.
    */

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    /*
        Estágios de Função Fixa:

        -Dynamic States;
        -Vertex Input;
        -Input Assembly;
        -Viewport e Scissors;
        -Rasterizador;
        -Multiamostragem;
        -Depth e teste de estêncil;
        -Color blending;
        -Pipeline Layout;

    */

    // Dynamic States
    std::vector<VkDynamicState> dynamicStates = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Vertex Input 
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = 
        static_cast<uint32_t>(attributeDescriptions.size());

    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport e Scissors
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.pScissors = &scissor;

    // Rasterizador
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Multiamostragem
    /*
        Geralmente usado para fazer anti-aliasing, amostrando vários resultados dos
        fragmentos de muitos polígonos que rasterizam no mesmo pixel.
    */
   VkPipelineMultisampleStateCreateInfo multisampling{};
   multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   multisampling.sampleShadingEnable = VK_FALSE;
   multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
   multisampling.minSampleShading = 1.0f;
   multisampling.pSampleMask = nullptr;
   multisampling.alphaToCoverageEnable = VK_FALSE;
   multisampling.alphaToOneEnable = VK_FALSE;

   // Depth e teste de estêncil

    // Nada por enquanto (aula do Hello Triangle)

   // Color blending
   VkPipelineColorBlendAttachmentState colorBlendAttachment;
   colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                         VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT |
                                         VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, 
                                nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao criar o Pipeline Layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao criar o graphics pipeline!");
    }
    
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void HelloTriangleApplication::createFramebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());
    
    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        VkImageView attachments[] =
        {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Falha ao criar o framebuffer!");
        }
    }
}

VkShaderModule HelloTriangleApplication::createShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao criar o shader module!");
    }

    return shaderModule;
}

void HelloTriangleApplication::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    /*
        Um único render pass pode consistir de múltiplos subpasses. Subpasses são operações
        de renderização sequenciais que dependem do conteúdo dos passes anteriores dos
        framebuffers, como por exemplo uma sequência de pós processamento que são aplicados
        um após o outro.

        Se você agrupar essas operações de renderização num único render pass, então o Vulkan 
        poderá reordenar essas operações e economizar largura de banda da memória e possibilitar
        um desempenho melhor. Para o nosso primeito triângulo, porém, usaremos um único
        subpass.

        Cada subpass referencia um ou mais dos attachments (anexos?) que descrevemos usando a
        estrutura nas sessões anteriores. Essas referências são estruturas 'VkAttachmentReference'
        que se parecem com isto:

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    */

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao criar o render pass!");
    }
}

void HelloTriangleApplication::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    /*
        Flags dos Command Pools    

        Há duas flags possíveis para os command pools:

        -VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Avisa que os command buffers
                                são regravados com novos comandos diversas
                                vezes, afetando o comportamento da alocação 
                                de memória.

        -VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Permite que command
                                buffers sejam regravados individualmente. Sem
                                essa flag, todos os command buffers são rese-
                                tados juntos.

    */

    /*
        Command buffers são executados ao serem submetidos em uma das filas do
        dispositivo lógico, como nas filas de gráficos e apresentação que solici-
        tamos. Cada command pool só pode alocar command buffers que são submetidos
        em um único tipo de fila. Estamos indo gravar comandos de desenho, e esse 
        é o motivo de termos escolhido a famíla de fila gráfica.
    */

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao criar o command pool!");
    }
}

void HelloTriangleApplication::createCommandBuffers()
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    /*
        O parâmetro 'level' especifica se os command buffers alocados são do
        tipo primário ou secundário:

        -VK_COMMAND_BUFFER_LEVEL_PRIMARY: Podem ser submetidas para uma fila para
                                execução, mas não pode ser chamada a partir de 
                                outros command buffers.
        
        -VK_COMMAND_BUFFER_LEVEL_SECONDARY: Não pode ser submetida diretamente, mas 
                                pode ser chamada por command buffers primários.

        Não iremos usar command buffers secundários aqui, entretando, você pode 
        imaginar que isso pode ser útil para reutilizar operações comuns de command
        buffers primários.
    */

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao alocar os command buffers!!!");
    }

}

void HelloTriangleApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo =  nullptr;

    /*
        O parâmetro `flags` especifica como utilizaremos o buffer de comandos. Os 
        seguintes valores estão disponíveis:

        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: O buffer de comandos será re-
                                gravado logo após ser executado uma única vez. 

        VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: Trata-se de um buffer de 
                                comandos secundário que estará inteiramente contido 
                                em um único render pass. 

        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: O buffer de comandos pode ser 
                                submetido novamente enquanto ainda estiver pendente 
                                de execução.

        Nenhuma dessas flags é aplicável ao nosso caso no momento.
    */

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao começar a gravar o command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    /*
        O segundo parâmetro abaixo especifica se o objeto de pipeline é do tipo 
        'graphics' ou 'compute'. Nós estamos dizendo ao Vulkan que operações exe-
        cutar no Graphics Pipeline e quais attachments (anexos) usar no fragment shader.
    */

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    /*
        O uso de um buffer de índices para o desenho envolve duas alterações no 
        `recordCommandBuffer`. Primeiramente, precisamos vincular o buffer de índices, 
        assim como fizemos com o buffer de vértices. A diferença é que só é possível 
        ter um único buffer de índices. Infelizmente, não é possível usar índices 
        diferentes para cada atributo de vértice; portanto, ainda precisamos duplicar 
        completamente os dados dos vértices, mesmo que apenas um atributo varie.
    */
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    /*
        Um buffer de índice é vinculado com a função `vkCmdBindIndexBuffer`, que recebe 
        como parâmetros o buffer de índice, um deslocamento em bytes dentro dele e o tipo 
        dos dados de índice. Conforme mencionado anteriormente, os tipos possíveis são 
        `VK_INDEX_TYPE_UINT16` e `VK_INDEX_TYPE_UINT32`.

        A simples vinculação de um buffer de índice ainda não altera nada; também precisamos 
        alterar o comando de desenho para instruir o Vulkan a usar o buffer de índice. Remo- 
        veremos a linha vkCmdDraw e a substituiremos por vkCmdDrawIndexed:
    */
    // vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, 0, 1, &descriptorSets[currentFrame],
                            0, nullptr);
    /*
        Ao contrário dos buffers de vértices e de índices, os conjuntos de descritores não 
        são exclusivos dos pipelines gráficos. Portanto, precisamos especificar se queremos 
        vincular os conjuntos de descritores ao pipeline gráfico ou ao de computação. O próximo 
        parâmetro é o layout no qual os descritores se baseiam. Os três parâmetros seguintes 
        especificam o índice do primeiro conjunto de descritores, o número de conjuntos a serem 
        vinculados e a matriz dos conjuntos a serem vinculados. Os dois últimos parâmetros 
        especificam uma matriz de deslocamentos usados para descritores 
        dinâmicos.
    */

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    /*
        A chamada a essa função é muito semelhante à vkCmdDraw. Os dois primeiros parâmetros 
        especificam o número de índices e o número de instâncias. Como não estamos usando 
        instância, basta especificar 1 instância. 
        
        O número de índices representa o número de vértices que serão passados para o shader 
        de vértices. O próximo parâmetro especifica um deslocamento no buffer de índices; 
        usar o valor 1 faria com que a placa de vídeo começasse a ler a partir do segundo 
        índice. 
        
        O penúltimo parâmetro especifica um deslocamento a ser adicionado aos índices no buffer 
        de índices. O último parâmetro especifica um deslocamento para instância, que (novamente!) 
        não estamos utilizando.
    */

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao gravar o command buffer !");
    }
}

void HelloTriangleApplication::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);


    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i ++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, 
                        &imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, 
                        &renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, 
                        &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Falha ao criar os objetos de sincronização!");
        }   
    }
}

void HelloTriangleApplication::drawFrame()
{
    /*
        Visão Geral de um Frame

        Num alto nível, renderizar um frame em Vulkan consiste em uma
        configuração de passos comuns:

        1) Aguardar o frame anterior terminar;
        2) Adquirir uma imagem da Swap Chain;
        3) Gravar um command buffer que desenhe a cena nessa imagem;
        4) Submeter o command buffer gravado;
        5) Apresentar a imagem da Swap Chain.

        Embora venhamos a expandir a funcionalidade de desenho em 
        capítulos posteriores, por ora, este é o núcleo do nosso 
        loop de renderização.
    */

    /*
        Sincronização

        O núcleo da filosofia do Vulkan é que a sincronização da execução na GPU
        deve ser explícita. A definição da ordem de operações é de nossa 
        responsabilidade, e devemos usar várias primitivas de sincronização para
        avisar ao driver gráfico a ordem em que queremos que as coisas rodem.

        Isso significa que a maioria das chamadas na API Vulkan que começam sua
        execução na GPU são assíncronas, as funções vão retornar antes da operação 
        estiver concluída.

        Neste capítulo, há uma série de eventos que precisamos ordenar explicitamente, 
        uma vez que ocorrem na GPU, tais como:

        -Obter uma imagem da Swap Chain;
        -Executar comandos que desenham na imagem obtida;
        -Exibir essa imagem na tela, devolvendo-a à Swap Chain.

        Cada um desses eventos é iniciado por meio de uma única chamada de função, mas 
        todos são executados de forma assíncrona. As chamadas de função retornam antes 
        que as operações sejam, de fato, concluídas, e a ordem de execução também não é 
        definida. Isso é um problema, pois cada operação depende da conclusão da anterior. 
        Portanto, precisamos explorar quais primitivas podem ser utilizadas para garantir 
        a ordem desejada.

        Objetos de sincronização:

        1. VkSemaphore (Sincronização entre GPU e GPU / Filas)

        • O que faz: Ordena operações dentro da própria GPU ou entre diferentes filas (queues) 
        da GPU, sem a necessidade de envolver a CPU no meio do caminho.
        
        • Caso de uso típico: Garantir que uma imagem seja adquirida do swapchain antes de 
        começar a renderizar nela, ou que a renderização termine antes de a imagem ser 
        apresentada na tela.

        • Exemplo: Você submete um comando de desenho (draw call) e precisa que ele termine antes 
        de iniciar um pós-processamento. Você usa um semáforo para sinalizar: "só comece a etapa B 
        quando a GPU terminar a etapa A".

        2. VkFence (Sincronização entre GPU e CPU)

        • O que faz: Informa a CPU que a GPU terminou uma tarefa específica (geralmente a execução 
        de um frame inteiro ou de um vkQueueSubmit).

        • Caso de uso típico: Implementar o conceito de "frames in flight" (quadros em processamento). 
        A CPU precisa saber se a GPU já terminou de usar os buffers de um quadro anterior antes de 
        poder reutilizar ou sobrescrever a memória daquele quadro.

        • Importante: Ao contrário dos semáforos, as fences precisam ser reiniciadas manualmente pela 
        CPU (vkResetFences) quando voltam ao estado não sinalizado (unsignaled).
    */
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, 
        imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Falha ao adquirir a imagem da swap chain!!!");
    }

    vkResetFences(device, 1, &inFlightFences[currentFrame]);
    /*
        Os dois primeiros parâmetros de `vkAcquireNextImageKHR` são o dispositivo lógico e a Swap Chain 
        da qual desejamos adquirir uma imagem. O terceiro parâmetro especifica um tempo limite em nanossegundos 
        para que uma imagem fique disponível. Usar o valor máximo de um inteiro sem sinal de 64 bits significa 
        que efetivamente desativamos o tempo limite.

        Os dois parâmetros seguintes especificam objetos de sincronização que devem ser sinalizados quando 
        o mecanismo de apresentação terminar de usar a imagem. Esse é o momento em que podemos começar a 
        desenhar nela. É possível especificar um semáforo, uma cerca ou ambos. Usaremos nosso 
        `imageAvailableSemaphore` para essa finalidade aqui.

        O último parâmetro especifica uma variável para exibir o índice da imagem da cadeia de troca que 
        ficou disponível. O índice se refere ao `VkImage` em nossa matriz `swapChainImages`. Usaremos esse 
        índice para selecionar o `VkFrameBuffer`.
    */

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    updateUniformBuffer(currentFrame);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    
    VkPipelineStageFlags waitStages[VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao submeter o command buffer de desenho!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    
    /*
        É importante fazer isso após `vkQueuePresentKHR` para garantir que os semáforos estejam em 
        um estado consistente; caso contrário, um semáforo sinalizado pode nunca ser aguardado 
        corretamente.
    */
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    } else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao apresentar a imagem da swap chain!!!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void HelloTriangleApplication::recreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(device);
    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createFramebuffers();
}

void HelloTriangleApplication::cleanupSwapChain()
{
    for (auto framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void HelloTriangleApplication::createVertexBuffer()
{
      VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    /*
        Agora estamos usando um novo `stagingBuffer` com `stagingBufferMemory` para mapear 
        e copiar os dados dos vértices. Então, vamos usar dois novos sinalizadores 
        de uso de buffer:

        -VK_BUFFER_USAGE_TRANSFER_SRC_BIT: O buffer pode ser usado como fonte em uma operação 
                                           de transferência de memória.

        -VK_BUFFER_USAGE_TRANSFER_DST_BIT: O buffer pode ser usado como destino em uma operação 
                                           de transferência de memória.

        O `vertexBuffer` agora é alocado a partir de um tipo de memória local ao dispositivo, 
        o que geralmente significa que não podemos usar `vkMapMemory`. No entanto, podemos 
        copiar dados do `stagingBuffer` para o `vertexBuffer`. Precisamos indicar essa intenção 
        especificando o sinalizador de origem de transferência para o `stagingBuffer` e o 
        sinalizador de destino de transferência para o `vertexBuffer`, juntamente com o 
        sinalizador de uso do buffer de vértices.
    */
}

uint32_t HelloTriangleApplication::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Não foi encontrado um tipo de memória adequado!");
}

void HelloTriangleApplication::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                    VkMemoryPropertyFlags properties, VkBuffer& buffer, 
                    VkDeviceMemory& bufferMemory)
{
    /*
        - Introdução

        O buffer de vértices que temos no momento funciona corretamente, mas o 
        tipo de memória que nos permite acessá-lo a partir da CPU pode não ser 
        o tipo de memória mais adequado para a própria placa de vídeo ler. 
        
        A memória mais adequada possui o sinalizador VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT 
        e, geralmente, não é acessível pela CPU em placas de vídeo dedicadas. Neste 
        capítulo, vamos criar dois buffers de vértices. Um buffer intermediário na 
        memória acessível pela CPU, para o qual os dados da matriz de vértices 
        serão transferidos, e o buffer de vértices final na memória local do 
        dispositivo. 
        
        Em seguida, usaremos um comando de cópia de buffer para mover os 
        dados do buffer intermediário para o buffer de vértices propriamente dito.

        - Fila de transferência

        O comando de cópia de buffer requer uma família de filas que suporte operações 
        de transferência, o que é indicado por meio do VK_QUEUE_TRANSFER_BIT. A boa 
        notícia é que qualquer família de filas com os recursos VK_QUEUE_GRAPHICS_BIT ou 
        VK_QUEUE_COMPUTE_BIT já suporta implicitamente as operações VK_QUEUE_TRANSFER_BIT. 
        Nesses casos, a implementação não precisa listá-lo explicitamente em `queueFlags`.
    */

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao criar o buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao alocar a memória do buffer!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void HelloTriangleApplication::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void HelloTriangleApplication::createIndexBuffer()
{
    /*
        As malhas 3D que você renderizará em uma aplicação do mundo real costumam compartilhar 
        vértices entre vários triângulos. Isso já acontece mesmo em algo simples como desenhar 
        um retângulo.

        Para desenhar um retângulo, são necessários dois triângulos, o que significa que precisamos 
        de um buffer de vértices com 6 vértices. O problema é que os dados de dois vértices precisam 
        ser duplicados, resultando em 50% de redundância. A situação só piora com malhas mais complexas, 
        nas quais os vértices são reutilizados em uma média de 3 triângulos. A solução para esse problema 
        é usar um buffer de índice.

        Um buffer de índices é, essencialmente, uma matriz de ponteiros para o buffer de vértices. Ele 
        permite reordenar os dados dos vértices e reutilizar dados existentes para vários vértices. Os três 
        primeiros índices definem o triângulo superior direito e os três últimos índices definem os vértices 
        do triângulo inferior esquerdo.

        O uso de um buffer de índices para o desenho envolve duas alterações no `recordCommandBuffer`. 
        Primeiramente, precisamos vincular o buffer de índices, assim como fizemos com o buffer de vértices. 
        A diferença é que só é possível ter um único buffer de índices. Infelizmente, não é possível usar 
        índices diferentes para cada atributo de vértice; portanto, ainda precisamos duplicar completamente 
        os dados dos vértices, mesmo que apenas um atributo varie.
    */


    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::createDescriptorSetLayout()
{
    /*
        Cada ligação precisa ser descrita por meio de uma estrutura 
        VkDescriptorSetLayoutBinding. 
    */
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    /*
        Os dois primeiros campos especificam a ligação utilizada no shader e o tipo 
        de descritor, que é um objeto de buffer uniforme. É possível que a variável 
        do shader represente uma matriz de objetos de buffer uniforme, e o descriptorCount 
        especifica o número de valores na matriz. Isso poderia ser usado para especificar 
        uma transformação para cada um dos ossos de um esqueleto, para animação esquelética, 
        por exemplo. Nossa transformação MVP está em um único objeto de buffer uniforme, 
        portanto, estamos usando um descriptorCount igual a 1.

        Também precisamos especificar em quais estágios do shader o descritor será referenciado. 
        O campo `stageFlags` pode ser uma combinação de valores `VkShaderStageFlagBits` ou o 
        valor `VK_SHADER_STAGE_ALL_GRAPHICS`. No nosso caso, estamos referenciando o descritor 
        apenas a partir do shader de vértices.

        O campo `pImmutableSamplers` é relevante apenas para descritores relacionados à 
        amostragem de imagens, que abordaremos mais adiante. Você pode deixar esse campo 
        com seu valor padrão.
    */

    /*
        Precisamos especificar o layout do conjunto de descritores durante a criação do 
        pipeline para informar ao Vulkan quais descritores os shaders utilizarão. Os layouts 
        dos conjuntos de descritores são especificados no objeto de layout do pipeline:
    */
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao criar o descriptor set layout!");
    }
}

void HelloTriangleApplication::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

        if (vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Falha ao mapear a memória do uniform buffer!");
        }
    }

    /*
        Mapeamos o buffer logo após sua criação usando vkMapMemory para obter um ponteiro 
        no qual possamos gravar os dados posteriormente. O buffer permanece mapeado a esse 
        ponteiro durante todo o ciclo de vida do aplicativo. Essa técnica é chamada de 
        "persistent mapping” e funciona em todas as implementações do Vulkan. 
        Não precisar mapear o buffer toda vez que precisamos atualizá-lo aumenta o desempenho, 
        já que o mapeamento tem um custo.
    */
}

void HelloTriangleApplication::updateUniformBuffer(uint32_t currentImage)
{
    static const auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height), 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void HelloTriangleApplication::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao criar o descriptor pool!");
    }
}

void HelloTriangleApplication::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao alocar os descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

void HelloTriangleApplication::createTextureImage()
{
    // Carrega os pixels e prepara VkBufferCreateInfo, VkImageCreateInfo e VkBufferImageCopy para a transferência.
    int texWidth;
    int texHeight;
    int texChannels;
    stbi_uc* pixels = stbi_load("src/textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels)
    {
        throw std::runtime_error("Falha ao carregar a imagem da textura!");
    }

    VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * static_cast<VkDeviceSize>(texHeight) * 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    if (vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data) != VK_SUCCESS)
    {
        stbi_image_free(pixels);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
        throw std::runtime_error("Falha ao mapear a memória do staging buffer da textura!");
    }

    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);
    stbi_image_free(pixels);

    createImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),
                VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth),
                      static_cast<uint32_t>(texHeight));
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::createImage(uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory)
{
    // Preenche VkImageCreateInfo e VkMemoryAllocateInfo para criar a imagem e alocar sua memória.
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao criar a imagem!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        vkDestroyImage(device, image, nullptr);
        throw std::runtime_error("Falha ao alocar memória para a imagem!");
    }

    if (vkBindImageMemory(device, image, imageMemory, 0) != VK_SUCCESS)
    {
        vkFreeMemory(device, imageMemory, nullptr);
        vkDestroyImage(device, image, nullptr);
        throw std::runtime_error("Falha ao vincular a memória à imagem!");
    }
}

VkCommandBuffer HelloTriangleApplication::beginSingleTimeCommands()
{
    // Preenche VkCommandBufferAllocateInfo e VkCommandBufferBeginInfo para iniciar um comando de uso único.
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao alocar o command buffer!");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        throw std::runtime_error("Falha ao iniciar o command buffer!");
    }

    return commandBuffer;
}

void HelloTriangleApplication::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    // Preenche VkSubmitInfo para submeter o comando, aguardar a fila e liberar o command buffer.
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        throw std::runtime_error("Falha ao finalizar o command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        throw std::runtime_error("Falha ao submeter o command buffer!");
    }

    if (vkQueueWaitIdle(graphicsQueue) != VK_SUCCESS)
    {
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        throw std::runtime_error("Falha ao aguardar a fila gráfica!");
    }

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void HelloTriangleApplication::transitionImageLayout(VkImage image, VkFormat format, 
                            VkImageLayout oldLayout, VkImageLayout newLayout)
{
    // Preenche VkImageMemoryBarrier e as máscaras de acesso/estágio para a transição de layout.
    static_cast<void>(format);

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        throw std::invalid_argument("Transição de layout não suportada!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);

    endSingleTimeCommands(commandBuffer);
}

void HelloTriangleApplication::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    // Preenche VkBufferImageCopy para copiar os pixels do buffer à imagem em layout de destino.
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

void HelloTriangleApplication::createTextureImageView()
{
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

VkImageView HelloTriangleApplication::createImageView(VkImage image, VkFormat format)
{
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao criar a image view!");
    }

    return imageView;
}

void HelloTriangleApplication::createTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("Falha ao criar o texture sampler!");
    }
}

void HelloTriangleApplication::mainLoop()
{
    while (!glfwWindowShouldClose(window)) 
    {    
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(device);
}

void HelloTriangleApplication::cleanup()
{
    /*
        Precisamos desalocar todos os objetos Vulkan que criamos para evitar 
        memory leaks ou comportamentos indefinidos, já que ele foi projetado 
        para desempenho máximo em hardware moderno e não irá tolerar nenhum 
        erro SEU!
    */
    cleanupSwapChain();

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);

    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    for (size_t i = 0; i < uniformBuffers.size(); i++)
    {
        vkUnmapMemory(device, uniformBuffersMemory[i]);
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    vkDestroyRenderPass(device, renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) 
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);


    glfwDestroyWindow(window);
    glfwTerminate();
}