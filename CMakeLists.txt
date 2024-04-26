cmake_minimum_required(VERSION 2.8)

project(spinning-mug)

set(CMAKE_CXX_STANDARD 17)

set(GLFW_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/glfw/include")
set(GLM_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/glm")
set(STB_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/stb")

set(SM_HEADERS
	${PROJECT_SOURCE_DIR}/anti_alias.h
	${PROJECT_SOURCE_DIR}/buffer.h
	${PROJECT_SOURCE_DIR}/camera.h
	${PROJECT_SOURCE_DIR}/gpu.h
	${PROJECT_SOURCE_DIR}/imconfig.h
	${PROJECT_SOURCE_DIR}/imgui.h
	${PROJECT_SOURCE_DIR}/imgui_impl_glfw.h
	${PROJECT_SOURCE_DIR}/imgui_impl_vulkan.h
	${PROJECT_SOURCE_DIR}/imgui_internal.h
	${PROJECT_SOURCE_DIR}/imstb_rectpack.h
	${PROJECT_SOURCE_DIR}/imstb_textedit.h
	${PROJECT_SOURCE_DIR}/imstb_truetype.h
	${PROJECT_SOURCE_DIR}/light.h
	${PROJECT_SOURCE_DIR}/load_model.h
	${PROJECT_SOURCE_DIR}/math.h
	${PROJECT_SOURCE_DIR}/pipeline.h
	${PROJECT_SOURCE_DIR}/render_pass.h
	${PROJECT_SOURCE_DIR}/scene.h
	${PROJECT_SOURCE_DIR}/string_utils.h
	${PROJECT_SOURCE_DIR}/transform.h
	${PROJECT_SOURCE_DIR}/vertex.h)

set(SM_SOURCE
	${PROJECT_SOURCE_DIR}/anti_alias.cpp
	${PROJECT_SOURCE_DIR}/buffer.cpp
	${PROJECT_SOURCE_DIR}/camera.cpp
	${PROJECT_SOURCE_DIR}/gpu.cpp
	${PROJECT_SOURCE_DIR}/imgui.cpp
	${PROJECT_SOURCE_DIR}/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/imgui_impl_glfw.cpp
	${PROJECT_SOURCE_DIR}/imgui_impl_vulkan.cpp
	${PROJECT_SOURCE_DIR}/imgui_tables.cpp
	${PROJECT_SOURCE_DIR}/imgui_widgets.cpp
	${PROJECT_SOURCE_DIR}/light.cpp
	${PROJECT_SOURCE_DIR}/load_model.cpp
	${PROJECT_SOURCE_DIR}/main.cpp
	${PROJECT_SOURCE_DIR}/math.cpp
	${PROJECT_SOURCE_DIR}/pipeline.cpp
	${PROJECT_SOURCE_DIR}/render_pass.cpp
	${PROJECT_SOURCE_DIR}/scene.cpp
	${PROJECT_SOURCE_DIR}/string_utils.cpp
	${PROJECT_SOURCE_DIR}/transform.cpp
	${PROJECT_SOURCE_DIR}/vertex.cpp)

add_subdirectory(external/glfw)

find_package(Vulkan)

set(SHADERS
	${PROJECT_SOURCE_DIR}/shaders/shader.vert
	${PROJECT_SOURCE_DIR}/shaders/shader_t.vert
	${PROJECT_SOURCE_DIR}/shaders/shader.frag
	${PROJECT_SOURCE_DIR}/shaders/normal_mapping.vert
	${PROJECT_SOURCE_DIR}/shaders/normal_mapping.frag)

foreach(GLSL ${SHADERS})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
	message("${SPIRV}")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL})
	list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(shaders DEPENDS ${SPIRV_BINARY_FILES})

add_executable(spinning-mug ${SM_SOURCE} ${SM_HEADERS})

set_property(TARGET spinning-mug PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

add_dependencies(spinning-mug shaders)

target_include_directories(spinning-mug
	PRIVATE ${Vulkan_INCLUDE_DIR}
	PRIVATE ${GLFW_INCLUDE_DIRS}
	PRIVATE ${GLM_INCLUDE_DIRS}
	PRIVATE ${STB_INCLUDE_DIRS})

target_link_libraries(spinning-mug ${Vulkan_LIBRARY} glfw)