//
// Created by TD on 2022/1/18.
//

#pragma once
#include <doodle_core/doodle_core_fwd.h>
#include <doodle_core/platform/win/windows_alias.h>

namespace doodle::win {

class DOODLE_CORE_EXPORT d3d_device {
  static d3d_device* self;

 public:
  ID3D11Device* g_pd3dDevice                     = nullptr;
  ID3D11DeviceContext* g_pd3dDeviceContext       = nullptr;
  IDXGISwapChain* g_pSwapChain                   = nullptr;
  ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
  wnd_handle handle_wnd;

  explicit d3d_device(const wnd_handle& in_handle);
  virtual ~d3d_device();
  bool CreateDeviceD3D(HWND hWnd);
  void CleanupDeviceD3D();

  void CreateRenderTarget();
  void CleanupRenderTarget();
  static d3d_device& Get();
};

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

}  // namespace doodle::win
