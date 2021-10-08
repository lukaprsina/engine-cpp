set(ENGINE_SRC ${CMAKE_SOURCE_DIR}/engine/src  CACHE STRING "Engine source directory")

set(CORE_FILES
    # Header Files
    ${ENGINE_SRC}/core/application.h
    ${ENGINE_SRC}/core/options.h
    ${ENGINE_SRC}/core/log.h
    ${ENGINE_SRC}/core/timer.h
    # Source Files
    ${ENGINE_SRC}/core/application.cpp
    ${ENGINE_SRC}/core/options.cpp
    ${ENGINE_SRC}/core/log.cpp
    ${ENGINE_SRC}/core/timer.cpp)

set(COMMON_FILES
    # Header Files
    ${ENGINE_SRC}/common/base.h
    ${ENGINE_SRC}/common/error.h
    ${ENGINE_SRC}/common/vulkan.h
    ${ENGINE_SRC}/common/strings.h
    ${ENGINE_SRC}/common/resource_caching.h
    # Source files
    ${ENGINE_SRC}/common/base.cpp
    ${ENGINE_SRC}/common/error.cpp
    ${ENGINE_SRC}/common/vulkan.cpp
    ${ENGINE_SRC}/common/strings.cpp)

set(EVENT_FILES # Header Files
    ${ENGINE_SRC}/events/application_event.h ${ENGINE_SRC}/events/event.h ${ENGINE_SRC}/events/key_event.h
    ${ENGINE_SRC}/events/mouse_event.h)

set(WINDOW_FILES
    # Header Files
    ${ENGINE_SRC}/window/glfw_window.h
    ${ENGINE_SRC}/window/window.h
    ${ENGINE_SRC}/window/headless_window.h
    # Source Files
    ${ENGINE_SRC}/window/window.cpp
    ${ENGINE_SRC}/window/headless_window.cpp
    ${ENGINE_SRC}/window/glfw_window.cpp)

set(PLATFORM_FILES
    # Header Files
    ${ENGINE_SRC}/platform/platform.h
    # Source Files
    ${ENGINE_SRC}/platform/platform.cpp)

set(WINDOWS_FILES
    # Header Files
    ${ENGINE_SRC}/platform/windows_platform.h
    # Source Files
    ${ENGINE_SRC}/platform/windows_platform.cpp ${ENGINE_SRC}/window/glfw_input.cpp)

set(UNIX_FILES
    # Header Files
    ${ENGINE_SRC}/platform/unix_platform.h
    # Source Files
    ${ENGINE_SRC}/platform/unix_platform.cpp ${ENGINE_SRC}/window/glfw_input.cpp)

