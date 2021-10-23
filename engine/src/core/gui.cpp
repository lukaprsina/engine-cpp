#include "core/gui.h"

#include "window/input.h"
#include "platform/platform.h"
#include "vulkan_api/rendering/render_pipeline.h"
#include "window/glfw_window.h"
#include "vulkan_api/render_context.h"
#include "vulkan_api/device.h"
#include "core/application.h"
#include "vulkan_api/render_context.h"
#include "vulkan_api/core/pipeline_layout.h"
#include "renderer/shader.h"
#include "scene/scene.h"
#include "vulkan_api/subpasses/gui_subpass.h"
#include "events/key_event.h"
#include "vulkan_api/core/image.h"
#include "vulkan_api/core/image_view.h"
#include "vulkan_api/initializers.h"
#include "vulkan_api/rendering/pipeline_state.h"
#include "vulkan_api/render_frame.h"

#include <GLFW/glfw3.h>
#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // for glfwGetWin32Window
#endif
#define GLFW_HAS_WINDOW_TOPMOST (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3200)                                  // 3.2+ GLFW_FLOATING
#define GLFW_HAS_WINDOW_HOVERED (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300)                                  // 3.3+ GLFW_HOVERED
#define GLFW_HAS_WINDOW_ALPHA (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300)                                    // 3.3+ glfwSetWindowOpacity
#define GLFW_HAS_PER_MONITOR_DPI (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300)                                 // 3.3+ glfwGetMonitorContentScale
#define GLFW_HAS_VULKAN (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3200)                                          // 3.2+ glfwCreateWindowSurface
#define GLFW_HAS_FOCUS_WINDOW (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3200)                                    // 3.2+ glfwFocusWindow
#define GLFW_HAS_FOCUS_ON_SHOW (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300)                                   // 3.3+ GLFW_FOCUS_ON_SHOW
#define GLFW_HAS_MONITOR_WORK_AREA (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300)                               // 3.3+ glfwGetMonitorWorkarea
#define GLFW_HAS_OSX_WINDOW_POS_FIX (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 + GLFW_VERSION_REVISION * 10 >= 3310) // 3.3.1+ Fixed: Resizing window repositions it on MacOS #1553
#ifdef GLFW_RESIZE_NESW_CURSOR                                                                                                  // Let's be nice to people who pulled GLFW between 2019-04-16 (3.4 define) and 2019-11-29 (cursors defines) // FIXME: Remove when GLFW 3.4 is released?
#define GLFW_HAS_NEW_CURSORS (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3400)                                     // 3.4+ GLFW_RESIZE_ALL_CURSOR, GLFW_RESIZE_NESW_CURSOR, GLFW_RESIZE_NWSE_CURSOR, GLFW_NOT_ALLOWED_CURSOR
#else
#define GLFW_HAS_NEW_CURSORS (0)
#endif
#ifdef GLFW_MOUSE_PASSTHROUGH                                                                     // Let's be nice to people who pulled GLFW between 2019-04-16 (3.4 define) and 2020-07-17 (passthrough)
#define GLFW_HAS_MOUSE_PASSTHROUGH (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3400) // 3.4+ GLFW_MOUSE_PASSTHROUGH
#else
#define GLFW_HAS_MOUSE_PASSTHROUGH (0)
#endif

namespace engine
{
    namespace
    {
        void UploadDrawData(ImDrawData *draw_data, const uint8_t *vertex_data, const uint8_t *index_data)
        {
            ImDrawVert *vtx_dst = (ImDrawVert *)vertex_data;
            ImDrawIdx *idx_dst = (ImDrawIdx *)index_data;

            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList *cmd_list = draw_data->CmdLists[n];
                memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtx_dst += cmd_list->VtxBuffer.Size;
                idx_dst += cmd_list->IdxBuffer.Size;
            }
        }

        struct ImGui_ImplVulkanH_FrameRenderBuffers
        {
            VkDeviceMemory VertexBufferMemory;
            VkDeviceMemory IndexBufferMemory;
            VkDeviceSize VertexBufferSize;
            VkDeviceSize IndexBufferSize;
            VkBuffer VertexBuffer;
            VkBuffer IndexBuffer;
        };

        // Each viewport will hold 1 ImGui_ImplVulkanH_WindowRenderBuffers
        // [Please zero-clear before use!]
        struct ImGui_ImplVulkanH_WindowRenderBuffers
        {
            uint32_t Index;
            uint32_t Count;
            ImGui_ImplVulkanH_FrameRenderBuffers *FrameRenderBuffers;
        };

        struct ImGui_ImplVulkan_InitInfo
        {
            VkInstance Instance;
            VkPhysicalDevice PhysicalDevice;
            VkDevice Device;
            uint32_t QueueFamily;
            VkQueue Queue;
            VkPipelineCache PipelineCache;
            VkDescriptorPool DescriptorPool;
            uint32_t Subpass;
            uint32_t MinImageCount;            // >= 2
            uint32_t ImageCount;               // >= MinImageCount
            VkSampleCountFlagBits MSAASamples; // >= VK_SAMPLE_COUNT_1_BIT
            const VkAllocationCallbacks *Allocator;
            void (*CheckVkResultFn)(VkResult err);
        };

        struct ImGui_ImplVulkan_Data
        {
            ImGui_ImplVulkan_InitInfo VulkanInitInfo;
            VkRenderPass RenderPass;
            VkDeviceSize BufferMemoryAlignment;
            VkPipelineCreateFlags PipelineCreateFlags;
            VkDescriptorSetLayout DescriptorSetLayout;
            VkPipelineLayout PipelineLayout;
            VkDescriptorSet DescriptorSet;
            VkPipeline Pipeline;
            uint32_t Subpass;
            VkShaderModule ShaderModuleVert;
            VkShaderModule ShaderModuleFrag;

            // Font data
            VkSampler FontSampler;
            VkDeviceMemory FontMemory;
            VkImage FontImage;
            VkImageView FontView;
            VkDeviceMemory UploadBufferMemory;
            VkBuffer UploadBuffer;

            // Render buffers for main window
            ImGui_ImplVulkanH_WindowRenderBuffers MainWindowRenderBuffers;

            ImGui_ImplVulkan_Data()
            {
                memset(this, 0, sizeof(*this));
                BufferMemoryAlignment = 256;
            }
        };

        enum GlfwClientApi
        {
            GlfwClientApi_Unknown,
            GlfwClientApi_OpenGL,
            GlfwClientApi_Vulkan
        };

        struct ImGui_ImplGlfw_Data
        {
            GLFWwindow *Window;
            GlfwClientApi ClientApi;
            double Time;
            GLFWwindow *MouseWindow;
            bool MouseJustPressed[ImGuiMouseButton_COUNT];
            GLFWcursor *MouseCursors[ImGuiMouseCursor_COUNT];
            GLFWwindow *KeyOwnerWindows[512];
            bool InstalledCallbacks;
            bool WantUpdateMonitors;

            // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
            GLFWwindowfocusfun PrevUserCallbackWindowFocus;
            GLFWcursorenterfun PrevUserCallbackCursorEnter;
            GLFWmousebuttonfun PrevUserCallbackMousebutton;
            GLFWscrollfun PrevUserCallbackScroll;
            GLFWkeyfun PrevUserCallbackKey;
            GLFWcharfun PrevUserCallbackChar;
            GLFWmonitorfun PrevUserCallbackMonitor;

            ImGui_ImplGlfw_Data() { memset(this, 0, sizeof(*this)); }
        };

        ImGui_ImplGlfw_Data *ImGui_ImplGlfw_GetBackendData()
        {
            return ImGui::GetCurrentContext() ? (ImGui_ImplGlfw_Data *)ImGui::GetIO().BackendPlatformUserData : NULL;
        }

        // Forward Declarations
        void ImGui_ImplGlfw_UpdateMonitors();
        void ImGui_ImplGlfw_InitPlatformInterface();
        void ImGui_ImplGlfw_ShutdownPlatformInterface();

        // Functions
        const char *ImGui_ImplGlfw_GetClipboardText(void *user_data)
        {
            return glfwGetClipboardString((GLFWwindow *)user_data);
        }

        void ImGui_ImplGlfw_SetClipboardText(void *user_data, const char *text)
        {
            glfwSetClipboardString((GLFWwindow *)user_data, text);
        }

        void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            if (bd->PrevUserCallbackMousebutton != NULL && window == bd->Window)
                bd->PrevUserCallbackMousebutton(window, button, action, mods);

            if (action == GLFW_PRESS && button >= 0 && button < IM_ARRAYSIZE(bd->MouseJustPressed))
                bd->MouseJustPressed[button] = true;
        }

        void ImGui_ImplGlfw_ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            if (bd->PrevUserCallbackScroll != NULL && window == bd->Window)
                bd->PrevUserCallbackScroll(window, xoffset, yoffset);

            ImGuiIO &io = ImGui::GetIO();
            io.MouseWheelH += (float)xoffset;
            io.MouseWheel += (float)yoffset;
        }

        void ImGui_ImplGlfw_KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            if (bd->PrevUserCallbackKey != NULL && window == bd->Window)
                bd->PrevUserCallbackKey(window, key, scancode, action, mods);

            ImGuiIO &io = ImGui::GetIO();
            if (key >= 0 && key < IM_ARRAYSIZE(io.KeysDown))
            {
                if (action == GLFW_PRESS)
                {
                    io.KeysDown[key] = true;
                    bd->KeyOwnerWindows[key] = window;
                }
                if (action == GLFW_RELEASE)
                {
                    io.KeysDown[key] = false;
                    bd->KeyOwnerWindows[key] = NULL;
                }
            }

            // Modifiers are not reliable across systems
            io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
            io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
            io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
