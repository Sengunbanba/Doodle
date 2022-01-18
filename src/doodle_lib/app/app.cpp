//
// Created by TD on 2022/1/18.
//

#include "app.h"
#include <platform/win/wnd_proc.h>
#include <doodle_lib/core/core_set.h>
#include <doodle_lib/long_task/process_pool.h>
#include <doodle_lib/lib_warp/imgui_warp.h>
#include <doodle_lib/gui/main_menu_bar.h>
#include <doodle_lib/gui/main_status_bar.h>

// Helper functions
#include <d3d11.h>
#include <tchar.h>
#include <shellapi.h>

#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui.h>

namespace doodle {
app::app()
    : app_command_base(),
      p_hwnd(),
      p_win_class() {
  p_win_class =
      {sizeof(WNDCLASSEX),
       CS_CLASSDC, win::WndProc,
       0L,
       0L,
       instance,
       nullptr, nullptr, nullptr, nullptr,
       _T("doodle"),
       nullptr};

  // Create application window
  // ImGui_ImplWin32_EnableDpiAwareness();
  ::RegisterClassEx(&p_win_class);
  p_hwnd = ::CreateWindow(p_win_class.lpszClassName,
                          p_title.c_str(),
                          WS_OVERLAPPEDWINDOW,
                          100, 100, 1280, 800,
                          nullptr, nullptr,
                          p_win_class.hInstance,
                          nullptr);

  // Initialize Direct3D
  if (!win::CreateDeviceD3D(p_hwnd)) {
    win::CleanupDeviceD3D();
    ::UnregisterClass(p_win_class.lpszClassName, p_win_class.hInstance);
  }

  // Show the window
  ::ShowWindow(p_hwnd, SW_SHOWDEFAULT);
  ::UpdateWindow(p_hwnd);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();

  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport / Platform Windows
  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;
  // io.ConfigViewportsNoDefaultParent = true;
  // io.ConfigDockingAlwaysTabBar = true;
  // io.ConfigDockingTransparentPayload = true;
  // io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: Experimental. THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
  // io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI: Experimental.

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsClassic();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding              = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  ImGui_ImplWin32_Init(p_hwnd);
  ImGui_ImplDX11_Init(win::g_pd3dDevice, win::g_pd3dDeviceContext);
  d3dDevice = win::g_pd3dDevice;
  /// 初始化文件拖拽
  //  DragAcceptFiles(p_impl->p_hwnd, true);
  //  OleInitialize(nullptr);
  //  auto k_r = RegisterDragDrop(p_impl->p_hwnd, new win::drop_manager{});
  //  chick_true<doodle_error>(k_r == S_OK, DOODLE_LOC, "无法注册拖拽com");

  //  ::ShowWindow(p_impl->p_hwnd, SW_HIDE);
  //  HMONITOR hmon  = MonitorFromWindow(p_impl->p_hwnd,
  //                                     MONITOR_DEFAULTTONEAREST);
  //  MONITORINFO mi = {sizeof(mi)};
  //  auto k_r       = GetMonitorInfo(hmon, &mi);
  //  chick_true<doodle_error>(k_r != 0, DOODLE_LOC, "无法设置全屏");
  //  SetWindowPos(p_impl->p_hwnd, nullptr, mi.rcMonitor.left,
  //               mi.rcMonitor.top,
  //               mi.rcMonitor.right - mi.rcMonitor.left,
  //               mi.rcMonitor.bottom - mi.rcMonitor.top,
  //               SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
  // IM_ASSERT(font != NULL);
  static string imgui_file_path{(core_set::getSet().get_cache_root("imgui") / "imgui.ini").generic_string()};
  set_imgui_dock_space(imgui_file_path);

  //  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\simkai.ttf)", 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
  io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\simhei.ttf)", 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
  io.IniFilename = imgui_file_path.c_str();
  this->load_windows();
}
void app::loop_one() {
  // Poll and handle messages (inputs, window resize, etc.)
  // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
  // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
  // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
  // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
  MSG msg;
  while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
    if (msg.message == WM_QUIT)
      stop_ = true;
  }
  if (stop_)
    return;

  // Start the Dear ImGui frame
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  imgui::DockSpaceOverViewport(imgui::GetMainViewport());
  static std::string str{};

  try {
    if (!p_show_err) {
      app_command_base::loop_one();
    }
  } catch (doodle_error& err) {
    DOODLE_LOG_ERROR("捕获 doodle_error异常 {}", err.what());
    p_show_err = true;
    str        = err.what();
    imgui::OpenPopup("警告");
  } catch (std::runtime_error& err) {
    DOODLE_LOG_ERROR("捕获 runtime_error异常 {}", err.what());
    p_show_err = true;
    str        = err.what();
    imgui::OpenPopup("警告");
  } catch (std::exception& err) {
    DOODLE_LOG_ERROR("捕获 exception异常 {}", err.what());
    p_show_err = true;
    str        = err.what();
    imgui::OpenPopup("警告");
  }
  dear::PopupModal{"警告", &p_show_err} && [str1 = str, this]() {
    dear::Text(str);
    if (ImGui::Button("OK")) {
      this->p_show_err = false;
      ImGui::CloseCurrentPopup();
    }
  };

  // Rendering
  ImGui::Render();
  ImVec4 clear_color                    = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  const float clear_color_with_alpha[4] = {clear_color.x * clear_color.w,
                                           clear_color.y * clear_color.w,
                                           clear_color.z * clear_color.w,
                                           clear_color.w};
  win::g_pd3dDeviceContext->OMSetRenderTargets(1, &win::g_mainRenderTargetView, nullptr);
  win::g_pd3dDeviceContext->ClearRenderTargetView(win::g_mainRenderTargetView, clear_color_with_alpha);
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  // Update and Render additional Platform Windows
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }

  win::g_pSwapChain->Present(1, 0);  // Present with vsync
                                     // g_pSwapChain->Present(0, 0); // Present without vsync
}
app& app::Get() {
  return *(dynamic_cast<app*>(self));
}
bool app::valid() const {
  return this->p_hwnd != nullptr;
}
void app::hide_windows() {
  ::ShowWindow(p_hwnd, SW_HIDE);
  stop_ = true;
  ::DestroyWindow(p_hwnd);
}
void app::show_windows() {
  if (!stop_)
    ::ShowWindow(p_hwnd, SW_SHOW);
}
void app::load_windows() {
  g_main_loop().attach<main_menu_bar>();
  g_main_loop().attach<main_status_bar>();
  g_main_loop().attach<null_process_t>().then([](auto, auto, auto s, auto) {
    core_set::getSet().load_first_project();
    s();
  });
}
void app::set_imgui_dock_space(const FSys::path& in_path) const {
  auto k_f = cmrc::DoodleLibResource::get_filesystem().open("resource/imgui.ini");
  if (FSys::exists(in_path))
    return;
  FSys::ofstream l_ofs{in_path, std::ios::out | std::ios::binary};
  if (l_ofs)
    l_ofs.write(k_f.begin(), k_f.size());
}
app::~app() {
  // Cleanup
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  win::CleanupDeviceD3D();
  ::DestroyWindow(p_hwnd);
  ::UnregisterClass(p_win_class.lpszClassName, p_win_class.hInstance);
  //  OleUninitialize();
}

}  // namespace doodle
