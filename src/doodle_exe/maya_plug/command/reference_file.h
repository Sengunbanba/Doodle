//
// Created by TD on 2021/11/30.
//
#pragma once
#include <doodle_lib/doodle_lib_fwd.h>
#include <maya/MSelectionList.h>
namespace doodle::maya_plug {

class reference_file {
  uuid prj_ref;
  string ref_file_uuid;

 public:
  string path;
  bool use_sim;
  bool high_speed_sim;
  std::vector<string> collision_model;
  std::vector<string> collision_model_show_str;

  reference_file();
  explicit reference_file(const entt::handle &in_uuid, const string &in_u8_path);
  explicit reference_file(const entt::handle &in_uuid, const MObject &in_ref_node);

  void init_show_name();
  [[nodiscard]] MSelectionList get_collision_model() const;
  void set_collision_model(const MSelectionList &in_list);
  string get_namespace();

 private:
  friend void to_json(nlohmann::json &j, const reference_file &p) {
    j["path"]            = p.path;
    j["use_sim"]         = p.use_sim;
    j["high_speed_sim"]  = p.high_speed_sim;
    j["collision_model"] = p.collision_model;
    j["prj_ref"]         = p.prj_ref;
  }
  friend void from_json(const nlohmann::json &j, reference_file &p) {
    j.at("path").get_to(p.path);
    j.at("use_sim").get_to(p.use_sim);
    j.at("prj_ref").get_to(p.prj_ref);
    j.at("high_speed_sim").get_to(p.high_speed_sim);
    j.at("collision_model").get_to(p.collision_model);
  }
};

}  // namespace doodle::maya_plug
