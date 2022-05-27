//
// Created by TD on 2021/9/18.
//

#include "command_tool.h"

#include <gui/open_file_dialog.h>

#include <doodle_core/metadata/episodes.h>
#include <doodle_core/metadata/season.h>
#include <doodle_core/metadata/assets_file.h>
#include <doodle_core/metadata/shot.h>
#include <doodle_core/metadata/organization.h>
#include <doodle_lib/exe_warp/maya_exe.h>
#include <doodle_core/core/core_set.h>
#include <doodle_core/core/core_sig.h>
#include <doodle_core/core/doodle_lib.h>
#include <doodle_lib/exe_warp/maya_exe.h>

#include <long_task/image_to_move.h>
#include <gui/gui_ref/ref_base.h>
#include <long_task/join_move.h>

#include <utility>

namespace doodle {

comm_maya_tool::comm_maya_tool()
    : p_cloth_path(),
      p_text(),
      p_sim_path(),
      p_only_sim(false),
      p_use_all_ref(false),
      p_upload_files(false) {
  title_name_ = std::string{name};
}
void comm_maya_tool::init() {
  gui::window_panel::init();

  g_reg()->ctx().at<core_sig>().project_end_open.connect(
      [this](const entt::handle& in_handle, const doodle::project&) {
        p_text = in_handle.get_or_emplace<project_config::base_config>().vfx_cloth_sim_path.generic_string();
      });
  g_reg()->ctx().at<core_sig>().select_handles.connect(
      [this](const std::vector<entt::handle>& in_list) {
        p_sim_path = in_list |
                     ranges::views::filter([](const entt::handle& in_handle) -> bool {
                       return in_handle &&
                              in_handle.any_of<assets_file>();
                     }) |
                     ranges::views::filter([](const entt::handle& in_handle) -> bool {
                       auto l_ex = in_handle.get<assets_file>().path.extension();
                       return l_ex == ".ma" ||
                              l_ex == ".mb";
                     }) |
                     ranges::views::transform([](const entt::handle& in_handle) -> FSys::path {
                       return in_handle.get<assets_file>().get_path_normal();
                     }) |
                     ranges::to_vector;
      });

  p_text = g_reg()->ctx().at<project_config::base_config>().vfx_cloth_sim_path.generic_string();
  g_reg()->ctx().emplace<comm_maya_tool&>(*this);
}

void comm_maya_tool::render() {
  dear::ListBox{"file_list"} && [this]() {
    for (const auto& f : p_sim_path) {
      dear::Selectable(f.generic_string());
    }
  };

  dear::Text(fmt::format("解算资产: {}", p_text));

  imgui::Checkbox("自动上传", &p_upload_files);
  dear::TreeNode{"解算设置"} && [this]() {
    imgui::Checkbox("只解算不替换引用", &p_only_sim);
  };
  dear::TreeNode{"fbx导出设置"} && [&]() {
    imgui::Checkbox("直接加载所有引用", &p_use_all_ref);
  };

  if (imgui::Button("解算")) {
    std::for_each(p_sim_path.begin(), p_sim_path.end(),
                  [this](const FSys::path& in_path) {
                    auto k_arg        = maya_exe_ns::qcloth_arg{};
                    k_arg.file_path   = in_path;
                    k_arg.only_sim    = p_only_sim;
                    k_arg.upload_file = p_upload_files;
                    k_arg.project_    = g_reg()->ctx().at<database_info>().path_;
                    g_bounded_pool().attach<maya_exe>(
                        make_handle(),
                        k_arg);
                  });
  }
  ImGui::SameLine();
  if (imgui::Button("fbx导出")) {
    std::for_each(p_sim_path.begin(), p_sim_path.end(),
                  [this](const FSys::path& i) {
                    auto k_arg        = maya_exe_ns::export_fbx_arg{};
                    k_arg.file_path   = i;
                    k_arg.use_all_ref = this->p_use_all_ref;
                    k_arg.upload_file = p_upload_files;

                    k_arg.project_    = g_reg()->ctx().at<database_info>().path_;
                    g_bounded_pool().attach<maya_exe>(
                        make_handle(),
                        k_arg);
                  });
  }
  ImGui::SameLine();
  if (imgui::Button("引用文件替换")) {
    std::for_each(p_sim_path.begin(), p_sim_path.end(),
                  [this](const FSys::path& i) {
                    auto k_arg             = maya_exe_ns::replace_file_arg{};
                    k_arg.file_path        = i;
                    k_arg.replace_file_all = true;
                    k_arg.project_         = g_reg()->ctx().at<database_info>().path_;
                    g_bounded_pool().attach<maya_exe>(
                        make_handle(),
                        k_arg);
                  });
  }
}

class comm_create_video::image_arg : public gui::gui_cache<std::string> {
 public:
  using base_type = gui::gui_cache<std::string>;
  explicit image_arg(const entt::handle& in_handle,
                     std::vector<FSys::path> in_image_attr,
                     const std::string& in_show_str)
      : base_type(in_show_str),
        out_handle(in_handle),
        image_attr(std::move(in_image_attr)){};

  entt::handle out_handle;
  std::vector<FSys::path> image_attr;
};

class comm_create_video::impl {
 public:
  gui::gui_cache<std::string> out_path{"输出路径"s, ""s};

  using image_cache = comm_create_video::image_arg;
  using video_cache = gui::gui_cache<std::string>;

