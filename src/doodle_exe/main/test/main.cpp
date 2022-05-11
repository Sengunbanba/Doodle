//
// Created by TD on 2021/7/26.
//

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#include <doodle_lib/app/app.h>
#include <doodle_core/core/doodle_lib.h>

#include <boost/locale.hpp>
// struct init_main : public Catch::TestEventListenerBase {
//  public:
//   using TestEventListenerBase::TestEventListenerBase;
//   void testRunStarting(Catch::TestRunInfo const& testRunInfo) override {
//     std::setlocale(LC_CTYPE, ".UTF8");
//     auto k_doodle = doodle::new_object<doodle::doodle_lib>();
//     doodle::core_set_init k_init{};
//     k_init.config_to_user();

//     doodle::logger_ctrl::get_log().set_log_name("doodle_test.txt");
//   }
// };
// CATCH_REGISTER_LISTENER(init_main);

int main(int argc, char *argv[]) {
  // 初始化测试环境
  boost::locale::generator k_gen{};
  k_gen.categories(boost::locale::all_categories ^
                   boost::locale::formatting_facet ^
                   boost::locale::parsing_facet);
  std::locale::global(k_gen("zh_CN.UTF-8"));
  std::setlocale(LC_NUMERIC, "C");
  std::setlocale(LC_COLLATE, "C");
  std::setlocale(LC_TIME, "C");
  std::setlocale(LC_MONETARY, "C");
  std::setlocale(LC_CTYPE, ".UTF8");
  //  doodle::app l_app{};
  int k_r = Catch::Session().run(argc, argv);
  return k_r;
}
