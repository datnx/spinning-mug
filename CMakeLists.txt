cmake_minimum_required(VERSION 2.8)

project(spinning-mug)

set(CMAKE_CXX_STANDARD 17)

set(GLFW_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/glfw/include")
set(GLM_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/glm")
set(STB_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/stb")
set(SM_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")

set(SM_HEADERS
	${PROJECT_SOURCE_DIR}/include/anti_alias.h
	${PROJECT_SOURCE_DIR}/include/buffer.h
	${PROJECT_SOURCE_DIR}/include/camera.h
	${PROJECT_SOURCE_DIR}/include/gpu.h
	${PROJECT_SOURCE_DIR}/include/imconfig.h
	${PROJECT_SOURCE_DIR}/include/imgui.h
	${PROJECT_SOURCE_DIR}/include/imgui_impl_glfw.h
	${PROJECT_SOURCE_DIR}/include/imgui_impl_vulkan.h
	${PROJECT_SOURCE_DIR}/include/imgui_internal.h
	${PROJECT_SOURCE_DIR}/include/imstb_rectpack.h
	${PROJECT_SOURCE_DIR}/include/imstb_textedit.h
	${PROJECT_SOURCE_DIR}/include/imstb_truetype.h
	${PROJECT_SOURCE_DIR}/include/light.h
	${PROJECT_SOURCE_DIR}/include/load_model.h
	${PROJECT_SOURCE_DIR}/include/sm_math.h
	${PROJECT_SOURCE_DIR}/include/pipeline.h
	${PROJECT_SOURCE_DIR}/include/render_pass.h
	${PROJECT_SOURCE_DIR}/include/scene.h
	${PROJECT_SOURCE_DIR}/include/string_utils.h
	${PROJECT_SOURCE_DIR}/include/transform.h
	${PROJECT_SOURCE_DIR}/include/vertex.h)

set(SM_SOURCE
	${PROJECT_SOURCE_DIR}/src/anti_alias.cpp
	${PROJECT_SOURCE_DIR}/src/buffer.cpp
	${PROJECT_SOURCE_DIR}/src/camera.cpp
	${PROJECT_SOURCE_DIR}/src/gpu.cpp
	${PROJECT_SOURCE_DIR}/src/imgui.cpp
	${PROJECT_SOURCE_DIR}/src/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/src/imgui_impl_glfw.cpp
	${PROJECT_SOURCE_DIR}/src/imgui_impl_vulkan.cpp
	${PROJECT_SOURCE_DIR}/src/imgui_tables.cpp
	${PROJECT_SOURCE_DIR}/src/imgui_widgets.cpp
	${PROJECT_SOURCE_DIR}/src/light.cpp
	${PROJECT_SOURCE_DIR}/src/load_model.cpp
	${PROJECT_SOURCE_DIR}/src/main.cpp
	${PROJECT_SOURCE_DIR}/src/math.cpp
	${PROJECT_SOURCE_DIR}/src/pipeline.cpp
	${PROJECT_SOURCE_DIR}/src/render_pass.cpp
	${PROJECT_SOURCE_DIR}/src/scene.cpp
	${PROJECT_SOURCE_DIR}/src/string_utils.cpp
	${PROJECT_SOURCE_DIR}/src/transform.cpp
	${PROJECT_SOURCE_DIR}/src/vertex.cpp)

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
	PRIVATE ${STB_INCLUDE_DIRS}
	PRIVATE ${SM_INCLUDE_DIRS})

target_link_libraries(spinning-mug ${Vulkan_LIBRARY} glfw)