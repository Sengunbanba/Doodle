#include "shot.h"

#include <doodle_core/database_task/details/tool.h>
#include <doodle_core/database_task/sql_com.h>
#include <doodle_core/logger/logger.h>

#include <algorithm>
#include <entt/entity/fwd.hpp>
#include <lib_warp/enum_template_tool.h>
#include <magic_enum.hpp>
#include <sqlpp11/aggregate_functions/count.h>
#include <sqlpp11/sqlite3/sqlite3.h>
#include <sqlpp11/sqlpp11.h>
#include <string>
#include <vector>

namespace doodle::database_n {
void sql_com<doodle::shot>::insert(conn_ptr& in_ptr, const std::vector<entt::entity>& in_id) {
  auto& l_conn   = *in_ptr;
  auto l_handles = in_id | ranges::views::transform([&](entt::entity in_entity) {
                     return entt::handle{*reg_, in_entity};
                   }) |
                   ranges::to_vector;
  tables::shot l_table{};

  auto l_pre = l_conn.prepare(sqlpp::sqlite3::insert_or_replace_into(l_table).set(
      l_table.shot_ab = sqlpp::parameter(l_table.shot_ab), l_table.shot_int = sqlpp::parameter(l_table.shot_int),
      l_table.entity_id = sqlpp::parameter(l_table.entity_id)
  ));

  for (auto& l_h : l_handles) {
    auto& l_shot           = l_h.get<shot>();
    l_pre.params.shot_int  = l_shot.p_shot;
    l_pre.params.shot_ab   = l_shot.get_shot_ab();
    l_pre.params.entity_id = boost::numeric_cast<std::int64_t>(l_h.get<database>().get_id());
    auto l_r               = l_conn(l_pre);
    DOODLE_LOG_INFO("插入数据库id {} -> 实体 {} 组件 {} ", l_r, l_h.entity(), entt::type_id<shot>().name());
  }
}

void sql_com<doodle::shot>::select(conn_ptr& in_ptr, const std::map<std::int64_t, entt::entity>& in_handle) {
  auto& l_conn = *in_ptr;
  tables::shot l_table{};
  std::vector<shot> l_shot;
  std::vector<entt::entity> l_entts;
  // 调整内存
  for (auto&& raw :
       l_conn(sqlpp::select(sqlpp::count(l_table.entity_id)).from(l_table).where(l_table.entity_id.is_not_null()))) {
    l_shot.reserve(raw.count.value());
    l_entts.reserve(raw.count.value());
    break;
  }

  for (auto& row : l_conn(sqlpp::select(l_table.entity_id, l_table.shot_ab, l_table.shot_int)
                              .from(l_table)
                              .where(l_table.entity_id.is_not_null()))) {
    shot l_s{};
    l_s.p_shot      = row.shot_int.value();
    l_s.p_shot_ab   = row.shot_ab.value();
    l_s.p_shot_enum = magic_enum::enum_cast<shot::shot_ab_enum>(row.shot_ab.value()).value_or(shot::shot_ab_enum::None);
    auto l_id       = row.entity_id.value();
    if (in_handle.find(l_id) != in_handle.end()) {
      l_shot.emplace_back(std::move(l_s));
      l_entts.emplace_back(in_handle.at(l_id));
      DOODLE_LOG_INFO("选择数据库id {} 插入实体 {}", l_id, in_handle.at(l_id));
    } else {
      DOODLE_LOG_INFO("选择数据库id {} 未找到实体", l_id);
    }
  }
  reg_->insert<doodle::shot>(l_entts.begin(), l_entts.end(), l_shot.begin());
}
void sql_com<doodle::shot>::destroy(conn_ptr& in_ptr, const std::vector<std::int64_t>& in_handle) {
  detail::sql_com_destroy<tables::shot>(in_ptr, in_handle);
}

}  // namespace doodle::database_n