set(VULKAN_API_FILES
    # Header files
    ${ENGINE_SRC}/vulkan_api/instance.h
    ${ENGINE_SRC}/vulkan_api/physical_device.h
    ${ENGINE_SRC}/vulkan_api/device.h
    ${ENGINE_SRC}/vulkan_api/resource_cache.h
    ${ENGINE_SRC}/vulkan_api/queue.h
    ${ENGINE_SRC}/vulkan_api/command_pool.h
    ${ENGINE_SRC}/vulkan_api/fence_pool.h
    ${ENGINE_SRC}/vulkan_api/render_context.h
    ${ENGINE_SRC}/vulkan_api/swapchain.h
    ${ENGINE_SRC}/vulkan_api/command_buffer.h
    ${ENGINE_SRC}/vulkan_api/core/image.h
    ${ENGINE_SRC}/vulkan_api/core/image_view.h
    ${ENGINE_SRC}/vulkan_api/render_target.h
    ${ENGINE_SRC}/vulkan_api/render_frame.h
    ${ENGINE_SRC}/vulkan_api/semaphore_pool.h
    ${ENGINE_SRC}/vulkan_api/queue_family.h
    ${ENGINE_SRC}/vulkan_api/rendering/render_pipeline.h
    ${ENGINE_SRC}/vulkan_api/subpasses/subpass.h
    ${ENGINE_SRC}/renderer/shader.h
    ${ENGINE_SRC}/platform/filesystem.h
    ${ENGINE_SRC}/vulkan_api/subpasses/geometry_subpass.h
    ${ENGINE_SRC}/vulkan_api/subpasses/forward_subpass.h
    ${ENGINE_SRC}/scene/scene.h
    ${ENGINE_SRC}/scene/components/camera.h
    ${ENGINE_SRC}/vulkan_api/core/pipeline_layout.h
    ${ENGINE_SRC}/vulkan_api/core/descriptor_set_layout.h
    ${ENGINE_SRC}/vulkan_api/core/descriptor_pool.h
    ${ENGINE_SRC}/vulkan_api/core/render_pass.h
    ${ENGINE_SRC}/vulkan_api/core/descriptor_set.h
    ${ENGINE_SRC}/vulkan_api/core/framebuffer.h
    ${ENGINE_SRC}/scene/gltf_loader.h
    ${ENGINE_SRC}/scene/components/light.h
    ${ENGINE_SRC}/vulkan_api/rendering/resource_binding_state.h
    ${ENGINE_SRC}/vulkan_api/rendering/pipeline_state.h
    ${ENGINE_SRC}/vulkan_api/core/sampler.h
    ${ENGINE_SRC}/vulkan_api/core/buffer.h
    ${ENGINE_SRC}/vulkan_api/core/buffer_pool.h
    ${ENGINE_SRC}/vulkan_api/core/pipeline.h
    ${ENGINE_SRC}/renderer/glsl_compiler.h
    ${ENGINE_SRC}/renderer/spirv_reflection.h
    ${ENGINE_SRC}/vulkan_api/resource_record.h
    ${ENGINE_SRC}/vulkan_api/resource_replay.h
    ${ENGINE_SRC}/scene/entity.h
    ${ENGINE_SRC}/scene/components/sampler.h
    ${ENGINE_SRC}/scene/components/image.h
    ${ENGINE_SRC}/scene/components/image/ktx.h
    ${ENGINE_SRC}/scene/components/image/stb.h
    ${ENGINE_SRC}/scene/components/image/astc.h
    ${ENGINE_SRC}/scene/components/texture.h
    ${ENGINE_SRC}/scene/components/material.h
    ${ENGINE_SRC}/scene/components/pbr_material.h
    ${ENGINE_SRC}/scene/components/mesh.h
    ${ENGINE_SRC}/scene/components/aabb.h
    ${ENGINE_SRC}/scene/components/submesh.h
    ${ENGINE_SRC}/scene/components/transform.h
    ${ENGINE_SRC}/scene/components/perspective_camera.h
    ${ENGINE_SRC}/scene/script.h
    ${ENGINE_SRC}/scene/scripts/free_camera.h
    ${ENGINE_SRC}/window/input.h
    ${ENGINE_SRC}/vulkan_api/initializers.h
    ${ENGINE_SRC}/core/layer.h
    ${ENGINE_SRC}/core/layer_stack.h
    ${ENGINE_SRC}/core/gui.h
    # Source files
    ${ENGINE_SRC}/vulkan_api/instance.cpp
    ${ENGINE_SRC}/vulkan_api/physical_device.cpp
    ${ENGINE_SRC}/vulkan_api/device.cpp
    ${ENGINE_SRC}/vulkan_api/resource_cache.cpp
    ${ENGINE_SRC}/vulkan_api/queue.cpp
    ${ENGINE_SRC}/vulkan_api/command_pool.cpp
    ${ENGINE_SRC}/vulkan_api/fence_pool.cpp
    ${ENGINE_SRC}/vulkan_api/render_context.cpp
    ${ENGINE_SRC}/vulkan_api/swapchain.cpp
    ${ENGINE_SRC}/vulkan_api/command_buffer.cpp
    ${ENGINE_SRC}/vulkan_api/core/image.cpp
    ${ENGINE_SRC}/vulkan_api/core/image_view.cpp
    ${ENGINE_SRC}/vulkan_api/render_target.cpp
    ${ENGINE_SRC}/vulkan_api/render_frame.cpp
    ${ENGINE_SRC}/vulkan_api/semaphore_pool.cpp
    ${ENGINE_SRC}/vulkan_api/queue_family.cpp
    ${ENGINE_SRC}/vulkan_api/rendering/render_pipeline.cpp
    ${ENGINE_SRC}/vulkan_api/subpasses/subpass.cpp
    ${ENGINE_SRC}/renderer/shader.cpp
    ${ENGINE_SRC}/platform/filesystem.cpp
    ${ENGINE_SRC}/vulkan_api/subpasses/geometry_subpass.cpp
    ${ENGINE_SRC}/vulkan_api/subpasses/forward_subpass.cpp
    ${ENGINE_SRC}/scene/scene.cpp
    ${ENGINE_SRC}/scene/components/camera.cpp
    ${ENGINE_SRC}/vulkan_api/core/pipeline_layout.cpp
    ${ENGINE_SRC}/vulkan_api/core/descriptor_set_layout.cpp
    ${ENGINE_SRC}/vulkan_api/core/descriptor_pool.cpp
    ${ENGINE_SRC}/vulkan_api/core/render_pass.cpp
    ${ENGINE_SRC}/vulkan_api/core/descriptor_set.cpp
    ${ENGINE_SRC}/vulkan_api/core/framebuffer.cpp
    ${ENGINE_SRC}/scene/gltf_loader.cpp
    ${ENGINE_SRC}/scene/components/light.cpp
    ${ENGINE_SRC}/vulkan_api/rendering/resource_binding_state.cpp
    ${ENGINE_SRC}/vulkan_api/rendering/pipeline_state.cpp
    ${ENGINE_SRC}/vulkan_api/core/sampler.cpp
    ${ENGINE_SRC}/vulkan_api/core/buffer.cpp
    ${ENGINE_SRC}/vulkan_api/core/buffer_pool.cpp
    ${ENGINE_SRC}/vulkan_api/core/pipeline.cpp
    ${ENGINE_SRC}/renderer/glsl_compiler.cpp
    ${ENGINE_SRC}/renderer/spirv_reflection.cpp
    ${ENGINE_SRC}/vulkan_api/resource_record.cpp
    ${ENGINE_SRC}/vulkan_api/resource_replay.cpp
    ${ENGINE_SRC}/scene/entity.cpp
    ${ENGINE_SRC}/scene/components/sampler.cpp
    ${ENGINE_SRC}/scene/components/image.cpp
    ${ENGINE_SRC}/scene/components/image/ktx.cpp
    ${ENGINE_SRC}/scene/components/image/stb.cpp
    ${ENGINE_SRC}/scene/components/image/astc.cpp
    ${ENGINE_SRC}/scene/components/texture.cpp
    ${ENGINE_SRC}/scene/components/material.cpp
    ${ENGINE_SRC}/scene/components/pbr_material.cpp
    ${ENGINE_SRC}/scene/components/mesh.cpp
    ${ENGINE_SRC}/scene/components/aabb.cpp
    ${ENGINE_SRC}/scene/components/submesh.cpp
    ${ENGINE_SRC}/scene/components/transform.cpp
    ${ENGINE_SRC}/scene/components/perspective_camera.cpp
    ${ENGINE_SRC}/scene/script.cpp
    ${ENGINE_SRC}/scene/scripts/free_camera.cpp
    ${ENGINE_SRC}/core/layer.cpp
    ${ENGINE_SRC}/core/layer_stack.cpp
    ${ENGINE_SRC}/entrypoint.cpp
    ${ENGINE_SRC}/core/gui.cpp
)

