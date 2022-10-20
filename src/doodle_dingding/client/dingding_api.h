//
// Created by TD on 2022/9/9.
//
#pragma once

#include <doodle_core/doodle_core.h>
#include <doodle_core/lib_warp/entt_warp.h>

#include <doodle_dingding/client/client.h>
#include <nlohmann/json_fwd.hpp>
namespace doodle::dingding {

namespace user_dd_ns {
class dep_query;
class find_by_mobile;
class get_user_info;
}  // namespace user_dd_ns

namespace attendance {
namespace query {
class get_day_data;
class get_update_data;
}  // namespace query
}  // namespace attendance

namespace department_ns {
class department_query;
class user_to_dep_query;
}  // namespace department_ns

class access_token;
using dingidng_call_fun     = std::function<void(const std::vector<entt::handle>&)>;
using read_access_token_fun = std::function<void(const access_token&)>;

class DOODLE_DINGDING_API dingding_api : public client {
 private:
  void async_get_user_info(
      const user_dd_ns::get_user_info& in_query, const access_token& in_token, dingidng_call_fun&& in_fun
  );

 public:
  constexpr static const std::string_view dingding_host{"https://oapi.dingtalk.com"};

  explicit dingding_api(const boost::asio::any_io_executor& in_executor, boost::asio::ssl::context& in_ssl_context);
  /**
   * @brief 获取钉钉的授权令牌
   * @param in  回调
   */
  void async_get_token(read_access_token_fun&& in);

  /**
   * 获取部门
   * @param in_query
   * @param in_token
   * @param in_fun
   */
  void async_get_departments(
      const department_ns::department_query& in_query, const access_token& in_token, dingidng_call_fun&& in_fun
  );
  /**
   * @brief 获取部门用户
   * @param in_query
   * @param in_token
   * @param in_fun
   */
  void async_get_departments_user(
      const user_dd_ns::dep_query& in_query, const access_token& in_token, dingidng_call_fun&& in_fun
  );

  /**
   * @brief 根据手机获取用户
   * @param in_query
   * @param in_token
   * @param in_fun
   */
  void async_find_mobile_user(
      const user_dd_ns::find_by_mobile& in_query, const access_token& in_token, dingidng_call_fun&& in_fun
  );
  /**
   * @brief 获取考勤
   * @param in_query
   * @param in_token
   * @param in_fun
   */
  void async_get_user_day_attendance(
      const attendance::query::get_day_data& in_query, const access_token& in_token, dingidng_call_fun&& in_fun
  );
  /**
   * @brief 获取考勤
   * @param in_query
   * @param in_token
   * @param in_fun
   */
  void async_get_user_updatedata_attendance(
      const attendance::query::get_update_data& in_query, const access_token& in_token, dingidng_call_fun&& in_fun
  );
};

}  // namespace doodle::dingding
