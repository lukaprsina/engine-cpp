set(CORE_FILES
    # Header Files
    src/core/application.h
    src/core/options.h
    src/core/log.h
    src/core/timer.h
    # Source Files
    src/core/application.cpp
    src/core/options.cpp
    src/core/log.cpp
    src/core/timer.cpp)

set(COMMON_FILES
    # Header Files
    src/common/base.h
    src/common/error.h
    src/common/vulkan.h
    src/common/strings.h
    src/common/resource_caching.h
    # Source files
    src/common/base.cpp
    src/common/error.cpp
    src/common/vulkan.cpp
    src/common/strings.cpp)

set(EVENT_FILES # Header Files
    src/events/application_event.h src/events/event.h src/events/key_event.h
    src/events/mouse_event.h)

set(WINDOW_FILES
    # Header Files
    src/window/glfw_window.h
    src/window/window.h
    src/window/headless_window.h
    # Source Files
    src/window/window.cpp
    src/window/headless_window.cpp
    src/window/glfw_window.cpp)

set(PLATFORM_FILES
    # Header Files
    src/platform/platform.h
    # Source Files
    src/entrypoint.cpp src/platform/platform.cpp)

set(WINDOWS_FILES
    # Header Files
    src/platform/windows_platform.h
    # Source Files
    src/platform/windows_platform.cpp)

set(UNIX_FILES
    # Header Files
    src/platform/unix_platform.h
    # Source Files
    src/platform/unix_platform.cpp)