  std::vector<image_cache> image_to_video_list;
  std::vector<video_cache> video_list;
  entt::handle out_video_h;
};

comm_create_video::comm_create_video()
    : p_i(std::make_unique<impl>()) {
  title_name_ = std::string{name};
}
void comm_create_video::init() {
  gui::window_panel::init();
  p_i->out_video_h = make_handle();

  g_reg()->ctx().emplace<comm_create_video&>(*this);
}

void comm_create_video::render() {
  if (ImGui::InputText(*p_i->out_path.gui_name, &p_i->out_path.data)) {
    ::ranges::for_each(p_i->image_to_video_list, [this](impl::image_cache& in_image_cache) {
      in_image_cache.out_handle.emplace_or_replace<FSys::path>(p_i->out_path.data);
    });
  };
  ImGui::SameLine();
  if (ImGui::Button("选择")) {
    auto l_ptr = std::make_shared<FSys::path>();
    g_main_loop()
        .attach<file_dialog>(file_dialog::dialog_args{l_ptr}
                                   .set_title("选择目录"s)
                                   .set_use_dir())
        .then<one_process_t>([this, l_ptr]() {
          p_i->out_path.data = l_ptr->generic_string();
          ranges::for_each(p_i->image_to_video_list, [this](impl::image_cache& in_image_cache) {
            in_image_cache.out_handle.emplace_or_replace<FSys::path>(p_i->out_path.data);
          });
        });
  }

  if (imgui::Button("选择图片")) {
    auto l_ptr = std::make_shared<std::vector<FSys::path>>();
    g_main_loop()
        .attach<file_dialog>(file_dialog::dialog_args{l_ptr}
                               .set_title("选择序列"s)
                               .set_filter(string_list{".png", ".jpg"}))
        .then<one_process_t>([this, l_ptr]() {
          p_i->image_to_video_list.emplace_back(
              create_image_to_move_handle(l_ptr->front()),
              *l_ptr,
              l_ptr->front().generic_string());
        });
  }
  imgui::SameLine();
  if (imgui::Button("选择文件夹")) {
    auto l_ptr = std::make_shared<std::vector<FSys::path>>();
    g_main_loop()
        .attach<file_dialog>(file_dialog::dialog_args{l_ptr}
                               .set_title("select dir"s)
                               .set_use_dir())
        .then<one_process_t>([=]() {
          ranges::for_each(*l_ptr, [this](const FSys::path& in_path) {
            std::vector<FSys::path> list =
                ranges::make_subrange(FSys::directory_iterator{in_path},
                                      FSys::directory_iterator{}) |
                ranges::views::filter([](const FSys::directory_entry& in_file) {
                  return FSys::is_regular_file(in_file);
                }) |
                ranges::view::transform([](const FSys::directory_entry& in_file) -> FSys::path {
                  return in_file.path();
                }) |
                ranges::to_vector;
            p_i->image_to_video_list.emplace_back(
                create_image_to_move_handle(in_path),
                list,
                in_path.generic_string());
          });
        });
  }

  imgui::SameLine();
  if (imgui::Button("清除")) {
    p_i->image_to_video_list.clear();
  }
  imgui::SameLine();
  if (imgui::Button("创建视频")) {
    ranges::for_each(p_i->image_to_video_list,
                     [this](const impl::image_cache& in_cache) {
                       g_bounded_pool().attach<image_to_move>(
                                           in_cache.out_handle,
                                           in_cache.image_attr)
                           .then<one_process_t>([this, l_h = in_cache.out_handle]() {  /// \brief 在这里我们将合成的视频添加到下一个工具栏中
                             auto l_out_path = l_h.get<FSys::path>();
                             p_i->video_list.emplace_back(l_out_path.generic_string(), l_out_path.generic_string());
                           });
                     });
  }

  dear::ListBox{"image_list"} && [this]() {
    for (const auto& i : p_i->image_to_video_list) {
      dear::Selectable(*i.gui_name);
    }
  };

  if (imgui::Button("选择视频")) {
    auto l_ptr = std::make_shared<std::vector<FSys::path>>();
    g_main_loop()
        .attach<file_dialog>(file_dialog::dialog_args{l_ptr}
                               .set_title("select mp4 file"s)
                               .add_filter(".mp4"))
        .then<one_process_t>([=]() {
          p_i->video_list |= ranges::action::push_back(
              *l_ptr |
              ranges::views::transform([](const FSys::path& in_path)
                                           -> impl::video_cache {
                return impl::video_cache{in_path.generic_string()};
              }));
        });
  }
  imgui::SameLine();
  if (imgui::Button("清除视频")) {
    p_i->video_list.clear();
  }
  imgui::SameLine();
  if (imgui::Button("连接视频")) {
    auto l_list = p_i->video_list | ranges::views::transform([this](impl::video_cache& in_cache) -> FSys::path {
                    return in_cache.data;
                  }) |
                  ranges::to_vector;

    p_i->out_video_h.remove<episodes>();
    ranges::find_if(p_i->video_list, [this](impl::video_cache& in_cache) -> bool {
      return episodes::analysis_static(p_i->out_video_h, in_cache.data);
    });

    p_i->out_video_h.emplace_or_replace<FSys::path>(p_i->out_path.data);
    p_i->out_video_h.emplace_or_replace<process_message>();
    g_bounded_pool().attach<details::join_move>(p_i->out_video_h, l_list);
  }

  dear::ListBox{"video_list"} && [this]() {
    for (const auto& i : p_i->video_list) {
      dear::Selectable(*i.gui_name);
    }
  };
}
entt::handle comm_create_video::create_image_to_move_handle(
    const FSys::path& in_path) {
  auto l_h = make_handle();
  l_h.emplace<process_message>();
  season::analysis_static(l_h, in_path);
  episodes::analysis_static(l_h, in_path);
  shot::analysis_static(l_h, in_path);
  l_h.emplace_or_replace<FSys::path>(p_i->out_path.data);
  return l_h;
}
comm_create_video::~comm_create_video() = default;

}  // namespace doodle
