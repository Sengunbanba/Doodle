//
// Created by TD on 2021/9/15.
//

#pragma once
#include <DoodleLib/DoodleLib_fwd.h>
#include <DoodleLib/libWarp/imgui_warp.h>

#include <boost/hana/experimental/printable.hpp>
namespace doodle {

class DOODLELIB_API base_windows : public details::no_copy {
 public:
  virtual void frame_render(const bool_ptr& is_show) = 0;
};
class DOODLELIB_API base_widget : public details::no_copy {
 public:
  virtual void frame_render() = 0;
  //  virtual const string& get_class_name();
};

class DOODLELIB_API metadata_widget : public base_widget {
 protected:
  attribute_factory_ptr p_factory;

 public:
  virtual attribute_factory_ptr get_factory();
};

template <class widget>
class DOODLELIB_API windows_warp : public base_widget {
 public:
  using widget_ptr = std::shared_ptr<widget>;

  bool_ptr p_show;
  widget_ptr p_widget;

  windows_warp()
      : p_show(new_object<bool>()),
        p_widget(new_object<widget>()){};

  void frame_render() override {
    if (*p_show) {
      dear::Begin{"rea",
                  p_show.get()} &&
          std::bind(&widget::frame_render, this->p_widget.get());
    }
  }
};

template <class widget>
std::shared_ptr<widget> win_cast(const base_widget_ptr& in) {
  auto ptr = std::dynamic_pointer_cast<windows_warp<widget>>(in);
  if (ptr)
    return ptr->p_widget;
  else
    return nullptr;
}
}  // namespace doodle