#ifdef _WIN32
            io.KeySuper = false;
#else
            io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
#endif
        }

        void ImGui_ImplGlfw_WindowFocusCallback(GLFWwindow *window, int focused)
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            if (bd->PrevUserCallbackWindowFocus != NULL && window == bd->Window)
                bd->PrevUserCallbackWindowFocus(window, focused);

            ImGuiIO &io = ImGui::GetIO();
            io.AddFocusEvent(focused != 0);
        }

        void ImGui_ImplGlfw_CursorEnterCallback(GLFWwindow *window, int entered)
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            if (bd->PrevUserCallbackCursorEnter != NULL && window == bd->Window)
                bd->PrevUserCallbackCursorEnter(window, entered);

            if (entered)
                bd->MouseWindow = window;
            if (!entered && bd->MouseWindow == window)
                bd->MouseWindow = NULL;
        }

        void ImGui_ImplGlfw_CharCallback(GLFWwindow *window, unsigned int c)
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            if (bd->PrevUserCallbackChar != NULL && window == bd->Window)
                bd->PrevUserCallbackChar(window, c);

            ImGuiIO &io = ImGui::GetIO();
            io.AddInputCharacter(c);
        }

        void ImGui_ImplGlfw_MonitorCallback(GLFWmonitor *, int)
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            bd->WantUpdateMonitors = true;
        }

        bool ImGui_ImplGlfw_Init(GLFWwindow *window, bool install_callbacks, GlfwClientApi client_api)
        {
            ImGuiIO &io = ImGui::GetIO();
            IM_ASSERT(io.BackendPlatformUserData == NULL && "Already initialized a platform backend!");

            // Setup backend capabilities flags
            ImGui_ImplGlfw_Data *bd = IM_NEW(ImGui_ImplGlfw_Data)();
            io.BackendPlatformUserData = (void *)bd;
            io.BackendPlatformName = "imgui_impl_glfw";
            io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;      // We can honor GetMouseCursor() values (optional)
            io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;       // We can honor io.WantSetMousePos requests (optional, rarely used)
            io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports; // We can create multi-viewports on the Platform side (optional)
#if GLFW_HAS_MOUSE_PASSTHROUGH || (GLFW_HAS_WINDOW_HOVERED && defined(_WIN32))
            io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can set io.MouseHoveredViewport correctly (optional, not easy)
#endif
            bd->Window = window;
            bd->Time = 0.0;
            bd->WantUpdateMonitors = true;

            // Keyboard mapping. Dear ImGui will use those indices to peek into the io.KeysDown[] array.
            io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
            io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
            io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
            io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
            io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
            io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
            io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
            io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
            io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
            io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
            io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
            io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
            io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
            io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
            io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
            io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
            io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
            io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
            io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
            io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
            io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
            io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

            io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
            io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
            io.ClipboardUserData = bd->Window;

            // Create mouse cursors
            // (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
            // GLFW will emit an error which will often be printed by the app, so we temporarily disable error reporting.
            // Missing cursors will return NULL and our _UpdateMouseCursor() function will use the Arrow cursor instead.)
            GLFWerrorfun prev_error_callback = glfwSetErrorCallback(NULL);
            bd->MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
            bd->MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
            bd->MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
            bd->MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
            bd->MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
#if GLFW_HAS_NEW_CURSORS
            bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
            bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
            bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
            bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
#else
            bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
            bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
            bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
            bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
#endif
            glfwSetErrorCallback(prev_error_callback);

            // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
            bd->PrevUserCallbackWindowFocus = NULL;
            bd->PrevUserCallbackMousebutton = NULL;
            bd->PrevUserCallbackScroll = NULL;
            bd->PrevUserCallbackKey = NULL;
            bd->PrevUserCallbackChar = NULL;
            bd->PrevUserCallbackMonitor = NULL;
            if (install_callbacks)
            {
                bd->InstalledCallbacks = true;
                bd->PrevUserCallbackWindowFocus = glfwSetWindowFocusCallback(window, ImGui_ImplGlfw_WindowFocusCallback);
                bd->PrevUserCallbackCursorEnter = glfwSetCursorEnterCallback(window, ImGui_ImplGlfw_CursorEnterCallback);
                bd->PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
                bd->PrevUserCallbackScroll = glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
                bd->PrevUserCallbackKey = glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
                bd->PrevUserCallbackChar = glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
                bd->PrevUserCallbackMonitor = glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);
            }

            // Update monitors the first time (note: monitor callback are broken in GLFW 3.2 and earlier, see github.com/glfw/glfw/issues/784)
            ImGui_ImplGlfw_UpdateMonitors();
            glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);

            // Our mouse update function expect PlatformHandle to be filled for the main viewport
            ImGuiViewport *main_viewport = ImGui::GetMainViewport();
            main_viewport->PlatformHandle = (void *)bd->Window;
#ifdef _WIN32
            main_viewport->PlatformHandleRaw = glfwGetWin32Window(bd->Window);
#endif
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                ImGui_ImplGlfw_InitPlatformInterface();

            bd->ClientApi = client_api;
            return true;
        }

        bool ImGui_ImplGlfw_InitForVulkan(GLFWwindow *window, bool install_callbacks)
        {
            return ImGui_ImplGlfw_Init(window, install_callbacks, GlfwClientApi_Vulkan);
        }

        void ImGui_ImplGlfw_Shutdown()
        {
            ImGuiIO &io = ImGui::GetIO();
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();

            ImGui_ImplGlfw_ShutdownPlatformInterface();

            if (bd->InstalledCallbacks)
            {
                glfwSetWindowFocusCallback(bd->Window, bd->PrevUserCallbackWindowFocus);
                glfwSetCursorEnterCallback(bd->Window, bd->PrevUserCallbackCursorEnter);
                glfwSetMouseButtonCallback(bd->Window, bd->PrevUserCallbackMousebutton);
                glfwSetScrollCallback(bd->Window, bd->PrevUserCallbackScroll);
                glfwSetKeyCallback(bd->Window, bd->PrevUserCallbackKey);
                glfwSetCharCallback(bd->Window, bd->PrevUserCallbackChar);
                glfwSetMonitorCallback(bd->PrevUserCallbackMonitor);
            }

            for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
                glfwDestroyCursor(bd->MouseCursors[cursor_n]);

            io.BackendPlatformName = NULL;
            io.BackendPlatformUserData = NULL;
            IM_DELETE(bd);
        }

        void ImGui_ImplGlfw_UpdateMousePosAndButtons()
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            ImGuiIO &io = ImGui::GetIO();
            ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();

            const ImVec2 mouse_pos_prev = io.MousePos;
            io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
            io.MouseHoveredViewport = 0;

            // Update mouse buttons
            // (if a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame)
            for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
            {
                io.MouseDown[i] = bd->MouseJustPressed[i] || glfwGetMouseButton(bd->Window, i) != 0;
                bd->MouseJustPressed[i] = false;
            }

            for (int n = 0; n < platform_io.Viewports.Size; n++)
            {
                ImGuiViewport *viewport = platform_io.Viewports[n];
                GLFWwindow *window = (GLFWwindow *)viewport->PlatformHandle;

#ifdef __EMSCRIPTEN__
                const bool focused = true;
#else
                const bool focused = glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0;
#endif
                GLFWwindow *mouse_window = (bd->MouseWindow == window || focused) ? window : NULL;

                // Update mouse buttons
                if (focused)
                    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
                        io.MouseDown[i] |= glfwGetMouseButton(window, i) != 0;

                // Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
                // (When multi-viewports are enabled, all Dear ImGui positions are same as OS positions)
                if (io.WantSetMousePos && focused)
                    glfwSetCursorPos(window, (double)(mouse_pos_prev.x - viewport->Pos.x), (double)(mouse_pos_prev.y - viewport->Pos.y));

                // Set Dear ImGui mouse position from OS position
                if (mouse_window != NULL)
                {
                    double mouse_x, mouse_y;
                    glfwGetCursorPos(mouse_window, &mouse_x, &mouse_y);
                    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                    {
                        // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
                        int window_x, window_y;
                        glfwGetWindowPos(window, &window_x, &window_y);
                        io.MousePos = ImVec2((float)mouse_x + window_x, (float)mouse_y + window_y);
                    }
                    else
                    {
                        // Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window)
                        io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
                    }
                }

                // (Optional) When using multiple viewports: set io.MouseHoveredViewport to the viewport the OS mouse cursor is hovering.
                // Important: this information is not easy to provide and many high-level windowing library won't be able to provide it correctly, because
                // - This is _ignoring_ viewports with the ImGuiViewportFlags_NoInputs flag (pass-through windows).
                // - This is _regardless_ of whether another viewport is focused or being dragged from.
                // If ImGuiBackendFlags_HasMouseHoveredViewport is not set by the backend, imgui will ignore this field and infer the information by relying on the
                // rectangles and last focused time of every viewports it knows about. It will be unaware of other windows that may be sitting between or over your windows.
                // [GLFW] FIXME: This is currently only correct on Win32. See what we do below with the WM_NCHITTEST, missing an equivalent for other systems.
                // See https://github.com/glfw/glfw/issues/1236 if you want to help in making this a GLFW feature.
#if GLFW_HAS_MOUSE_PASSTHROUGH || (GLFW_HAS_WINDOW_HOVERED && defined(_WIN32))
                const bool window_no_input = (viewport->Flags & ImGuiViewportFlags_NoInputs) != 0;
#if GLFW_HAS_MOUSE_PASSTHROUGH
                glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, window_no_input);
