//
// Created by TD on 2022/1/22.
//

#include "screenshot_widget.h"
#include <doodle_lib/lib_warp/imgui_warp.h>
#include <doodle_lib/app/app.h>
#include <doodle_lib/core/image_loader.h>
#include <doodle_lib/platform/win/wnd_proc.h>
#include <doodle_lib/metadata/image_icon.h>
namespace doodle {

class screenshot_widget::impl {
 public:
  std::vector<std::function<void()>> begen_loop;
  std::shared_ptr<void> image_gui;
  cv::Mat image_mat;
  cv::Rect2f virtual_screen;
  cv::Rect2f mouse_rect;

  cv::Point2f mouse_begin;
  cv::Point2f mouse_end;

  bool mouse_state{};

  entt::handle handle;
};

screenshot_widget::screenshot_widget(const entt::handle& in_handle)
    : p_i(std::make_unique<impl>()) {
  p_i->handle = in_handle;
}
screenshot_widget::~screenshot_widget() = default;
void screenshot_widget::init() {
  if (!p_i->handle.all_of<image_icon>())
    p_i->handle.emplace<image_icon>();

  p_i->begen_loop.emplace_back([&]() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    auto k_rect             = win::get_system_metrics_VIRTUALSCREEN();
    auto k_size             = k_rect.size();
    ImGui::SetNextWindowSize({k_size.width, k_size.height});
    imgui::SetNextWindowPos({k_rect.x, k_rect.y});
    ImGui::SetNextWindowViewport(viewport->ID);
    p_i->image_mat      = image_loader{}.screenshot();
    p_i->image_gui      = image_loader{}.cv_to_d3d(p_i->image_mat);
    p_i->virtual_screen = win::get_system_metrics_VIRTUALSCREEN();
  });
}
void screenshot_widget::succeeded() {
  image_loader{}.save(p_i->handle, p_i->image_mat, p_i->mouse_rect);
}
void screenshot_widget::failed() {
}
void screenshot_widget::aborted() {
}
void screenshot_widget::update(chrono::duration<chrono::system_clock::rep, chrono::system_clock::period>, void* data) {
  for (auto&& fun : p_i->begen_loop) {
    fun();
  }
  p_i->begen_loop.clear();
  //  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.9f, 0.5f, 0.5f, 0.0f));
  dear::Begin{name.data(),
              nullptr,
              ImGuiWindowFlags_NoDecoration |
                  ImGuiWindowFlags_NoResize |
                  ImGuiWindowFlags_NoMove} &&
      [&]() {
        ImGui::ImageButton(p_i->image_gui.get(),
                           {p_i->virtual_screen.size().width,
                            p_i->virtual_screen.size().height});
        if (imgui::IsItemActive() && p_i->mouse_state) {
          p_i->mouse_end.x = imgui::GetIO().MousePos.x;
          p_i->mouse_end.y = imgui::GetIO().MousePos.y;
          p_i->mouse_rect  = {p_i->mouse_begin, p_i->mouse_end};
        }
        if (imgui::IsItemClicked() && !p_i->mouse_state) {
          p_i->mouse_begin.x = imgui::GetIO().MousePos.x;
          p_i->mouse_begin.y = imgui::GetIO().MousePos.y;
          p_i->mouse_state   = true;
        }
        if (imgui::IsItemDeactivated() && p_i->mouse_state) {
          this->succeed();
        }

        ImGui::GetWindowDrawList()
            ->AddRectFilled({p_i->virtual_screen.tl().x,
                             p_i->virtual_screen.tl().y},
                            {p_i->virtual_screen.br().x,
                             p_i->virtual_screen.br().y},
                            ImGui::ColorConvertFloat4ToU32({0.1f, 0.4f, 0.5f, 0.2f}));
        if (!p_i->mouse_rect.empty()) {
          ImGui::GetWindowDrawList()
              ->AddRectFilled({p_i->mouse_rect.tl().x,
                               p_i->mouse_rect.tl().y},
                              {p_i->mouse_rect.br().x,
                               p_i->mouse_rect.br().y},
                              ImGui::ColorConvertFloat4ToU32({1.0f, 1.0f, 1.0f, 0.4f}),
                              0.f);
          ImGui::GetWindowDrawList()
              ->AddRect({p_i->mouse_rect.tl().x,
                         p_i->mouse_rect.tl().y},
                        {p_i->mouse_rect.br().x,
                         p_i->mouse_rect.br().y},
                        ImGui::ColorConvertFloat4ToU32({1.0f, 0.2f, 0.2f, 0.4f}),
                        0.f,
                        5.f);
        }

        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
          imgui::CloseCurrentPopup();
          this->fail();
        }
      };
  //  ImGui::PopStyleColor();
  //  dear::WithStyleVar{ImGuiStyleVar_WindowRounding, 0.0f} && [&]() {
  //  };
}
}  // namespace doodle