list(APPEND ENG_PROJECT_FILES ${CORE_FILES} ${COMMON_FILES} ${WINDOW_FILES}
                  ${PLATFORM_FILES} ${VULKAN_API_FILES})

if(WIN32)
  list(APPEND ENG_PROJECT_FILES ${WINDOWS_FILES})
elseif(UNIX)
  list(APPEND ENG_PROJECT_FILES ${UNIX_FILES})
endif()

add_executable(engine WIN32 ${ENG_PROJECT_FILES})

if(ENG_SHIPPING)
  target_compile_definitions(engine PUBLIC ENG_SHIPPING)
endif()

if(ENG_VALIDATION_LAYERS)
  target_compile_definitions(engine PUBLIC ENG_VALIDATION_LAYERS)
endif()

if(ENG_VALIDATION_LAYERS_GPU_ASSISTED)
  target_compile_definitions(engine PUBLIC ENG_VALIDATION_LAYERS_GPU_ASSISTED)
endif()

target_include_directories(
  engine
  PUBLIC ${ENGINE_SRC}
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_precompile_headers(engine PUBLIC ${ENGINE_SRC}/pch.h)

configure_file(engine_config.h.in engine_config.h @ONLY)

if(MSVC)
  target_compile_options(engine PRIVATE /W3 /WX)
else()
  target_compile_options(engine PRIVATE -Wall) # -Weffc++)
endif()

if(ENG_SHIPPING)
  file(GLOB ENG_ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/shaders
       ${CMAKE_CURRENT_SOURCE_DIR}/assets)
  file(COPY ${ENG_ASSETS} DESTINATION ${CMAKE_BINARY_DIR}/engine)
endif()

add_subdirectory(vendor)