#endif
                if (glfwGetWindowAttrib(window, GLFW_HOVERED) && !window_no_input)
                    io.MouseHoveredViewport = viewport->ID;
#endif
            }
        }

        void ImGui_ImplGlfw_UpdateMouseCursor()
        {
            ImGuiIO &io = ImGui::GetIO();
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(bd->Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
                return;

            ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
            ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
            for (int n = 0; n < platform_io.Viewports.Size; n++)
            {
                GLFWwindow *window = (GLFWwindow *)platform_io.Viewports[n]->PlatformHandle;
                if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
                {
                    // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
                }
                else
                {
                    // Show OS mouse cursor
                    // FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
                    glfwSetCursor(window, bd->MouseCursors[imgui_cursor] ? bd->MouseCursors[imgui_cursor] : bd->MouseCursors[ImGuiMouseCursor_Arrow]);
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }
        }

        void ImGui_ImplGlfw_UpdateGamepads()
        {
            ImGuiIO &io = ImGui::GetIO();
            memset(io.NavInputs, 0, sizeof(io.NavInputs));
            if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
                return;

// Update gamepad inputs
#define MAP_BUTTON(NAV_NO, BUTTON_NO)                                      \
    {                                                                      \
        if (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS) \
            io.NavInputs[NAV_NO] = 1.0f;                                   \
    }
#define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1)                    \
    {                                                          \
        float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; \
        v = (v - V0) / (V1 - V0);                              \
        if (v > 1.0f)                                          \
            v = 1.0f;                                          \
        if (io.NavInputs[NAV_NO] < v)                          \
            io.NavInputs[NAV_NO] = v;                          \
    }
            int axes_count = 0, buttons_count = 0;
            const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
            const unsigned char *buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
            MAP_BUTTON(ImGuiNavInput_Activate, 0);   // Cross / A
            MAP_BUTTON(ImGuiNavInput_Cancel, 1);     // Circle / B
            MAP_BUTTON(ImGuiNavInput_Menu, 2);       // Square / X
            MAP_BUTTON(ImGuiNavInput_Input, 3);      // Triangle / Y
            MAP_BUTTON(ImGuiNavInput_DpadLeft, 13);  // D-Pad Left
            MAP_BUTTON(ImGuiNavInput_DpadRight, 11); // D-Pad Right
            MAP_BUTTON(ImGuiNavInput_DpadUp, 10);    // D-Pad Up
            MAP_BUTTON(ImGuiNavInput_DpadDown, 12);  // D-Pad Down
            MAP_BUTTON(ImGuiNavInput_FocusPrev, 4);  // L1 / LB
            MAP_BUTTON(ImGuiNavInput_FocusNext, 5);  // R1 / RB
            MAP_BUTTON(ImGuiNavInput_TweakSlow, 4);  // L1 / LB
            MAP_BUTTON(ImGuiNavInput_TweakFast, 5);  // R1 / RB
            MAP_ANALOG(ImGuiNavInput_LStickLeft, 0, -0.3f, -0.9f);
            MAP_ANALOG(ImGuiNavInput_LStickRight, 0, +0.3f, +0.9f);
            MAP_ANALOG(ImGuiNavInput_LStickUp, 1, +0.3f, +0.9f);
            MAP_ANALOG(ImGuiNavInput_LStickDown, 1, -0.3f, -0.9f);
#undef MAP_BUTTON
#undef MAP_ANALOG
            if (axes_count > 0 && buttons_count > 0)
                io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
            else
                io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
        }

        void ImGui_ImplGlfw_UpdateMonitors()
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
            int monitors_count = 0;
            GLFWmonitor **glfw_monitors = glfwGetMonitors(&monitors_count);
            platform_io.Monitors.resize(0);
            for (int n = 0; n < monitors_count; n++)
            {
                ImGuiPlatformMonitor monitor;
                int x, y;
                glfwGetMonitorPos(glfw_monitors[n], &x, &y);
                const GLFWvidmode *vid_mode = glfwGetVideoMode(glfw_monitors[n]);
                monitor.MainPos = monitor.WorkPos = ImVec2((float)x, (float)y);
                monitor.MainSize = monitor.WorkSize = ImVec2((float)vid_mode->width, (float)vid_mode->height);
#if GLFW_HAS_MONITOR_WORK_AREA
                int w, h;
                glfwGetMonitorWorkarea(glfw_monitors[n], &x, &y, &w, &h);
                if (w > 0 && h > 0) // Workaround a small GLFW issue reporting zero on monitor changes: https://github.com/glfw/glfw/pull/1761
                {
                    monitor.WorkPos = ImVec2((float)x, (float)y);
                    monitor.WorkSize = ImVec2((float)w, (float)h);
                }
#endif
#if GLFW_HAS_PER_MONITOR_DPI
                // Warning: the validity of monitor DPI information on Windows depends on the application DPI awareness settings, which generally needs to be set in the manifest or at runtime.
                float x_scale, y_scale;
                glfwGetMonitorContentScale(glfw_monitors[n], &x_scale, &y_scale);
                monitor.DpiScale = x_scale;
#endif
                platform_io.Monitors.push_back(monitor);
            }
            bd->WantUpdateMonitors = false;
        }

        void ImGui_ImplGlfw_NewFrame()
        {
            ImGuiIO &io = ImGui::GetIO();
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            IM_ASSERT(bd != NULL && "Did you call ImGui_ImplGlfw_InitForXXX()?");

            // Setup display size (every frame to accommodate for window resizing)
            int w, h;
            int display_w, display_h;
            glfwGetWindowSize(bd->Window, &w, &h);
            glfwGetFramebufferSize(bd->Window, &display_w, &display_h);
            io.DisplaySize = ImVec2((float)w, (float)h);
            if (w > 0 && h > 0)
                io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);
            if (bd->WantUpdateMonitors)
                ImGui_ImplGlfw_UpdateMonitors();

            // Setup time step
            double current_time = glfwGetTime();
            io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
            bd->Time = current_time;

            ImGui_ImplGlfw_UpdateMousePosAndButtons();
            ImGui_ImplGlfw_UpdateMouseCursor();

            // Update game controllers (if enabled and available)
            ImGui_ImplGlfw_UpdateGamepads();
        }

        //--------------------------------------------------------------------------------------------------------
        // MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
        // This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
        // If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
        //--------------------------------------------------------------------------------------------------------

        // Helper structure we store in the void* RenderUserData field of each ImGuiViewport to easily retrieve our backend data.
        struct ImGui_ImplGlfw_ViewportData
        {
            GLFWwindow *Window;
            bool WindowOwned;
            int IgnoreWindowPosEventFrame;
            int IgnoreWindowSizeEventFrame;

            ImGui_ImplGlfw_ViewportData()
            {
                Window = NULL;
                WindowOwned = false;
                IgnoreWindowSizeEventFrame = IgnoreWindowPosEventFrame = -1;
            }
            ~ImGui_ImplGlfw_ViewportData() { IM_ASSERT(Window == NULL); }
        };

        void ImGui_ImplGlfw_WindowCloseCallback(GLFWwindow *window)
        {
            if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(window))
                viewport->PlatformRequestClose = true;
        }

        // GLFW may dispatch window pos/size events after calling glfwSetWindowPos()/glfwSetWindowSize().
        // However: depending on the platform the callback may be invoked at different time:
        // - on Windows it appears to be called within the glfwSetWindowPos()/glfwSetWindowSize() call
        // - on Linux it is queued and invoked during glfwPollEvents()
        // Because the event doesn't always fire on glfwSetWindowXXX() we use a frame counter tag to only
        // ignore recent glfwSetWindowXXX() calls.
        void ImGui_ImplGlfw_WindowPosCallback(GLFWwindow *window, int, int)
        {
            if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(window))
            {
                if (ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData)
                {
                    bool ignore_event = (ImGui::GetFrameCount() <= vd->IgnoreWindowPosEventFrame + 1);
                    // data->IgnoreWindowPosEventFrame = -1;
                    if (ignore_event)
                        return;
                }
                viewport->PlatformRequestMove = true;
            }
        }

        void ImGui_ImplGlfw_WindowSizeCallback(GLFWwindow *window, int, int)
        {
            if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(window))
            {
                if (ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData)
                {
                    bool ignore_event = (ImGui::GetFrameCount() <= vd->IgnoreWindowSizeEventFrame + 1);
                    // data->IgnoreWindowSizeEventFrame = -1;
                    if (ignore_event)
                        return;
                }
                viewport->PlatformRequestResize = true;
            }
        }

        void ImGui_ImplGlfw_CreateWindow(ImGuiViewport *viewport)
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            ImGui_ImplGlfw_ViewportData *vd = IM_NEW(ImGui_ImplGlfw_ViewportData)();
            viewport->PlatformUserData = vd;

            // GLFW 3.2 unfortunately always set focus on glfwCreateWindow() if GLFW_VISIBLE is set, regardless of GLFW_FOCUSED
            // With GLFW 3.3, the hint GLFW_FOCUS_ON_SHOW fixes this problem
            glfwWindowHint(GLFW_VISIBLE, false);
            glfwWindowHint(GLFW_FOCUSED, false);
