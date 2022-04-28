﻿//
// Created by TD on 2021/5/7.
//

#pragma once
#include <doodle_core/doodle_core_fwd.h>

namespace doodle {
class user {
 public:
  using time_pair_list = std::vector<std::pair<chrono::sys_time_pos, chrono::sys_time_pos> >;

 private:
  std::string p_string_;
  std::string p_ENUS;

  time_pair_list p_time_rest;
  time_pair_list p_time_work;

 public:
  user();
  DOODLE_MOVE(user);
  explicit user(std::string in_string);
  explicit user(std::string in_string, std::string in_ENUS);

  [[nodiscard]] const std::string& get_name() const;
  void set_name(const std::string& in_string);
  void set_name(const std::string& in_string, const std::string& in_ENUS);

  [[nodiscard]] const std::string& get_enus() const;
  void set_enus(const std::string& in_string);

  const time_pair_list& get_time_work_list() const;
  time_pair_list& get_time_work_list();

  const time_pair_list& get_time_rest_list() const;
  time_pair_list& get_time_rest_list();

 private:

  friend void to_json(nlohmann::json& j, const user& p) {
    j["string_"]   = p.p_string_;
    j["ENUS"]      = p.p_ENUS;
    j["time_rest"] = p.p_time_rest;
    j["time_work"] = p.p_time_work;
  }
  friend void from_json(const nlohmann::json& j, user& p) {
    j.at("string_").get_to(p.p_string_);
    j.at("ENUS").get_to(p.p_ENUS);
    j.at("time_rest").get_to(p.p_time_rest);
    j.at("time_work").get_to(p.p_time_work);
  }
};


}  // namespace doodle
