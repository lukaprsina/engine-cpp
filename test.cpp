struct Callback
{
    static void callback(ImGuiViewport *) { return func(ImGuiViewport *); }
    static std::Gui::ImGuiCreateWindow<void(ImGuiViewport *)> func;
};
Callback::func = std::bind(&Gui::ImGuiCreateWindow, this, std::placeholders::_1);
callback_fnc c_func = static_cast<callback_fnc>(Callback::callback);
platform_io.Platform_CreateWindow = c_func;