#if GLFW_HAS_FOCUS_ON_SHOW
            glfwWindowHint(GLFW_FOCUS_ON_SHOW, false);
#endif
            glfwWindowHint(GLFW_DECORATED, (viewport->Flags & ImGuiViewportFlags_NoDecoration) ? false : true);
#if GLFW_HAS_WINDOW_TOPMOST
            glfwWindowHint(GLFW_FLOATING, (viewport->Flags & ImGuiViewportFlags_TopMost) ? true : false);
#endif
            GLFWwindow *share_window = (bd->ClientApi == GlfwClientApi_OpenGL) ? bd->Window : NULL;
            vd->Window = glfwCreateWindow((int)viewport->Size.x, (int)viewport->Size.y, "No Title Yet", NULL, share_window);
            vd->WindowOwned = true;
            viewport->PlatformHandle = (void *)vd->Window;
#ifdef _WIN32
            viewport->PlatformHandleRaw = glfwGetWin32Window(vd->Window);
#endif
            glfwSetWindowPos(vd->Window, (int)viewport->Pos.x, (int)viewport->Pos.y);

            // Install GLFW callbacks for secondary viewports
            glfwSetWindowFocusCallback(vd->Window, ImGui_ImplGlfw_WindowFocusCallback);
            glfwSetCursorEnterCallback(vd->Window, ImGui_ImplGlfw_CursorEnterCallback);
            glfwSetMouseButtonCallback(vd->Window, ImGui_ImplGlfw_MouseButtonCallback);
            glfwSetScrollCallback(vd->Window, ImGui_ImplGlfw_ScrollCallback);
            glfwSetKeyCallback(vd->Window, ImGui_ImplGlfw_KeyCallback);
            glfwSetCharCallback(vd->Window, ImGui_ImplGlfw_CharCallback);
            glfwSetWindowCloseCallback(vd->Window, ImGui_ImplGlfw_WindowCloseCallback);
            glfwSetWindowPosCallback(vd->Window, ImGui_ImplGlfw_WindowPosCallback);
            glfwSetWindowSizeCallback(vd->Window, ImGui_ImplGlfw_WindowSizeCallback);
            if (bd->ClientApi == GlfwClientApi_OpenGL)
            {
                glfwMakeContextCurrent(vd->Window);
                glfwSwapInterval(0);
            }
        }

        void ImGui_ImplGlfw_DestroyWindow(ImGuiViewport *viewport)
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            if (ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData)
            {
                if (vd->WindowOwned)
                {
#if !GLFW_HAS_MOUSE_PASSTHROUGH && GLFW_HAS_WINDOW_HOVERED && defined(_WIN32)
                    HWND hwnd = (HWND)viewport->PlatformHandleRaw;
                    ::RemovePropA(hwnd, "IMGUI_VIEWPORT");
#endif

                    // Release any keys that were pressed in the window being destroyed and are still held down,
                    // because we will not receive any release events after window is destroyed.
                    for (int i = 0; i < IM_ARRAYSIZE(bd->KeyOwnerWindows); i++)
                        if (bd->KeyOwnerWindows[i] == vd->Window)
                            ImGui_ImplGlfw_KeyCallback(vd->Window, i, 0, GLFW_RELEASE, 0); // Later params are only used for main viewport, on which this function is never called.

                    glfwDestroyWindow(vd->Window);
                }
                vd->Window = NULL;
                IM_DELETE(vd);
            }
            viewport->PlatformUserData = viewport->PlatformHandle = NULL;
        }

// We have submitted https://github.com/glfw/glfw/pull/1568 to allow GLFW to support "transparent inputs".
// In the meanwhile we implement custom per-platform workarounds here (FIXME-VIEWPORT: Implement same work-around for Linux/OSX!)
#if !GLFW_HAS_MOUSE_PASSTHROUGH && GLFW_HAS_WINDOW_HOVERED && defined(_WIN32)
        WNDPROC g_GlfwWndProc = NULL;
        LRESULT CALLBACK WndProcNoInputs(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            if (msg == WM_NCHITTEST)
            {
                // Let mouse pass-through the window. This will allow the backend to set io.MouseHoveredViewport properly (which is OPTIONAL).
                // The ImGuiViewportFlags_NoInputs flag is set while dragging a viewport, as want to detect the window behind the one we are dragging.
                // If you cannot easily access those viewport flags from your windowing/event code: you may manually synchronize its state e.g. in
                // your main loop after calling UpdatePlatformWindows(). Iterate all viewports/platform windows and pass the flag to your windowing system.
                ImGuiViewport *viewport = (ImGuiViewport *)::GetPropA(hWnd, "IMGUI_VIEWPORT");
                if (viewport->Flags & ImGuiViewportFlags_NoInputs)
                    return HTTRANSPARENT;
            }
            return ::CallWindowProc(g_GlfwWndProc, hWnd, msg, wParam, lParam);
        }
#endif

        void ImGui_ImplGlfw_ShowWindow(ImGuiViewport *viewport)
        {
            ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData;

#if defined(_WIN32)
            // GLFW hack: Hide icon from task bar
            HWND hwnd = (HWND)viewport->PlatformHandleRaw;
            if (viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon)
            {
                LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
                ex_style &= ~WS_EX_APPWINDOW;
                ex_style |= WS_EX_TOOLWINDOW;
                ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
            }

            // GLFW hack: install hook for WM_NCHITTEST message handler
#if !GLFW_HAS_MOUSE_PASSTHROUGH && GLFW_HAS_WINDOW_HOVERED && defined(_WIN32)
            ::SetPropA(hwnd, "IMGUI_VIEWPORT", viewport);
            if (g_GlfwWndProc == NULL)
                g_GlfwWndProc = (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
            ::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProcNoInputs);
#endif

#if !GLFW_HAS_FOCUS_ON_SHOW
            // GLFW hack: GLFW 3.2 has a bug where glfwShowWindow() also activates/focus the window.
            // The fix was pushed to GLFW repository on 2018/01/09 and should be included in GLFW 3.3 via a GLFW_FOCUS_ON_SHOW window attribute.
            // See https://github.com/glfw/glfw/issues/1189
            // FIXME-VIEWPORT: Implement same work-around for Linux/OSX in the meanwhile.
            if (viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing)
            {
                ::ShowWindow(hwnd, SW_SHOWNA);
                return;
            }
#endif
#endif

            glfwShowWindow(vd->Window);
        }

        ImVec2 ImGui_ImplGlfw_GetWindowPos(ImGuiViewport *viewport)
        {
            ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData;
            int x = 0, y = 0;
            glfwGetWindowPos(vd->Window, &x, &y);
            return ImVec2((float)x, (float)y);
        }

        void ImGui_ImplGlfw_SetWindowPos(ImGuiViewport *viewport, ImVec2 pos)
        {
            ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData;
            vd->IgnoreWindowPosEventFrame = ImGui::GetFrameCount();
            glfwSetWindowPos(vd->Window, (int)pos.x, (int)pos.y);
        }

        ImVec2 ImGui_ImplGlfw_GetWindowSize(ImGuiViewport *viewport)
        {
            ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData;
            int w = 0, h = 0;
            glfwGetWindowSize(vd->Window, &w, &h);
            return ImVec2((float)w, (float)h);
        }

        void ImGui_ImplGlfw_SetWindowSize(ImGuiViewport *viewport, ImVec2 size)
        {
            ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData;
#if __APPLE__ && !GLFW_HAS_OSX_WINDOW_POS_FIX
            // Native OS windows are positioned from the bottom-left corner on macOS, whereas on other platforms they are
            // positioned from the upper-left corner. GLFW makes an effort to convert macOS style coordinates, however it
            // doesn't handle it when changing size. We are manually moving the window in order for changes of size to be based
            // on the upper-left corner.
            int x, y, width, height;
            glfwGetWindowPos(vd->Window, &x, &y);
            glfwGetWindowSize(vd->Window, &width, &height);
            glfwSetWindowPos(vd->Window, x, y - height + size.y);
#endif
            vd->IgnoreWindowSizeEventFrame = ImGui::GetFrameCount();
            glfwSetWindowSize(vd->Window, (int)size.x, (int)size.y);
        }

        void ImGui_ImplGlfw_SetWindowTitle(ImGuiViewport *viewport, const char *title)
        {
            ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData;
            glfwSetWindowTitle(vd->Window, title);
        }

        void ImGui_ImplGlfw_SetWindowFocus(ImGuiViewport *viewport)
        {
#if GLFW_HAS_FOCUS_WINDOW
            ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData;
            glfwFocusWindow(vd->Window);
#else
            // FIXME: What are the effect of not having this function? At the moment imgui doesn't actually call SetWindowFocus - we set that up ahead, will answer that question later.
            (void)viewport;
#endif
        }

        bool ImGui_ImplGlfw_GetWindowFocus(ImGuiViewport *viewport)
        {
            ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData;
            return glfwGetWindowAttrib(vd->Window, GLFW_FOCUSED) != 0;
        }

        bool ImGui_ImplGlfw_GetWindowMinimized(ImGuiViewport *viewport)
        {
            ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData;
            return glfwGetWindowAttrib(vd->Window, GLFW_ICONIFIED) != 0;
        }

