#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <array>

#include "gpu.h"
#include "transform.h"
#include "vertex.h"
#include "load_model.h"
#include "light.h"
#include "anti_alias.h"
#include "render_pass.h"
#include "sm_math.h"
#include "scene.h"
#include "pipeline.h"
#include "imgui.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

class Application {
public:
    void run() {
        initWindow();
        initVulkan();
        loadScene();
        initImGui();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    GPU gpu;

    MSAA* msaa;

    RenderPass* renderPass;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkDescriptorSetLayout descriptorSetLayout_0, descriptorSetLayout_1,
        descriptorSetLayout_2;
    
    Pipeline basic_graphic_pipeline, basic_t_graphic_pipeline, normal_mapping_pipeline;

    Scene* scene;

    std::vector<VkImage> textureImage;
    std::vector<VkImageView> textureImageView;
    VkDeviceMemory textureImageMemory;
    VkSampler textureSampler;

    std::vector<VkImage> normalMapImage;
    std::vector<VkImageView> normalMapImageView;
    VkDeviceMemory normalMapImageMemory;

    FragmentUniform fubo;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    bool framebufferResized = false;

    ImDrawData* imgui_draw_data;

    void processInput(GLFWwindow* window) {
        /*
        check for keyboard inputs and act acordingly
        */

        // Esc to exit the program
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // awsd to move around
        scene->camera.awsd_movement(window);

        // press n to change the debug index to the next
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE) {
            if (scene->debug_press_n) {
                scene->debug_press_n = false;
                scene->debug_index++;
                if (scene->debug_index == scene->debug_node_names.size())
                    scene->debug_index = 0;
                std::cout << scene->debug_node_names[scene->debug_index] << std::endl;
            }
        } else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) scene->debug_press_n = true;

