#pragma once

#include <doodle_core/database_task/details/tool.h>
#include <doodle_core/doodle_core_fwd.h>
#include <doodle_core/metadata/comment.h>

namespace doodle::database_n {
template <>
struct sql_com<doodle::comment> : detail::sql_create_table_base<tables::comment> {
  registry_ptr reg_;
  sql_com() = default;
  sql_com(registry_ptr reg) : reg_{std::move(reg)} {}
  void insert(conn_ptr& in_ptr, const std::vector<entt::entity>& in_id);
  /**
   *
   * @param in_ptr
   * @param in_handle id与之相对的实体
   */
  void select(conn_ptr& in_ptr, const std::map<std::int64_t, entt::entity>& in_handle);
  void destroy(conn_ptr& in_ptr, const std::vector<std::int64_t>& in_handle);
};
}  // namespace doodle::database_n