#if GLFW_HAS_WINDOW_ALPHA
        void ImGui_ImplGlfw_SetWindowAlpha(ImGuiViewport *viewport, float alpha)
        {
            ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData;
            glfwSetWindowOpacity(vd->Window, alpha);
        }
#endif

        void ImGui_ImplGlfw_RenderWindow(ImGuiViewport *viewport, void *)
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData;
            if (bd->ClientApi == GlfwClientApi_OpenGL)
                glfwMakeContextCurrent(vd->Window);
        }

        void ImGui_ImplGlfw_SwapBuffers(ImGuiViewport *viewport, void *)
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData;
            if (bd->ClientApi == GlfwClientApi_OpenGL)
            {
                glfwMakeContextCurrent(vd->Window);
                glfwSwapBuffers(vd->Window);
            }
        }

//--------------------------------------------------------------------------------------------------------
// IME (Input Method Editor) basic support for e.g. Asian language users
//--------------------------------------------------------------------------------------------------------

// We provide a Win32 implementation because this is such a common issue for IME users
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS)
#define HAS_WIN32_IME 1
#include <imm.h>
#ifdef _MSC_VER
#pragma comment(lib, "imm32")
#endif
        void ImGui_ImplWin32_SetImeInputPos(ImGuiViewport *viewport, ImVec2 pos)
        {
            COMPOSITIONFORM cf = {CFS_FORCE_POSITION, {(LONG)(pos.x - viewport->Pos.x), (LONG)(pos.y - viewport->Pos.y)}, {0, 0, 0, 0}};
            if (HWND hwnd = (HWND)viewport->PlatformHandleRaw)
                if (HIMC himc = ::ImmGetContext(hwnd))
                {
                    ::ImmSetCompositionWindow(himc, &cf);
                    ::ImmReleaseContext(hwnd, himc);
                }
        }
#else
#define HAS_WIN32_IME 0
#endif

//--------------------------------------------------------------------------------------------------------
// Vulkan support (the Vulkan renderer needs to call a platform-side support function to create the surface)
//--------------------------------------------------------------------------------------------------------

// Avoid including <vulkan.h> so we can build without it
#if GLFW_HAS_VULKAN
#ifndef VULKAN_H_
#define VK_DEFINE_HANDLE(object) typedef struct object##_T *object;
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif
        VK_DEFINE_HANDLE(VkInstance)
        VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
        struct VkAllocationCallbacks;
        enum VkResult
        {
            VK_RESULT_MAX_ENUM = 0x7FFFFFFF
        };
#endif // VULKAN_H_
        /*         extern "C"
        {
            extern GLFWAPI VkResult glfwCreateWindowSurface(VkInstance instance, GLFWwindow *window, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);
        } */
        int ImGui_ImplGlfw_CreateVkSurface(ImGuiViewport *viewport, ImU64 vk_instance, const void *vk_allocator, ImU64 *out_vk_surface)
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            ImGui_ImplGlfw_ViewportData *vd = (ImGui_ImplGlfw_ViewportData *)viewport->PlatformUserData;
            IM_UNUSED(bd);
            IM_ASSERT(bd->ClientApi == GlfwClientApi_Vulkan);
            VkResult err = glfwCreateWindowSurface((VkInstance)vk_instance, vd->Window, (const VkAllocationCallbacks *)vk_allocator, (VkSurfaceKHR *)out_vk_surface);
            return (int)err;
        }