        // press b to change the debug index back
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
            if (scene->debug_press_b) {
                scene->debug_press_b = false;
                scene->debug_index--;
                if (scene->debug_index == -1)
                    scene->debug_index = scene->debug_node_names.size() - 1;
                std::cout << scene->debug_node_names[scene->debug_index] << std::endl;
            }
        } else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) scene->debug_press_b = true;

        // press t to toggle debug mode
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
            if (scene->debug_press_t) {
                scene->debug_press_t = false;
                scene->debug_mode = !scene->debug_mode;
                std::cout << "debug mode: " << (scene->debug_mode ? "on" : "off") << std::endl;
            }
        } else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) scene->debug_press_t = true;
    }

    void initImGui() {
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = instance;
        init_info.PhysicalDevice = gpu.physical_gpu;
        init_info.Device = gpu.logical_gpu;
        QueueFamilyIndices indices(gpu.physical_gpu, surface);
        init_info.QueueFamily = indices.graphicsFamily.value();
        init_info.Queue = gpu.graphicsQueue;
        init_info.DescriptorPool = descriptorPool;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = swapChainImages.size();
        init_info.MSAASamples = msaa->getSampleCount();
        init_info.RenderPass = renderPass->getRenderPass();
        ImGui_ImplVulkan_Init(&init_info);
    }

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

        // hide and capture cursor
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // setup mouse callback to process mouse movements
        glfwSetCursorPosCallback(window, cursorPosCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        if (!app->scene->debug_mode) {
            app->scene->camera.mouse_callback(xpos, ypos);
        }
    }

    void create_graphic_pipelines() {
        createDescriptorSetLayout();

        std::vector<VkDescriptorSetLayout> setLayouts =
            { descriptorSetLayout_0, descriptorSetLayout_1, descriptorSetLayout_2 };

        // create the basic pipeline to render basic meshes
        basic_graphic_pipeline.create(&gpu, msaa, renderPass->getRenderPass(),
            "shaders/shader.vert.spv", "shaders/shader.frag.spv", Vertex::getBindingDescription(),
            Vertex::getAttributeDescriptions(), setLayouts);

        // create a graphic pipeline that takes vertex with tangent
        // and render it without normal mapping
        basic_t_graphic_pipeline.create(&gpu, msaa, renderPass->getRenderPass(),
            "shaders/shader_t.vert.spv", "shaders/shader.frag.spv", VertexWithTangent::getBindingDescription(),
            VertexWithTangent::getAttributeDescriptions(), setLayouts);

        // create normal mapping pipeline
        setLayouts[2] = descriptorSetLayout_1;
        setLayouts.push_back(descriptorSetLayout_2);
        normal_mapping_pipeline.create(&gpu, msaa, renderPass->getRenderPass(),
            "shaders/normal_mapping.vert.spv", "shaders/normal_mapping.frag.spv",
            VertexWithTangent::getBindingDescription(),
            VertexWithTangent::getAttributeDescriptions(), setLayouts);
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        gpu = GPU(instance, surface);
        createSwapChain();
        msaa = new MSAA(&gpu, swapChainImageFormat, swapChainExtent);
        renderPass = new RenderPass(gpu.logical_gpu, swapChainImageFormat, findDepthFormat(), msaa->getSampleCount());
        create_graphic_pipelines();
        createDepthResources();
        createFramebuffers();
        createTextureSampler();
        createCommandBuffers();
        createSyncObjects();
    }

    void loadScene() {

        scene = new Scene();
        load_meshes_and_textures_obj(
            scene,
            "3d_models/San_Miguel/san-miguel-low-poly.obj",
            "3d_models/San_Miguel/san-miguel-low-poly.mtl"
        );
        scene->debug_index = 0;
        scene->debug_press_n = false;
        scene->debug_press_b = false;
        scene->debug_press_t = false;
        scene->debug_mode = false;
        scene->enable_normal_map = false;
        scene->lights = light();
        scene->lights.load_file("config/all_lights.txt");
        fubo.lights = scene->lights;

        // create VkImage and VkImageView for textures
        createTextureImages();
        createTextureImageViews();

        // create VkImage and VkImageView for normal maps
        createNormalMapImages();

        scene->createVertexBuffer(&gpu);
        scene->createIndexBuffer(&gpu);
        scene->createUniformBuffer(&gpu);
        createDescriptorPool();
        createDescriptorSets();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            
            // process mouse and keyboard input
            if (!scene->debug_mode) {
                if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    scene->camera.set_first_mouse();
                }
                processInput(window);
            } else {
                if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }

            glfwPollEvents();

            // Start the Dear ImGui frame
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Simple UI
            if (scene->debug_mode) {
                ImGui::Begin("Options");
                ImGui::Checkbox("Debug mode", &scene->debug_mode);
                ImGui::Checkbox("Normal map", &scene->enable_normal_map);
                ImGui::End();
            }

            // Rendering
            ImGui::Render();
            imgui_draw_data = ImGui::GetDrawData();

            drawFrame();
        }

        vkDeviceWaitIdle(gpu.logical_gpu);
    }

    void cleanupSwapChain() {
        msaa->destroyColorResources();

        vkDestroyImageView(gpu.logical_gpu, depthImageView, nullptr);
        vkDestroyImage(gpu.logical_gpu, depthImage, nullptr);
        vkFreeMemory(gpu.logical_gpu, depthImageMemory, nullptr);

        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(gpu.logical_gpu, framebuffer, nullptr);
        }

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(gpu.logical_gpu, imageView, nullptr);
        }

        vkDestroySwapchainKHR(gpu.logical_gpu, swapChain, nullptr);
    }

    void cleanup() {

        // clean up imgui
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        cleanupSwapChain();

        vkDestroySampler(gpu.logical_gpu, textureSampler, nullptr);

        // destroy the VkImage and VkImageView
        for (int i = 0; i < scene->textures.size(); i++) {
            vkDestroyImageView(gpu.logical_gpu, textureImageView[i], nullptr);
            vkDestroyImage(gpu.logical_gpu, textureImage[i], nullptr);
        }
        for (int i = 0; i < scene->normal_maps.size(); i++) {
            vkDestroyImageView(gpu.logical_gpu, normalMapImageView[i], nullptr);
            vkDestroyImage(gpu.logical_gpu, normalMapImage[i], nullptr);
        }
        
        vkFreeMemory(gpu.logical_gpu, textureImageMemory, nullptr);
        vkFreeMemory(gpu.logical_gpu, normalMapImageMemory, nullptr);

        delete msaa;

        delete scene;

        vkDestroyDescriptorPool(gpu.logical_gpu, descriptorPool, nullptr);

        vkDestroyDescriptorSetLayout(gpu.logical_gpu, descriptorSetLayout_0, nullptr);
        vkDestroyDescriptorSetLayout(gpu.logical_gpu, descriptorSetLayout_1, nullptr);
        vkDestroyDescriptorSetLayout(gpu.logical_gpu, descriptorSetLayout_2, nullptr);

        vkDestroyPipeline(gpu.logical_gpu, basic_graphic_pipeline.pipeline, nullptr);
        vkDestroyPipelineLayout(gpu.logical_gpu, basic_graphic_pipeline.layout, nullptr);
        vkDestroyPipeline(gpu.logical_gpu, basic_t_graphic_pipeline.pipeline, nullptr);
        vkDestroyPipelineLayout(gpu.logical_gpu, basic_t_graphic_pipeline.layout, nullptr);
        vkDestroyPipeline(gpu.logical_gpu, normal_mapping_pipeline.pipeline, nullptr);
        vkDestroyPipelineLayout(gpu.logical_gpu, normal_mapping_pipeline.layout, nullptr);

        delete renderPass;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(gpu.logical_gpu, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(gpu.logical_gpu, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(gpu.logical_gpu, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(gpu.logical_gpu, gpu.commandPool, nullptr);

        vkDestroyDevice(gpu.logical_gpu, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void recreateSwapChain() {
        int width = 0, height = 0;
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(gpu.logical_gpu);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        msaa->createColorResources(swapChainImageFormat, swapChainExtent);
        createDepthResources();
        createFramebuffers();
    }

    void calculate_offsets(
        std::vector<VkDeviceSize>& offsets,
        std::vector<VkMemoryRequirements>& requirements
    ) {
        /*
        Calculate an array of offsets for images so that the alignment is correct
        */

        // sort the indices of the image in the order of decreasing alignment
        std::vector<int> sorted_indices;
        std::vector<VkDeviceSize> alignments;
        alignments.resize(requirements.size());
        for (int i = 0; i < requirements.size(); i++) {
            alignments[i] = requirements[i].alignment;
        }
        argsort(alignments, sorted_indices);

        // calculate the offsets
        std::vector<VkDeviceSize> temp_offsets;
        temp_offsets.resize(requirements.size());
        temp_offsets[0] = 0;
        for (int i = 1; i < requirements.size(); i++) {
            temp_offsets[i] = temp_offsets[i - 1] + requirements[sorted_indices[i - 1]].size;
        }

        // put the offsets into the correct order
        offsets.resize(requirements.size());
        for (int i = 0; i < requirements.size(); i++) {
            offsets[sorted_indices[i]] = temp_offsets[i];
        }
    }

    void createTextureImages() {
        /*
        Load images from files and create texture images
        */

        // image size
        VkDeviceSize totalImageSize = 0;
        std::vector<VkDeviceSize> imageSize;
        imageSize.resize(scene->textures.size());
        std::vector<int> texWidth;
        texWidth.resize(scene->textures.size());
        std::vector<int> texHeight;
        texHeight.resize(scene->textures.size());

        // image data
        std::vector<stbi_uc*> pixels;
        pixels.resize(scene->textures.size());

        // load images from files
        for (int i = 0; i < scene->textures.size(); i++) {
            
            // load the image using stb library
            int texChannels;
            pixels[i] = stbi_load(scene->textures[i].file_path.c_str(),
                &texWidth[i], &texHeight[i], &texChannels, STBI_rgb_alpha);
            if (!pixels[i]) throw std::runtime_error("failed to load texture image!");

            // calculate image size
            imageSize[i] = texWidth[i] * texHeight[i] * 4;
            totalImageSize += imageSize[i];
        }

        // create staging buffer
        Buffer staging_buffer(&gpu, totalImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // map the staging buffer memory on the GPU to a memory block on the RAM
        void* data;
        vkMapMemory(gpu.logical_gpu, staging_buffer.memory, 0, totalImageSize, 0, &data);

        // copy the image data to the mapped memory
        int offset = 0;
        for (int i = 0; i < scene->textures.size(); i++) {
            
            // flip the memory layout vertically so that it can map correctly
            for (int j = 0; j < texHeight[i]; j++) {
                memcpy((char*)data + offset + j * texWidth[i] * 4, pixels[i] + (texHeight[i] - 1 - j) * texWidth[i] * 4, texWidth[i] * 4);
            }

            // free the loaded image data
            stbi_image_free(pixels[i]);

            // update the offset
            offset += imageSize[i];
        }

        // After copy is complete, unmap the GPU memory
        vkUnmapMemory(gpu.logical_gpu, staging_buffer.memory);

        // create the VkImages and get memory requirements
        textureImage.resize(scene->textures.size());
        std::vector<VkMemoryRequirements> memRequirements;
        memRequirements.resize(scene->textures.size());
        VkDeviceSize totalRequiredSize = 0;
        uint32_t typeFilter = UINT32_MAX;
        for (int i = 0; i < scene->textures.size(); i++) {
            gpu.createImage(static_cast<uint32_t>(texWidth[i]), static_cast<uint32_t>(texHeight[i]), VK_SAMPLE_COUNT_1_BIT,
                VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, textureImage[i]);
            vkGetImageMemoryRequirements(gpu.logical_gpu, textureImage[i], &memRequirements[i]);
            totalRequiredSize += memRequirements[i].size;
            typeFilter &= memRequirements[i].memoryTypeBits;
        }

        // allocate memory
        gpu.allocateMemory(totalRequiredSize,
            gpu.findMemoryType(typeFilter, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
            textureImageMemory);

        // reorder the offsets
        std::vector<VkDeviceSize> imageOffset;
        calculate_offsets(imageOffset, memRequirements);

        // bind the VkImage to the memory and copy data from the staging buffer to the VkImage
        VkDeviceSize bufferOffset = 0;
        for (int i = 0; i < scene->textures.size(); i++) {
            
            // bind the VkImage
            vkBindImageMemory(gpu.logical_gpu, textureImage[i], textureImageMemory, imageOffset[i]);

            // copy data
            transitionImageLayout(textureImage[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copyBufferToImage(staging_buffer.buffer, bufferOffset, textureImage[i],
                static_cast<uint32_t>(texWidth[i]), static_cast<uint32_t>(texHeight[i]));
            transitionImageLayout(textureImage[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            // update the offset
            bufferOffset += imageSize[i];
        }
    }

    void createTextureImageViews() {
        textureImageView.resize(scene->textures.size());
        for (int i = 0; i < scene->textures.size(); i++) {
            textureImageView[i] = gpu.createImageView(textureImage[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    void createNormalMapImages() {
        /*
        Load images from files and create normal map images
        */

        // select the format of the images
        VkFormat format = findSupportedFormat(
            { VK_FORMAT_R8G8B8_SRGB, VK_FORMAT_R8G8B8A8_SRGB },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT
        );
        int num_channels;
        if (format == VK_FORMAT_R8G8B8_SRGB) num_channels = 3;
        else num_channels = 4;

        // image size
        VkDeviceSize totalImageSize = 0;
        std::vector<VkDeviceSize> imageSize;
        imageSize.resize(scene->normal_maps.size());
        std::vector<int> texWidth;
        texWidth.resize(scene->normal_maps.size());
        std::vector<int> texHeight;
        texHeight.resize(scene->normal_maps.size());

        // image data
        std::vector<stbi_uc*> pixels;
        pixels.resize(scene->normal_maps.size());

        // load images from files
        for (int i = 0; i < scene->normal_maps.size(); i++) {

            // load the image using stb library
            int texChannels;
            pixels[i] = stbi_load(scene->normal_maps[i].file_path.c_str(),
                &texWidth[i], &texHeight[i], &texChannels, num_channels);
            if (!pixels[i]) throw std::runtime_error("failed to load texture image!");

            // calculate image size
            imageSize[i] = texWidth[i] * texHeight[i] * num_channels;
            totalImageSize += imageSize[i];

        }

        // create staging buffer
        Buffer staging_buffer(&gpu, totalImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // map the staging buffer memory on the GPU to a memory block on the RAM
        void* data;
        vkMapMemory(gpu.logical_gpu, staging_buffer.memory, 0, totalImageSize, 0, &data);

        // copy the image data to the mapped memory
        int offset = 0;
        for (int i = 0; i < scene->normal_maps.size(); i++) {

            // flip the memory layout vertically so that it can map correctly
            for (int j = 0; j < texHeight[i]; j++) {
                memcpy(
                    (char*)data + offset + j * texWidth[i] * num_channels,
                    pixels[i] + (texHeight[i] - 1 - j) * texWidth[i] * num_channels,
                    texWidth[i] * num_channels
                );
            }

            // free the loaded image data
            stbi_image_free(pixels[i]);

            // update the offset
            offset += imageSize[i];
        }

        // After copy is complete, unmap the GPU memory
        vkUnmapMemory(gpu.logical_gpu, staging_buffer.memory);

        // create the VkImages and get memory requirements
        normalMapImage.resize(scene->normal_maps.size());
        std::vector<VkMemoryRequirements> memRequirements;
        memRequirements.resize(scene->normal_maps.size());
        VkDeviceSize totalRequiredSize = 0;
        uint32_t typeFilter = UINT32_MAX;
        for (int i = 0; i < scene->normal_maps.size(); i++) {
            gpu.createImage(static_cast<uint32_t>(texWidth[i]), static_cast<uint32_t>(texHeight[i]), VK_SAMPLE_COUNT_1_BIT,
                format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, normalMapImage[i]);
            vkGetImageMemoryRequirements(gpu.logical_gpu, normalMapImage[i], &memRequirements[i]);
            totalRequiredSize += memRequirements[i].size;
            typeFilter &= memRequirements[i].memoryTypeBits;
        }

        // allocate memory
        gpu.allocateMemory(totalRequiredSize,
            gpu.findMemoryType(typeFilter, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
            normalMapImageMemory
        );

        // reorder the offsets
        std::vector<VkDeviceSize> imageOffset;
        calculate_offsets(imageOffset, memRequirements);

        // bind the VkImage to the memory and copy data from the staging buffer to the VkImage
        VkDeviceSize bufferOffset = 0;
        for (int i = 0; i < scene->normal_maps.size(); i++) {

            // bind the VkImage
            vkBindImageMemory(gpu.logical_gpu, normalMapImage[i], normalMapImageMemory, imageOffset[i]);

            // copy data
            transitionImageLayout(
                normalMapImage[i],
                format,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            );
            copyBufferToImage(staging_buffer.buffer, bufferOffset, normalMapImage[i],
                static_cast<uint32_t>(texWidth[i]), static_cast<uint32_t>(texHeight[i]));
            transitionImageLayout(
                normalMapImage[i],
                format,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );

            // update the offset
            bufferOffset += imageSize[i];
        }

        createNormalMapImageViews(format);
    }

    void createNormalMapImageViews(VkFormat format) {
        normalMapImageView.resize(scene->normal_maps.size());
        for (int i = 0; i < scene->normal_maps.size(); i++) {
            normalMapImageView[i] = gpu.createImageView(normalMapImage[i], format, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    void createTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 4.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        if (vkCreateSampler(gpu.logical_gpu, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport(gpu.physical_gpu, surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
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

        // TODO: optimize QueueFamilyIndices
        QueueFamilyIndices indices(gpu.physical_gpu, surface);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(gpu.logical_gpu, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(gpu.logical_gpu, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(gpu.logical_gpu, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;

        createImageViews();
    }

    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews[i] = gpu.createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    void createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding vertex_uniform_binding{};
        vertex_uniform_binding.binding = 0;
        vertex_uniform_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vertex_uniform_binding.descriptorCount = 1;
        vertex_uniform_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding fragment_uniform_binding{};
        fragment_uniform_binding.binding = 1;
        fragment_uniform_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        fragment_uniform_binding.descriptorCount = 1;
        fragment_uniform_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 0;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings_0 = {vertex_uniform_binding, fragment_uniform_binding};
        std::array<VkDescriptorSetLayoutBinding, 1> bindings_1 = {samplerLayoutBinding};
        std::array<VkDescriptorSetLayoutBinding, 1> bindings_2 = {vertex_uniform_binding};

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

        layoutInfo.bindingCount = 2;

        layoutInfo.pBindings = bindings_0.data();
        if (vkCreateDescriptorSetLayout(gpu.logical_gpu, &layoutInfo, nullptr, &descriptorSetLayout_0) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        layoutInfo.bindingCount = 1;

        layoutInfo.pBindings = bindings_1.data();
        if (vkCreateDescriptorSetLayout(gpu.logical_gpu, &layoutInfo, nullptr, &descriptorSetLayout_1) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        layoutInfo.pBindings = bindings_2.data();
        if (vkCreateDescriptorSetLayout(gpu.logical_gpu, &layoutInfo, nullptr, &descriptorSetLayout_2) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void createFramebuffers() {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::array<VkImageView, 3> attachments = {
                msaa->getColorImageView(),
                depthImageView,
                swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass->getRenderPass();
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(gpu.logical_gpu, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createDepthResources() {
        VkFormat depthFormat = findDepthFormat();
        gpu.createImage(swapChainExtent.width, swapChainExtent.height, msaa->getSampleCount(),
            depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImage);
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(gpu.logical_gpu, depthImage, &memRequirements);
        gpu.allocateMemory(memRequirements.size,
            gpu.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
            depthImageMemory);
        vkBindImageMemory(gpu.logical_gpu, depthImage, depthImageMemory, 0);
        depthImageView = gpu.createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(gpu.physical_gpu, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat findDepthFormat() {
        return findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    bool hasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void createDescriptorPool() {
        /*
        create the discriptor pool
        */

        // two types of discriptor
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        
        // the first type is uniform buffer
        // (model matrix, view matrix, projection matrix, eye location, and light)
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT *
            (scene->meshes.size() + scene->meshes_with_normal_map.size() + 2));

        // the second type image samplers for texture mapping
        // TODO: maybe we only need as many descriptors and descriptor set as the number of textures.
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(
            MAX_FRAMES_IN_FLIGHT * (scene->textures.size() + scene->normal_maps.size()) + 1);

        // prepare for pool creation
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT *
            (scene->meshes.size() + scene->meshes_with_normal_map.size() +
            1 + scene->textures.size() + scene->normal_maps.size()) + 1);

        // create the pool
        if (vkCreateDescriptorPool(gpu.logical_gpu, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    std::vector<VkDescriptorSetLayout> arrange_layouts() {
        std::vector<VkDescriptorSetLayout> layouts;

        // for each frame
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

            // this set is for view matrix, projection matrix, eye location, and lights
            layouts.push_back(descriptorSetLayout_0);

            // these sets are for textures
            for (int j = 0; j < scene->textures.size(); j++)
                layouts.push_back(descriptorSetLayout_1);

            // these sets are for normal maps
            for (int j = 0; j < scene->normal_maps.size(); j++)
                layouts.push_back(descriptorSetLayout_1);

            // these sets are for model matrices
            for (int j = 0; j < scene->meshes.size(); j++)
                layouts.push_back(descriptorSetLayout_2);
            for (int j = 0; j < scene->meshes_with_normal_map.size(); j++)
                layouts.push_back(descriptorSetLayout_2);
        }

        return layouts;
    }

    void allocate_descriptor_sets() {
        // arrange the layouts
        std::vector<VkDescriptorSetLayout> layouts = arrange_layouts();
        
        // prepare for allocation
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
        allocInfo.pSetLayouts = layouts.data();

        // allocate
        descriptorSets.resize(layouts.size());
        if (vkAllocateDescriptorSets(gpu.logical_gpu, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
    }

    std::vector<VkDescriptorBufferInfo> prepare_buffer_info() {
        
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = scene->uniform_buffer->buffer;
        std::vector<VkDescriptorBufferInfo> bufferInfos(
            (2 + scene->meshes.size() + scene->meshes_with_normal_map.size()) *
            MAX_FRAMES_IN_FLIGHT, buffer_info);

        VkDeviceSize offset = 0;
        int index = 0;
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            
            // view matrix and projection matrix
            bufferInfos[index].offset = offset;
            bufferInfos[index].range = sizeof(ViewProjectrion);
            offset += gpu.getAlignSize(sizeof(ViewProjectrion));
            index++;

            // eye location and lights
            bufferInfos[index].offset = offset;
            bufferInfos[index].range = sizeof(FragmentUniform);
            offset += gpu.getAlignSize(sizeof(FragmentUniform));
            index++;

            // meshes
            for (int j = 0; j < scene->meshes.size() + scene->meshes_with_normal_map.size(); j++) {
                bufferInfos[index].offset = offset;
                bufferInfos[index].range = sizeof(glm::mat4);
                offset += gpu.getAlignSize(sizeof(glm::mat4));
                index++;
            }
        }

        return bufferInfos;
    }

    std::vector<VkDescriptorImageInfo> prepare_image_info() {

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.sampler = textureSampler;
        std::vector<VkDescriptorImageInfo> imageInfos(
            (scene->textures.size() + scene->normal_maps.size()) *
            MAX_FRAMES_IN_FLIGHT, image_info);

        int index = 0;
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            
            // textures
            for (int j = 0; j < scene->textures.size(); j++) {
                imageInfos[index].imageView = textureImageView[j];
                index++;
            }

            // normal maps
            for (int j = 0; j < scene->normal_maps.size(); j++) {
                imageInfos[index].imageView = normalMapImageView[j];
                index++;
            }
        }

        return imageInfos;
    }

    std::vector<VkWriteDescriptorSet> prepare_descriptor_write(
        std::vector<VkDescriptorBufferInfo>& bufferInfos,
        std::vector<VkDescriptorImageInfo>& imageInfos
    ) {

        std::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.resize((2 + scene->textures.size() + scene->normal_maps.size() +
            scene->meshes.size() + scene->meshes_with_normal_map.size()) * MAX_FRAMES_IN_FLIGHT);

        int write_index = 0, set_index = 0, buffer_index = 0, image_index = 0;
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            
            // view matrix and projection matrix
            updateDescriptorWrite(descriptorWrites[write_index], descriptorSets[set_index], 0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfos[buffer_index], nullptr);
            write_index++; buffer_index++;

            // eye location and lights
            updateDescriptorWrite(descriptorWrites[write_index], descriptorSets[set_index], 1,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfos[buffer_index], nullptr);
            write_index++; buffer_index++; set_index++;
            
            // textures and normal maps
            for (int j = 0; j < scene->textures.size() + scene->normal_maps.size(); j++) {
                updateDescriptorWrite(descriptorWrites[write_index], descriptorSets[set_index], 0,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr, &imageInfos[image_index]);
                write_index++; image_index++; set_index++;
            }

            // meshes
            for (int j = 0; j < scene->meshes.size() + scene->meshes_with_normal_map.size(); j++) {
                updateDescriptorWrite(descriptorWrites[write_index], descriptorSets[set_index], 0,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfos[buffer_index], nullptr);
                write_index++; buffer_index++; set_index++;
            }
        }

        return descriptorWrites;
    }

    void createDescriptorSets() {
        /*
        create all the descriptor sets
        */

        allocate_descriptor_sets();

        std::vector<VkDescriptorBufferInfo> bufferInfos = prepare_buffer_info();

        std::vector<VkDescriptorImageInfo> imageInfos = prepare_image_info();

        std::vector<VkWriteDescriptorSet> descriptorWrites = prepare_descriptor_write(
            bufferInfos, imageInfos);

        // use the descriptorWrites to update descriptorSets
        vkUpdateDescriptorSets(gpu.logical_gpu, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    void updateDescriptorWrite(VkWriteDescriptorSet& descriptorWrite, VkDescriptorSet set, uint32_t binding,
        VkDescriptorType type, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo) {
        /*
        update a VkWriteDescriptorSet
        */

        descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = set;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = type;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = bufferInfo;
        descriptorWrite.pImageInfo = imageInfo;
    }

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = gpu.beginSingleTimeCommands();

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

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        gpu.endSingleTimeCommands(commandBuffer);
    }

    void copyBufferToImage(VkBuffer buffer, VkDeviceSize buffer_offset, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = gpu.beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = buffer_offset;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        gpu.endSingleTimeCommands(commandBuffer);
    }

    void createCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = gpu.commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

        if (vkAllocateCommandBuffers(gpu.logical_gpu, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void begin_command_buffer(VkCommandBuffer commandBuffer) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
    }

    void begin_render_pass(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass->getRenderPass();
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void set_viewport(VkCommandBuffer commandBuffer) {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    }

    void set_scissor(VkCommandBuffer commandBuffer) {
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void bind_vertex_and_index_buffer(VkCommandBuffer commandBuffer) {
        VkBuffer vertexBuffers[2] = { scene->vertex_buffer->buffer, scene->vertex_buffer->buffer };
        VkDeviceSize offsets[2] = { 0, sizeof(Vertex) * scene->get_num_vertices() };
        vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, scene->index_buffer->buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void bind_global_uniform(VkCommandBuffer commandBuffer) {
        /*
        This includes view matrix, projection matrix, lights, eye position
        */

        int index = (1 + scene->textures.size() + scene->normal_maps.size() + scene->meshes.size() +
            scene->meshes_with_normal_map.size()) * currentFrame;
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            basic_graphic_pipeline.layout, 0, 1, &descriptorSets[index], 0, nullptr);
    }

    void bind_texture(VkCommandBuffer commandBuffer, int i) {
        /*
        Bind the ith texture
        */
        int index = (1 + scene->textures.size() + scene->normal_maps.size() + scene->meshes.size() +
            scene->meshes_with_normal_map.size()) * currentFrame + 1 + i;
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            basic_graphic_pipeline.layout, 1, 1, &descriptorSets[index], 0, nullptr);
    }

    void render_basic_mesh(VkCommandBuffer commandBuffer, int i) {
        /*
        Render any basic mesh that has the ith texture
        */
        
        // bind the pipeline to render basic meshes first
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, basic_graphic_pipeline.pipeline);

        // for each mesh
        for (int j = 0; j < scene->meshes.size(); j++) {

            // if the mesh use the ith texture
            if (scene->meshes[j].texture_index == i) {

                // bind the model matrix
                int index = (1 + scene->textures.size() + scene->normal_maps.size() + scene->meshes.size() +
                    scene->meshes_with_normal_map.size()) * currentFrame + 1 + scene->textures.size() +
                    scene->normal_maps.size() + j;
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    basic_graphic_pipeline.layout, 2, 1, &descriptorSets[index], 0, nullptr);

                // draw call
                vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(scene->meshes[j].indices.size()),
                    1, scene->meshes[j].index_offset, scene->meshes[j].vertex_offset, 0);
            }
        }
    }

    void render_normal_map_meshes_without_normal_map(VkCommandBuffer commandBuffer, int i) {
        // bind the pipeline that render the meshes_with_normal_map
        // without normal mapping
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, basic_t_graphic_pipeline.pipeline);

        // for each mesh
        for (int j = 0; j < scene->meshes_with_normal_map.size(); j++) {

            // if the mesh use the ith texture
            if (scene->meshes_with_normal_map[j].texture_index == i) {

                // bind the model matrix
                int index = (1 + scene->textures.size() + scene->normal_maps.size() + scene->meshes.size() +
                    scene->meshes_with_normal_map.size()) * currentFrame + 1 + scene->textures.size() +
                    scene->normal_maps.size() + scene->meshes.size() + j;
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    basic_t_graphic_pipeline.layout, 2, 1, &descriptorSets[index],0, nullptr);

                // draw call
                vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(scene->meshes_with_normal_map[j].indices.size()),
                    1, scene->meshes_with_normal_map[j].index_offset, scene->meshes_with_normal_map[j].vertex_offset, 0);
            }
        }
    }

    void render_normal_map_meshes(VkCommandBuffer commandBuffer, int i) {
        // bind normal mapping pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, normal_mapping_pipeline.pipeline);

        // for each mesh
        for (int j = 0; j < scene->meshes_with_normal_map.size(); j++) {

            // if the mesh use the ith texture
            if (scene->meshes_with_normal_map[j].texture_index == i) {

                // bind the normal map and the model matrix
                int index_0 = (1 + scene->textures.size() + scene->normal_maps.size() + scene->meshes.size() +
                    scene->meshes_with_normal_map.size())* currentFrame + 1 + scene->textures.size() +
                    scene->meshes_with_normal_map[j].normal_map_index;
                int index_1 = (1 + scene->textures.size() + scene->normal_maps.size() + scene->meshes.size() +
                    scene->meshes_with_normal_map.size()) * currentFrame + 1 + scene->textures.size() +
                    scene->normal_maps.size() + scene->meshes.size() + j;
                std::vector<VkDescriptorSet> descriptor_sets = { descriptorSets[index_0], descriptorSets[index_1] };
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    normal_mapping_pipeline.layout, 2, 2, descriptor_sets.data(), 0, nullptr);

                // draw call
                vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(scene->meshes_with_normal_map[j].indices.size()),
                    1, scene->meshes_with_normal_map[j].index_offset, scene->meshes_with_normal_map[j].vertex_offset, 0);
            }
        }
    }

    void render_all_meshes(VkCommandBuffer commandBuffer) {
        bind_vertex_and_index_buffer(commandBuffer);

        bind_global_uniform(commandBuffer);

        // for each texture
        for (int i = 0; i < scene->textures.size(); i++) {

            bind_texture(commandBuffer, i);

            render_basic_mesh(commandBuffer, i);

            // if normal mapping is disabled, draw meshes with normal map
            // the same way as meshes
            if (!scene->enable_normal_map)
                render_normal_map_meshes_without_normal_map(commandBuffer, i);
            else render_normal_map_meshes(commandBuffer, i);
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        
        begin_command_buffer(commandBuffer);

        begin_render_pass(commandBuffer, imageIndex);

        set_viewport(commandBuffer);

        set_scissor(commandBuffer);

        render_all_meshes(commandBuffer);

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(imgui_draw_data, commandBuffer);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer!");
    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(gpu.logical_gpu, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(gpu.logical_gpu, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(gpu.logical_gpu, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    uint32_t get_next_image() {
        
        // wait for the current frame to finish rendering
        vkWaitForFences(gpu.logical_gpu, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        // get the next available image
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(gpu.logical_gpu, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return -1;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // reset the fence
        vkResetFences(gpu.logical_gpu, 1, &inFlightFences[currentFrame]);

        // return
        return imageIndex;
    }

    void update_view_projection(char* p, size_t& offset) {
        ViewProjectrion view_proj_matrix;
        view_proj_matrix.view = lookAt(
            scene->camera.cameraPos,
            scene->camera.cameraPos + scene->camera.cameraFront,
            scene->camera.cameraUp
        );
        view_proj_matrix.proj = perspective(
            glm::radians(45.0f),
            swapChainExtent.width / (float)swapChainExtent.height,
            0.1f, 100.0f
        );
        view_proj_matrix.proj[1][1] *= -1;
        memcpy(p + offset, &view_proj_matrix, sizeof(ViewProjectrion));
        offset += gpu.getAlignSize(sizeof(ViewProjectrion));
    }

    void update_eye(char* p, size_t& offset) {
        fubo.eye = scene->camera.cameraPos;
        memcpy(p + offset, &fubo, sizeof(FragmentUniform));
        offset += gpu.getAlignSize(sizeof(FragmentUniform));
    }

    void update_model_tranforms(char* p, size_t& offset) {
        for (int i = 0; i < scene->meshes.size(); i++) {
            memcpy(p + offset, &scene->meshes[i].init_transform, sizeof(glm::mat4));
            offset += gpu.getAlignSize(sizeof(glm::mat4));
        }
        for (int i = 0; i < scene->meshes_with_normal_map.size(); i++) {
            memcpy(p + offset,
                &scene->meshes_with_normal_map[i].init_transform, sizeof(glm::mat4));
            offset += gpu.getAlignSize(sizeof(glm::mat4));
        }
    }

    void update_uniform_buffer() {
        /*
        Update the uniform buffer
        */

        // memory address and offset
        char* p = (char*)scene->uniformBuffersMapped;
        size_t offset = currentFrame * (
            gpu.getAlignSize(sizeof(ViewProjectrion)) +
            gpu.getAlignSize(sizeof(FragmentUniform)) +
            gpu.getAlignSize(sizeof(glm::mat4)) *
            (scene->meshes.size() + scene->meshes_with_normal_map.size())
        );

        // update the view and projection matrix
        update_view_projection(p, offset);

        // update the eye location
        update_eye(p, offset);

        // update model transforms
        update_model_tranforms(p, offset);
    }

    void submit_draw_command_buffer() {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(gpu.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
    }

    void present_swap_chain_image(uint32_t imageIndex) {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        VkResult result = vkQueuePresentKHR(gpu.presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }

    void drawFrame() {
        
        uint32_t imageIndex = get_next_image();
        if (imageIndex == -1) return;

        update_uniform_buffer();

        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        submit_draw_command_buffer();

        present_swap_chain_image(imageIndex);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};

int main() {
    Application app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}