set(VULKAN_API_FILES
    # Header files
    src/vulkan_api/instance.h
    src/vulkan_api/physical_device.h
    src/vulkan_api/device.h
    src/vulkan_api/resource_cache.h
    src/vulkan_api/queue.h
    src/vulkan_api/command_pool.h
    src/vulkan_api/fence_pool.h
    src/vulkan_api/render_context.h
    src/vulkan_api/swapchain.h
    src/vulkan_api/command_buffer.h
    src/vulkan_api/core/image.h
    src/vulkan_api/core/image_view.h
    src/vulkan_api/render_target.h
    src/vulkan_api/render_frame.h
    src/vulkan_api/semaphore_pool.h
    src/vulkan_api/queue_family.h
    src/vulkan_api/rendering/render_pipeline.h
    src/vulkan_api/subpasses/subpass.h
    src/renderer/shader.h
    src/platform/filesystem.h
    src/vulkan_api/subpasses/geometry_subpass.h
    src/vulkan_api/subpasses/forward_subpass.h
    src/scene/scene.h
    src/scene/components/camera.h
    src/vulkan_api/core/pipeline_layout.h
    src/vulkan_api/core/descriptor_set_layout.h
    src/vulkan_api/core/descriptor_pool.h
    src/vulkan_api/core/render_pass.h
    src/vulkan_api/core/descriptor_set.h
    src/vulkan_api/core/framebuffer.h
    src/scene/gltf_loader.h
    src/scene/components/light.h
    src/vulkan_api/rendering/resource_binding_state.h
    src/vulkan_api/rendering/pipeline_state.h
    src/vulkan_api/core/sampler.h
    src/vulkan_api/core/buffer.h
    src/vulkan_api/core/buffer_pool.h
    src/vulkan_api/core/pipeline.h
    src/renderer/glsl_compiler.h
    src/renderer/spirv_reflection.h
    src/vulkan_api/resource_record.h
    src/vulkan_api/resource_replay.h
    src/scene/entity.h
    src/scene/components/sampler.h
    src/scene/components/image.h
    src/scene/components/image/ktx.h
    src/scene/components/image/stb.h
    src/scene/components/image/astc.h
    src/scene/components/texture.h
    src/scene/components/material.h
    src/scene/components/pbr_material.h
    src/scene/components/mesh.h
    src/scene/components/aabb.h
    src/scene/components/submesh.h
    src/scene/components/transform.h
    src/scene/components/perspective_camera.h
    # Source files
    src/vulkan_api/instance.cpp
    src/vulkan_api/physical_device.cpp
    src/vulkan_api/device.cpp
    src/vulkan_api/resource_cache.cpp
    src/vulkan_api/queue.cpp
    src/vulkan_api/command_pool.cpp
    src/vulkan_api/fence_pool.cpp
    src/vulkan_api/render_context.cpp
    src/vulkan_api/swapchain.cpp
    src/vulkan_api/command_buffer.cpp
    src/vulkan_api/core/image.cpp
    src/vulkan_api/core/image_view.cpp
    src/vulkan_api/render_target.cpp
    src/vulkan_api/render_frame.cpp
    src/vulkan_api/semaphore_pool.cpp
    src/vulkan_api/queue_family.cpp
    src/vulkan_api/rendering/render_pipeline.cpp
    src/vulkan_api/subpasses/subpass.cpp
    src/renderer/shader.cpp
    src/platform/filesystem.cpp
    src/vulkan_api/subpasses/geometry_subpass.cpp
    src/vulkan_api/subpasses/forward_subpass.cpp
    src/scene/scene.cpp
    src/scene/components/camera.cpp
    src/vulkan_api/core/pipeline_layout.cpp
    src/vulkan_api/core/descriptor_set_layout.cpp
    src/vulkan_api/core/descriptor_pool.cpp
    src/vulkan_api/core/render_pass.cpp
    src/vulkan_api/core/descriptor_set.cpp
    src/vulkan_api/core/framebuffer.cpp
    src/scene/gltf_loader.cpp
    src/scene/components/light.cpp
    src/vulkan_api/rendering/resource_binding_state.cpp
    src/vulkan_api/rendering/pipeline_state.cpp
    src/vulkan_api/core/sampler.cpp
    src/vulkan_api/core/buffer.cpp
    src/vulkan_api/core/buffer_pool.cpp
    src/vulkan_api/core/pipeline.cpp
    src/renderer/glsl_compiler.cpp
    src/renderer/spirv_reflection.cpp
    src/vulkan_api/resource_record.cpp
    src/vulkan_api/resource_replay.cpp
    src/scene/entity.cpp
    src/scene/components/sampler.cpp
    src/scene/components/image.cpp
    src/scene/components/image/ktx.cpp
    src/scene/components/image/stb.cpp
    src/scene/components/image/astc.cpp
    src/scene/components/texture.cpp
    src/scene/components/material.cpp
    src/scene/components/pbr_material.cpp
    src/scene/components/mesh.cpp
    src/scene/components/aabb.cpp
    src/scene/components/submesh.cpp
    src/scene/components/transform.cpp
    src/scene/components/perspective_camera.cpp)

set(PROJECT_FILES ${CORE_FILES} ${COMMON_FILES} ${WINDOW_FILES}
                  ${PLATFORM_FILES} ${VULKAN_API_FILES})

if(ANDROID)
  set(ENG_PLATFORM VK_USE_PLATFORM_ANDROID_KHR)
elseif(WIN32)
  list(APPEND PROJECT_FILES ${WINDOWS_FILES})
  set(ENG_PLATFORM VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
  set(ENG_PLATFORM VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
  list(APPEND PROJECT_FILES ${UNIX_FILES})
  set(ENG_PLATFORM VK_USE_PLATFORM_XCB_KHR)
endif()

add_executable(Engine WIN32 ${PROJECT_FILES})

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_precompile_headers(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h)

target_compile_definitions(
  Engine
  PRIVATE ENG_PLATFORM
  PRIVATE ENG_VALIDATION_LAYERS
  PRIVATE ENG_VALIDATION_LAYERS_GPU_ASSISTED)

if(MSVC)
  target_compile_options(Engine PRIVATE /W3 /WX)
else()
  target_compile_options(Engine PRIVATE -Wall) # -Weffc++)
endif()

add_subdirectory("vendor")