#endif // GLFW_HAS_VULKAN

        void ImGui_ImplGlfw_InitPlatformInterface()
        {
            // Register platform interface (will be coupled with a renderer interface)
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
            ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
            platform_io.Platform_CreateWindow = ImGui_ImplGlfw_CreateWindow;
            platform_io.Platform_DestroyWindow = ImGui_ImplGlfw_DestroyWindow;
            platform_io.Platform_ShowWindow = ImGui_ImplGlfw_ShowWindow;
            platform_io.Platform_SetWindowPos = ImGui_ImplGlfw_SetWindowPos;
            platform_io.Platform_GetWindowPos = ImGui_ImplGlfw_GetWindowPos;
            platform_io.Platform_SetWindowSize = ImGui_ImplGlfw_SetWindowSize;
            platform_io.Platform_GetWindowSize = ImGui_ImplGlfw_GetWindowSize;
            platform_io.Platform_SetWindowFocus = ImGui_ImplGlfw_SetWindowFocus;
            platform_io.Platform_GetWindowFocus = ImGui_ImplGlfw_GetWindowFocus;
            platform_io.Platform_GetWindowMinimized = ImGui_ImplGlfw_GetWindowMinimized;
            platform_io.Platform_SetWindowTitle = ImGui_ImplGlfw_SetWindowTitle;
            platform_io.Platform_RenderWindow = ImGui_ImplGlfw_RenderWindow;
            platform_io.Platform_SwapBuffers = ImGui_ImplGlfw_SwapBuffers;
#if GLFW_HAS_WINDOW_ALPHA
            platform_io.Platform_SetWindowAlpha = ImGui_ImplGlfw_SetWindowAlpha;
#endif
#if GLFW_HAS_VULKAN
            platform_io.Platform_CreateVkSurface = ImGui_ImplGlfw_CreateVkSurface;
#endif
#if HAS_WIN32_IME
            platform_io.Platform_SetImeInputPos = ImGui_ImplWin32_SetImeInputPos;
#endif

            // Register main window handle (which is owned by the main application, not by us)
            // This is mostly for simplicity and consistency, so that our code (e.g. mouse handling etc.) can use same logic for main and secondary viewports.
            ImGuiViewport *main_viewport = ImGui::GetMainViewport();
            ImGui_ImplGlfw_ViewportData *vd = IM_NEW(ImGui_ImplGlfw_ViewportData)();
            vd->Window = bd->Window;
            vd->WindowOwned = false;
            main_viewport->PlatformUserData = vd;
            main_viewport->PlatformHandle = (void *)bd->Window;
        }

        void ImGui_ImplGlfw_ShutdownPlatformInterface()
        {
        }

        void ImGuiCreateWindow(ImGuiViewport *viewport)
        {
            ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
        }

        void ImGuiDestroyWindow(ImGuiViewport *viewport)
        {
        }
    }

    const std::string Gui::default_font = "Roboto-Medium";
    int32_t GuiViewport::s_Counter = 0;

    Gui::Gui(Application *application,
             Window *window,
             const float font_size,
             bool explicit_update)
        : Layer(application, "gui"), m_Application(*application), m_ExplicitUpdate(explicit_update),
          m_Window(*window)
    {
        SetWindow(window);
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();

        auto &extent = m_Window.GetRenderContext().GetSurfaceExtent();
        io.DisplaySize.x = static_cast<float>(extent.width);
        io.DisplaySize.y = static_cast<float>(extent.height);
        io.FontGlobalScale = 1.0f;
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplVulkan_Data *bd = IM_NEW(ImGui_ImplVulkan_Data)();
        io.BackendRendererUserData = (void *)bd;
        io.BackendRendererName = "imgui_impl_vulkan";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
        io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports; // We can create multi-viewports on the Renderer side (optional)

        io.KeyMap[ImGuiKey_Space] = static_cast<int>(Key::Space);
        io.KeyMap[ImGuiKey_Enter] = static_cast<int>(Key::Enter);
        io.KeyMap[ImGuiKey_LeftArrow] = static_cast<int>(Key::Left);
        io.KeyMap[ImGuiKey_RightArrow] = static_cast<int>(Key::Right);
        io.KeyMap[ImGuiKey_UpArrow] = static_cast<int>(Key::Up);
        io.KeyMap[ImGuiKey_DownArrow] = static_cast<int>(Key::Down);
        io.KeyMap[ImGuiKey_Tab] = static_cast<int>(Key::Tab);

        ImGui::StyleColorsDark();
        m_Fonts.emplace_back(default_font, font_size);

        unsigned char *font_data;
        int tex_width, tex_height;
        io.Fonts->GetTexDataAsRGBA32(&font_data, &tex_width, &tex_height);
        size_t upload_size = tex_width * tex_height * 4 * sizeof(char);

        auto &device = m_Window.GetRenderContext().GetDevice();

        // Create target image for copy
        VkExtent3D font_extent{ToUint32_t(tex_width), ToUint32_t(tex_height), 1u};
        m_FontImage = std::make_unique<core::Image>(device, font_extent, VK_FORMAT_R8G8B8A8_UNORM,
                                                    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                    VMA_MEMORY_USAGE_GPU_ONLY);
        m_FontImageView = std::make_unique<core::ImageView>(*m_FontImage, VK_IMAGE_VIEW_TYPE_2D);

        {
            core::Buffer stage_buffer{device, upload_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, 0};
            stage_buffer.Update({font_data, font_data + upload_size});

            auto &command_buffer = device.RequestCommandBuffer();

            FencePool fence_pool{device};

            // Begin recording
            command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, 0);

            {
                // Prepare for transfer
                ImageMemoryBarrier memory_barrier{};
                memory_barrier.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
                memory_barrier.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                memory_barrier.src_access_mask = 0;
                memory_barrier.dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
                memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_HOST_BIT;
                memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;

                command_buffer.CreateImageMemoryBarrier(*m_FontImageView, memory_barrier);
            }

            // Copy
            VkBufferImageCopy buffer_copy_region{};
            buffer_copy_region.imageSubresource.layerCount = m_FontImageView->GetSubresourceRange().layerCount;
            buffer_copy_region.imageSubresource.aspectMask = m_FontImageView->GetSubresourceRange().aspectMask;
            buffer_copy_region.imageExtent = m_FontImage->GetExtent();

            command_buffer.CopyBufferToImage(stage_buffer, *m_FontImage, {buffer_copy_region});

            {
                // Prepare for fragmen shader
                ImageMemoryBarrier memory_barrier{};
                memory_barrier.old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                memory_barrier.new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                memory_barrier.src_access_mask = 0;
                memory_barrier.dst_access_mask = VK_ACCESS_SHADER_READ_BIT;
                memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

                command_buffer.CreateImageMemoryBarrier(*m_FontImageView, memory_barrier);
            }

            // End recording
            command_buffer.End();

            auto &queue = device.GetQueueFamilyByFlags(VK_QUEUE_GRAPHICS_BIT).GetQueues()[0];

            queue.Submit(command_buffer, device.RequestFence());

            // Wait for the command buffer to finish its work before destroying the staging buffer
            device.GetFencePool().Wait();
            device.GetFencePool().Reset();
            device.GetCommandPool().ResetPool();
        }

        // Create texture sampler
        VkSamplerCreateInfo sampler_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        sampler_info.maxAnisotropy = 1.0f;
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        ShaderSource vert_shader("imgui.vert");
        ShaderSource frag_shader("imgui.frag");

        /* SetScene(m_Application.LoadScene());
        Scene *scene = GetScene();

        auto scene_subpass = std::make_unique<GuiSubpass>(std::move(vert_shader),
                                                          std::move(frag_shader),
                                                          *scene, *this);

        auto render_pipeline = std::make_unique<RenderPipeline>(device);
        render_pipeline->AddSubpass(std::move(scene_subpass));
        scene->SetRenderPipeline(std::move(render_pipeline));

        auto &clear_color = scene->GetRenderPipeline()->GetClearColor();
        clear_color[0].color = {0.0f, 0.0f, 0.0f, 0.0f};
        scene->GetRenderPipeline()->SetClearColor(clear_color);

        auto &load_store = scene->GetRenderPipeline()->GetLoadStore();
        load_store[0].load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
        scene->GetRenderPipeline()->SetLoadStore(load_store); */

        std::vector<ShaderModule *> shader_modules;
        shader_modules.push_back(&device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_VERTEX_BIT, vert_shader, {}));
        shader_modules.push_back(&device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader, {}));

        m_PipelineLayout = &device.GetResourceCache().RequestPipelineLayout(shader_modules);

        m_Sampler = std::make_unique<core::Sampler>(device, sampler_info);

        if (explicit_update)
        {
            m_VertexBuffer = std::make_unique<core::Buffer>(m_Window.GetRenderContext().GetDevice(), 1, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_TO_CPU);
            m_IndexBuffer = std::make_unique<core::Buffer>(m_Window.GetRenderContext().GetDevice(), 1, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_TO_CPU);
        }

        // auto glfw_window = reinterpret_cast<GLFWwindow *>(m_Window.GetNativeWindow());
        // ImGui_ImplGlfw_InitForVulkan(glfw_window, true);
        ImGuiInitGlfw();
    }

    Gui::~Gui()
    {
        auto &device = m_Window.GetRenderContext().GetDevice();
        device.WaitIdle();
        vkDestroyDescriptorPool(device.GetHandle(), m_DescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device.GetHandle(), m_DescriptorSetLayout, nullptr);
        vkDestroyPipeline(device.GetHandle(), m_Pipeline, nullptr);

        ImGui::DestroyContext();
    }

    GuiViewport::GuiViewport(Application *application, Gui &gui, /* RenderPipeline *render_pipeline */ ImGuiViewport *viewport)
        : Layer(application, std::string("Gui ", s_Counter++)),
          m_Viewport(viewport), m_Gui(gui) /* , m_RenderPipeline(render_pipeline) */
    {
    }

    void GuiViewport::OnAttach()
    {
        WindowSettings settings;
        settings.width = static_cast<int32_t>(m_Viewport->Size.x);
        settings.height = static_cast<int32_t>(m_Viewport->Size.y);
        settings.posx = static_cast<int32_t>(m_Viewport->Pos.x);
        settings.posy = static_cast<int32_t>(m_Viewport->Pos.y);
        settings.decorated = (m_Viewport->Flags & ImGuiViewportFlags_NoDecoration) ? false : true;

        Window *window = GetApp().GetPlatform().CreatePlatformWindow(settings);
        SetWindow(window);

        std::vector<VkPresentModeKHR> present_mode_priority({VK_PRESENT_MODE_FIFO_KHR,
                                                             VK_PRESENT_MODE_IMMEDIATE_KHR,
                                                             VK_PRESENT_MODE_MAILBOX_KHR});

        std::vector<VkSurfaceFormatKHR> surface_format_priority({{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                                 {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                                 {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                                 {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}});

        m_Viewport->PlatformUserData = window;
        m_Viewport->PlatformHandle = static_cast<void *>(window->GetNativeWindow());
#ifdef VK_USE_PLATFORM_WIN32_KHR
        m_Viewport->PlatformHandleRaw = glfwGetWin32Window(static_cast<GLFWwindow *>(window->GetNativeWindow()));
#endif

        window->CreateSurface(GetApp().GetInstance(), GetApp().GetDevice().GetGPU());
        window->CreateRenderContext(GetApp().GetDevice(), present_mode_priority, surface_format_priority);
        window->GetRenderContext().Prepare();

        /* ShaderSource vert_shader("imgui.vert");
        ShaderSource frag_shader("imgui.frag");

        SetScene(GetApp().LoadScene());
        Scene *scene = GetScene();

        auto scene_subpass = std::make_unique<GuiSubpass>(std::move(vert_shader),
                                                          std::move(frag_shader),
                                                          *scene, m_Gui);

        auto render_pipeline = std::make_unique<RenderPipeline>(GetApp().GetDevice());
        render_pipeline->AddSubpass(std::move(scene_subpass));
        scene->SetRenderPipeline(std::move(render_pipeline)); */
    }

    void Gui::ImGuiCreateWindow(ImGuiViewport *viewport)
    {
        m_Application.GetLayerStack().PushLayer(std::make_shared<GuiViewport>(&m_Application, *this, /* GetScene()->GetRenderPipeline(), */ viewport));
    }

    void Gui::ImGuiDestroyWindow(ImGuiViewport *viewport)
    {
        Window *window = static_cast<Window *>(viewport->PlatformUserData);
        window->Close();
        viewport->PlatformUserData = nullptr;
        viewport->PlatformHandle = nullptr;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        viewport->PlatformHandleRaw = nullptr;
#endif
    }

    void Gui::ImGuiGlfwShowWindow(ImGuiViewport *viewport)
    {
    }

    void Gui::ImGuiGlfwSetWindowPos(ImGuiViewport *viewport, ImVec2)
    {
    }

    ImVec2 Gui::ImGuiGlfwGetWindowPos(ImGuiViewport *viewport)
    {
        return {0, 0};
    }

    void Gui::ImGuiGlfwSetWindowSize(ImGuiViewport *viewport, ImVec2)
    {
    }

    ImVec2 Gui::ImGuiGlfwGetWindowSize(ImGuiViewport *viewport)
    {
        return {1920, 1080};
    }

    void Gui::ImGuiGlfwSetWindowFocus(ImGuiViewport *viewport)
    {
    }

    bool Gui::ImGuiGlfwGetWindowFocus(ImGuiViewport *viewport)
    {
        return true;
    }

    bool Gui::ImGuiGlfwGetWindowMinimized(ImGuiViewport *viewport)
    {
        return false;
    }

    void Gui::ImGuiGlfwSetWindowTitle(ImGuiViewport *viewport, const char *)
    {
    }

    void Gui::ImGuiGlfwRenderWindow(ImGuiViewport *viewport, void *)
    {
    }

    void Gui::ImGuiGlfwSwapBuffers(ImGuiViewport *viewport, void *)
    {
    }

    void Gui::ImGuiGlfwSetWindowAlpha(ImGuiViewport *viewport, float)
    {
    }

    int Gui::ImGuiGlfwCreateVkSurface(ImGuiViewport *vp, ImU64 vk_inst, const void *vk_allocators, ImU64 *out_vk_surface)
    {
        return 0;
    }

    void Gui::ImGuiWin32SetImeInputPos(ImGuiViewport *viewport, ImVec2 position)
    {
    }

    void Gui::ImGuiInitGlfw()
    {
        ImGuiIO &io = ImGui::GetIO();
        IM_ASSERT(io.BackendPlatformUserData == NULL && "Already initialized a platform backend!");
        ImGui_ImplGlfw_Data *bd = IM_NEW(ImGui_ImplGlfw_Data)();
        io.BackendPlatformUserData = (void *)bd;
        io.BackendPlatformName = "imgui_impl_glfw";
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;      // We can honor GetMouseCursor() values (optional)
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;       // We can honor io.WantSetMousePos requests (optional, rarely used)
        io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports; // We can create multi-viewports on the Platform side (optional)
#if GLFW_HAS_MOUSE_PASSTHROUGH || (GLFW_HAS_WINDOW_HOVERED && defined(_WIN32))
        io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can set io.MouseHoveredViewport correctly (optional, not easy)
#endif
        bd->Window = static_cast<GLFWwindow *>(m_Window.GetNativeWindow());
        bd->Time = 0.0;
        bd->WantUpdateMonitors = true;

        ImGuiViewport *main_viewport = ImGui::GetMainViewport();
        main_viewport->PlatformHandle = (void *)bd->Window;
#ifdef _WIN32
        main_viewport->PlatformHandleRaw = glfwGetWin32Window(bd->Window);
#endif

        ImGui_ImplGlfw_ViewportData *vd = IM_NEW(ImGui_ImplGlfw_ViewportData)();
        vd->Window = bd->Window;
        vd->WindowOwned = false;
        main_viewport->PlatformUserData = vd;
        main_viewport->PlatformHandle = (void *)bd->Window;

        ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();

        // https://stackoverflow.com/questions/25732386/what-is-stddecay-and-when-it-should-be-used

        ENG_BIND_C_CALLBACK(platform_io.Platform_CreateWindow, Gui::ImGuiCreateWindow, this)
        ENG_BIND_C_CALLBACK(platform_io.Platform_DestroyWindow, Gui::ImGuiDestroyWindow, this)
        ENG_BIND_C_CALLBACK(platform_io.Platform_ShowWindow, Gui::ImGuiGlfwShowWindow, this)
        ENG_BIND_C_CALLBACK(platform_io.Platform_SetWindowPos, Gui::ImGuiGlfwSetWindowPos, this)
        ENG_BIND_C_CALLBACK(platform_io.Platform_GetWindowPos, Gui::ImGuiGlfwGetWindowPos, this)
        ENG_BIND_C_CALLBACK(platform_io.Platform_SetWindowSize, Gui::ImGuiGlfwSetWindowSize, this)
        ENG_BIND_C_CALLBACK(platform_io.Platform_GetWindowSize, Gui::ImGuiGlfwGetWindowSize, this)
        ENG_BIND_C_CALLBACK(platform_io.Platform_SetWindowFocus, Gui::ImGuiGlfwSetWindowFocus, this)
        ENG_BIND_C_CALLBACK(platform_io.Platform_GetWindowFocus, Gui::ImGuiGlfwGetWindowFocus, this)
        ENG_BIND_C_CALLBACK(platform_io.Platform_GetWindowMinimized, Gui::ImGuiGlfwGetWindowMinimized, this)
        ENG_BIND_C_CALLBACK(platform_io.Platform_SetWindowTitle, Gui::ImGuiGlfwSetWindowTitle, this)
        ENG_BIND_C_CALLBACK(platform_io.Platform_RenderWindow, Gui::ImGuiGlfwRenderWindow, this)
        ENG_BIND_C_CALLBACK(platform_io.Platform_SwapBuffers, Gui::ImGuiGlfwSwapBuffers, this)

#if GLFW_HAS_WINDOW_ALPHA
        ENG_BIND_C_CALLBACK(platform_io.Platform_SetWindowAlpha, Gui::ImGuiGlfwSetWindowAlpha, this)
#endif
#if GLFW_HAS_VULKAN
        ENG_BIND_C_CALLBACK(platform_io.Platform_CreateVkSurface, Gui::ImGuiGlfwCreateVkSurface, this)
#endif
#if 1 // HAS_WIN32_IME
        ENG_BIND_C_CALLBACK(platform_io.Platform_SetImeInputPos, Gui::ImGuiWin32SetImeInputPos, this)
#endif
    }

    void Gui::ImGuiGlfwNewFrame(float delta_time)
    {
        ImGuiIO &io = ImGui::GetIO();
        ImGui_ImplGlfw_Data *bd = ImGui_ImplGlfw_GetBackendData();
        // GLFWwindow *gltf_window = bd->Window;
        double current_time = glfwGetTime();
        io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
        bd->Time = current_time;

        // Setup display size (every frame to accommodate for window resizing)
        /* int w, h;
        int display_w, display_h;
        glfwGetWindowSize(bd->Window, &w, &h);
        glfwGetFramebufferSize(bd->Window, &display_w, &display_h);
        io.DisplaySize = ImVec2((float)w, (float)h);
        if (w > 0 && h > 0)
            io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);
        if (bd->WantUpdateMonitors)
            ImGui_ImplGlfw_UpdateMonitors();

        // Setup time step
        double current_time = glfwGetTime();
        io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
        bd->Time = current_time;

        ImGui_ImplGlfw_UpdateMousePosAndButtons();
        ImGui_ImplGlfw_UpdateMouseCursor();

        // Update game controllers (if enabled and available)
        ImGui_ImplGlfw_UpdateGamepads(); */
    }

    void Gui::NewFrame(float delta_time)
    {
        ImGui_ImplVulkan_Data *bd = ImGui::GetCurrentContext() ? (ImGui_ImplVulkan_Data *)ImGui::GetIO().BackendRendererUserData : NULL;
        ENG_ASSERT(bd != NULL);
        ImGui_ImplGlfw_NewFrame();
        // ImGuiGlfwNewFrame(delta_time);
        ImGui::NewFrame();
    }

    void Gui::OnUpdate(float delta_time)
    {
        ImGuiIO &io = ImGui::GetIO();
        NewFrame(delta_time);
        ImGui::ShowDemoWindow();
        auto extent = m_Window.GetRenderContext().GetSurfaceExtent();
        Resize(extent.width, extent.height);
        io.DeltaTime = delta_time;
        ImGui::Render();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            ImGui::UpdatePlatformWindows();
    }

    void Gui::Resize(const uint32_t width, const uint32_t height) const
    {
        auto &io = ImGui::GetIO();
        io.DisplaySize.x = static_cast<float>(width);
        io.DisplaySize.y = static_cast<float>(height);
    }

    void Gui::Draw(CommandBuffer &command_buffer, Window *platform_window)
    {
        // Vertex input state
        VkVertexInputBindingDescription vertex_input_binding{};
        vertex_input_binding.stride = ToUint32_t(sizeof(ImDrawVert));

        // Location 0: Position
        VkVertexInputAttributeDescription pos_attr{};
        pos_attr.format = VK_FORMAT_R32G32_SFLOAT;
        pos_attr.offset = ToUint32_t(offsetof(ImDrawVert, pos));

        // Location 1: UV
        VkVertexInputAttributeDescription uv_attr{};
        uv_attr.location = 1;
        uv_attr.format = VK_FORMAT_R32G32_SFLOAT;
        uv_attr.offset = ToUint32_t(offsetof(ImDrawVert, uv));

        // Location 2: Color
        VkVertexInputAttributeDescription col_attr{};
        col_attr.location = 2;
        col_attr.format = VK_FORMAT_R8G8B8A8_UNORM;
        col_attr.offset = ToUint32_t(offsetof(ImDrawVert, col));

        VertexInputState vertex_input_state{};
        vertex_input_state.bindings = {vertex_input_binding};
        vertex_input_state.attributes = {pos_attr, uv_attr, col_attr};

        command_buffer.GetPipelineState().SetVertexInputState(vertex_input_state);

        // Blend state
        ColorBlendAttachmentState color_attachment{};
        color_attachment.blend_enable = VK_TRUE;
        color_attachment.color_write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
        color_attachment.src_color_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_attachment.dst_color_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_attachment.src_alpha_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

        ColorBlendState blend_state{};
        blend_state.attachments = {color_attachment};

        command_buffer.GetPipelineState().SetColorBlendState(blend_state);

        RasterizationState rasterization_state{};
        rasterization_state.cull_mode = VK_CULL_MODE_NONE;
        command_buffer.GetPipelineState().SetRasterizationState(rasterization_state);

        DepthStencilState depth_state{};
        depth_state.depth_test_enable = VK_FALSE;
        depth_state.depth_write_enable = VK_FALSE;
        command_buffer.GetPipelineState().SetDepthStencilState(depth_state);

        // Bind pipeline layout
        command_buffer.GetPipelineState().SetPipelineLayout(*m_PipelineLayout);

        command_buffer.GetResourceBindingState().BindImage(*m_FontImageView, *m_Sampler, 0, 0, 0);

        // Pre-rotation
        auto &io = ImGui::GetIO();
        auto push_transform = glm::mat4(1.0f);
        ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();

        Window *window{nullptr};
        ImDrawData *draw_data{nullptr};
        for (int i = 0; i < platform_io.Viewports.Size; i++)
        {
            ImGuiViewport *viewport = platform_io.Viewports[i];
            draw_data = viewport->DrawData;
            void *window_handle = static_cast<Window *>(viewport->PlatformHandle);
            window = &m_Application.GetPlatform().GetWindow(window_handle);

            if (window->GetNativeWindow() == platform_window->GetNativeWindow())
                break;
        }

        auto &swapchain = window->GetRenderContext().GetSwapchain();

        if (swapchain != nullptr)
        {
            auto &transform = swapchain->GetProperties().pre_transform;

            glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
            if (transform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)
                push_transform = glm::rotate(push_transform, glm::radians(90.0f), rotation_axis);

            else if (transform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
                push_transform = glm::rotate(push_transform, glm::radians(270.0f), rotation_axis);

            else if (transform & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)
                push_transform = glm::rotate(push_transform, glm::radians(180.0f), rotation_axis);
        }

        // GUI coordinate space to screen space
        push_transform = glm::translate(push_transform, glm::vec3(-1.0f, -1.0f, 0.0f));
        push_transform = glm::scale(push_transform, glm::vec3(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y, 0.0f));

        // Push constants
        command_buffer.PushConstants(push_transform);

        // If a render context is used, then use the frames buffer pools to allocate GUI vertex/index data from
        if (!m_ExplicitUpdate)
        {
            UpdateBuffers(command_buffer, window->GetRenderContext().GetActiveFrame());
        }
        else
        {
            std::vector<std::reference_wrapper<const core::Buffer>> buffers;
            buffers.push_back(*m_VertexBuffer);
            command_buffer.BindVertexBuffers(0, buffers, {0});

            command_buffer.BindIndexBuffer(*m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
        }

        Render(draw_data, swapchain.get(), command_buffer);

        /* ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
        for (int i = 1; i < platform_io.Viewports.Size; i++)
        {
            ImGuiViewport *viewport = platform_io.Viewports[i];
            ImDrawData *draw_data = viewport->DrawData;
            Window *window = static_cast<Window *>(viewport->PlatformUserData);
            // Render(draw_data, swapchain.get(), command_buffer);
            Render(draw_data, window->GetRenderContext().GetSwapchain().get(), command_buffer);
        } */
    }

    void Gui::Render(ImDrawData *draw_data, Swapchain *swapchain, CommandBuffer &command_buffer)
    {
        auto &io = ImGui::GetIO();
        int32_t vertex_offset = 0;
        uint32_t index_offset = 0;

        if (!draw_data || draw_data->CmdListsCount == 0)
        {
            return;
        }

        for (int32_t i = 0; i < draw_data->CmdListsCount; i++)
        {
            const ImDrawList *cmd_list = draw_data->CmdLists[i];
            for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
            {
                const ImDrawCmd *cmd = &cmd_list->CmdBuffer[j];
                VkRect2D scissor_rect;
                scissor_rect.offset.x = std::max(static_cast<int32_t>(cmd->ClipRect.x), 0);
                scissor_rect.offset.y = std::max(static_cast<int32_t>(cmd->ClipRect.y), 0);
                scissor_rect.extent.width = static_cast<uint32_t>(cmd->ClipRect.z - cmd->ClipRect.x);
                scissor_rect.extent.height = static_cast<uint32_t>(cmd->ClipRect.w - cmd->ClipRect.y);

                // Adjust for pre-rotation if necessary
                if (swapchain != nullptr)
                {
                    auto &transform = swapchain->GetProperties().pre_transform;
                    if (transform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)
                    {
                        scissor_rect.offset.x = static_cast<uint32_t>(io.DisplaySize.y - cmd->ClipRect.w);
                        scissor_rect.offset.y = static_cast<uint32_t>(cmd->ClipRect.x);
                        scissor_rect.extent.width = static_cast<uint32_t>(cmd->ClipRect.w - cmd->ClipRect.y);
                        scissor_rect.extent.height = static_cast<uint32_t>(cmd->ClipRect.z - cmd->ClipRect.x);
                    }
                    else if (transform & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)
                    {
                        scissor_rect.offset.x = static_cast<uint32_t>(io.DisplaySize.x - cmd->ClipRect.z);
                        scissor_rect.offset.y = static_cast<uint32_t>(io.DisplaySize.y - cmd->ClipRect.w);
                        scissor_rect.extent.width = static_cast<uint32_t>(cmd->ClipRect.z - cmd->ClipRect.x);
                        scissor_rect.extent.height = static_cast<uint32_t>(cmd->ClipRect.w - cmd->ClipRect.y);
                    }
                    else if (transform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
                    {
                        scissor_rect.offset.x = static_cast<uint32_t>(cmd->ClipRect.y);
                        scissor_rect.offset.y = static_cast<uint32_t>(io.DisplaySize.x - cmd->ClipRect.z);
                        scissor_rect.extent.width = static_cast<uint32_t>(cmd->ClipRect.w - cmd->ClipRect.y);
                        scissor_rect.extent.height = static_cast<uint32_t>(cmd->ClipRect.z - cmd->ClipRect.x);
                    }
                }

                // TODO: crashes here
                command_buffer.SetScissor(0, {scissor_rect});
                command_buffer.DrawIndexed(cmd->ElemCount, 1, index_offset, vertex_offset, 0);
                index_offset += cmd->ElemCount;
            }
            vertex_offset += cmd_list->VtxBuffer.Size;
        }
    }

    void Gui::UpdateBuffers(CommandBuffer &command_buffer, RenderFrame &render_frame)
    {
        ImDrawData *draw_data = ImGui::GetDrawData();

        if (!draw_data)
        {
            return;
        }

        size_t vertex_buffer_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
        size_t index_buffer_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);

        if ((vertex_buffer_size == 0) || (index_buffer_size == 0))
        {
            return;
        }

        std::vector<uint8_t> vertex_data(vertex_buffer_size);
        std::vector<uint8_t> index_data(index_buffer_size);

        UploadDrawData(draw_data, vertex_data.data(), index_data.data());

        // auto &render_frame = m_Window.GetRenderContext().GetActiveFrame();
        auto vertex_allocation = render_frame.AllocateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_buffer_size);

        vertex_allocation.Update(vertex_data);

        std::vector<std::reference_wrapper<const core::Buffer>> buffers;
        buffers.emplace_back(std::ref(vertex_allocation.GetBuffer()));

        std::vector<VkDeviceSize> offsets{vertex_allocation.GetOffset()};

        command_buffer.BindVertexBuffers(0, buffers, offsets);

        auto index_allocation = render_frame.AllocateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_buffer_size);

        index_allocation.Update(index_data);

        command_buffer.BindIndexBuffer(index_allocation.GetBuffer(), index_allocation.GetOffset(), VK_INDEX_TYPE_UINT16);
    }

    void Gui::OnEvent(Event &event)
    {
        auto &io = ImGui::GetIO();
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseMovedEvent>(ENG_BIND_CALLBACK(Gui::OnMouseMoved));
        dispatcher.Dispatch<MouseScrolledEvent>(ENG_BIND_CALLBACK(Gui::OnMouseScrolled));
        dispatcher.Dispatch<MouseButtonPressedEvent>(ENG_BIND_CALLBACK(Gui::OnMouseButtonPressed));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(ENG_BIND_CALLBACK(Gui::OnMouseButtonReleased));
        dispatcher.Dispatch<KeyPressedEvent>(ENG_BIND_CALLBACK(Gui::OnKeyPressed));
        dispatcher.Dispatch<KeyReleasedEvent>(ENG_BIND_CALLBACK(Gui::OnKeyReleased));
        event.handled |= event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
        event.handled |= event.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
    }

    bool Gui::OnKeyPressed(KeyPressedEvent &event)
    {
        auto &io = ImGui::GetIO();
        io.KeysDown[event.GetKeyCode()] = true;
        return false;
    }

    bool Gui::OnKeyReleased(KeyReleasedEvent &event)
    {
        auto &io = ImGui::GetIO();
        io.KeysDown[event.GetKeyCode()] = false;
        return false;
    }

    bool Gui::OnMouseButtonPressed(MouseButtonPressedEvent &event)
    {
        auto &io = ImGui::GetIO();
        io.MouseDown[event.GetMouseButton()] = true;
        return false;
    }

    bool Gui::OnMouseButtonReleased(MouseButtonReleasedEvent &event)
    {
        auto &io = ImGui::GetIO();
        io.MouseDown[event.GetMouseButton()] = false;
        return false;
    }

    bool Gui::OnMouseMoved(MouseMovedEvent &event)
    {
        auto &io = ImGui::GetIO();
        io.MousePos = ImVec2{m_Window.GetInput().GetMouseX(), m_Window.GetInput().GetMouseY()};
        return false;
    }

    bool Gui::OnMouseScrolled(MouseScrolledEvent &event)
    {
        auto &io = ImGui::GetIO();
        io.MouseWheel = event.GetYOffset();
        io.MouseWheelH = event.GetXOffset();
        return false;
    }
}
