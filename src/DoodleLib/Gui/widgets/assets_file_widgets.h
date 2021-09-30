//
// Created by TD on 2021/9/16.
//

#pragma once
#include <DoodleLib/Gui/base_windwos.h>
#include <DoodleLib/doodle_lib_fwd.h>

#include <boost/signals2.hpp>
namespace doodle {

class DOODLELIB_API assets_file_widgets : public metadata_widget {
  metadata_ptr p_root;
  assets_file_ptr p_current_select;

 public:
  assets_file_widgets();
  virtual void frame_render() override;
  void set_metadata(const metadata_ptr& in_ptr);
  boost::signals2::signal<void(const metadata_ptr&)> select_change;
};
}  // namespace doodle
