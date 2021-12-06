#pragma once
#include <doodle_lib/doodle_lib_fwd.h>
#include <doodle_lib/metadata/metadata.h>

#include <nlohmann/json.hpp>

namespace doodle {
/**
 * 项目信息类
 */
class DOODLELIB_API project {
 public:
  class DOODLELIB_API cloth_config {
   public:
    cloth_config();
    FSys::path vfx_cloth_sim_path;
    bool simple_subsampling;
    std::float_t frame_samples;
    std::float_t time_scale;
    std::float_t length_scale;

    /**
     * @brief 解算形状(这个是解算器生成的布料节点,具有缓存属性等)
     */
    std::string cloth_shape;
    /**
     * @brief 定位布料所需节点名称,我们使用这个节点的outMesh属性拉动节点计算,
     * 需要解算布料节点的下游节点
     */
    std::string cloth_proxy;
    /**
     * @brief 导出整个解算文件所需要的选择组
     * 我们使用这个组导出整个解算结果
     */
    std::string export_group;

    friend void to_json(nlohmann::json& j, const cloth_config& p) {
      j["vfx_cloth_sim_path"] = p.vfx_cloth_sim_path;
      j["simple_subsampling"] = p.simple_subsampling;
      j["frame_samples"]      = p.frame_samples;
      j["time_scale"]         = p.time_scale;
      j["length_scale"]       = p.length_scale;
      j["cloth_shape"]        = p.cloth_shape;
      j["cloth_proxy"]        = p.cloth_proxy;
      j["export_group"]       = p.export_group;
    }
    friend void from_json(const nlohmann::json& j, cloth_config& p) {
      j.at("vfx_cloth_sim_path").get_to(p.vfx_cloth_sim_path);
      j.at("simple_subsampling").get_to(p.simple_subsampling);
      j.at("frame_samples").get_to(p.frame_samples);
      j.at("time_scale").get_to(p.time_scale);
      j.at("length_scale").get_to(p.length_scale);
      j.at("cloth_shape").get_to(p.cloth_shape);
      j.at("cloth_proxy").get_to(p.cloth_proxy);
      j.at("export_group").get_to(p.export_group);
    }
  };

 private:
  std::string p_name;
  std::string p_en_str;
  std::string p_shor_str;
  FSys::path p_path;
  void init_name();

 public:
  project();
  explicit project(FSys::path in_path, std::string in_name = {});

  [[nodiscard]] const std::string& get_name() const;
  void set_name(const std::string& Name) noexcept;

  [[nodiscard]] const FSys::path& get_path() const noexcept;
  void set_path(const FSys::path& Path);

  [[nodiscard]] std::string str() const;
  [[nodiscard]] std::string show_str() const;

  [[nodiscard]] std::string short_str() const;

  cloth_config& get_vfx_cloth_config() const;

  virtual void attribute_widget(const attribute_factory_ptr& in_factoryPtr);

  bool operator<(const project& in_rhs) const;
  bool operator>(const project& in_rhs) const;
  bool operator<=(const project& in_rhs) const;
  bool operator>=(const project& in_rhs) const;
  bool operator==(const project& in_rhs) const;
  bool operator!=(const project& in_rhs) const;

 private:
  friend void to_json(nlohmann::json& j, const project& p) {
    j["name"] = p.p_name;
    j["path"] = p.p_path;
  }
  friend void from_json(const nlohmann::json& j, project& p) {
    j.at("name").get_to(p.p_name);
    j.at("path").get_to(p.p_path);
    p.init_name();
  }
};

}  // namespace doodle
