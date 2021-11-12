//
// Created by TD on 2021/7/27.
//
#include <doodle_lib/doodle_lib_all.h>

#include <catch.hpp>

TEST_CASE("convert", "[metadata]") {
  using namespace doodle;
  auto reg   = g_reg();
  auto k_prj = make_handle(reg->create());
  auto& k_p  = k_prj.emplace<project>();
  REQUIRE(k_prj.all_of<project, tree_relationship, database>());

  auto& k_d = k_prj.get<database>();

  metadata_database k_data{k_d};
  std::cout << k_data.DebugString() << std::endl;

  auto k_s = make_handle(reg->create());
  auto& s  = k_s.emplace<shot>();
  s.set_shot(1);
  s.set_shot_ab(shot::shot_ab_enum::A);
  REQUIRE(k_s.all_of<shot, tree_relationship, database>());

  auto& k_d2 = reg->get<database>(k_s);

  metadata_database k_data2{k_d2};
  std::cout << k_data2.DebugString() << std::endl;

  auto k_tmp2 = make_handle(reg->create());
  auto& k_d3  = k_tmp2.get_or_emplace<database>();
  k_d3        = k_data2;

  std::cout << "k_d3 id: " << k_d3.get_url_uuid() << std::endl;
  std::cout << "k_d2 id: " << k_d2.get_url_uuid() << std::endl;
  REQUIRE(k_d3 == k_d2);
}

TEST_CASE("time duration", "[metadata]") {
  using namespace doodle;
  using namespace doodle::chrono::literals;
  auto k_new   = chrono::sys_days{2021_y / 06 / 16} + 10h + 34min + 37s;
  auto k_local = chrono::local_days{2021_y / 06 / 16} + 18h + 34min + 37s;
  //  REQUIRE(doodle::TimeDuration{}.getUTCTime() == chrono::system_clock::now());
  SECTION("date time sys") {
    auto k_time = chrono::make_zoned(chrono::current_zone(), k_new);
    REQUIRE(k_time.get_local_time() == k_local);
  }
  SECTION("date time local") {
    auto k_time = chrono::make_zoned(chrono::current_zone(), k_local);
    REQUIRE(k_time.get_sys_time() == k_new);
  }

  doodle::time_point_wrap my_t{};
  doodle::time_point_wrap my_t2{};
  SECTION("time set") {
    my_t.set_year(2021);
    my_t.set_month(6);
    my_t.set_day(16);
    my_t.set_hour(10);
    my_t.set_minutes(34);
    my_t.set_second(37);
    REQUIRE(my_t.get_year() == 2021);
    REQUIRE(my_t.get_month() == 6);
    REQUIRE(my_t.get_day() == 16);
    REQUIRE(my_t.get_hour() == 10);
    REQUIRE(my_t.get_minutes() == 34);
    REQUIRE(my_t.get_second() == 37);

    REQUIRE(my_t.get_local_time() == doodle::chrono::clock_cast<doodle::chrono::local_t>(k_new));
  }

  SECTION("set local") {
    my_t.set_local_time(k_local);
    REQUIRE(my_t.get_utc_time() == k_new);
  }
  SECTION("time duration") {
    auto k_sys_time1 = chrono::local_days(2021_y / 7 / 21_d) + 10h + 45min + 30s;
    auto k_sys_time2 = chrono::local_days(2021_y / 7 / 23_d) + 16h + 20min + 30s;
    my_t.set_local_time(k_sys_time1);
    my_t2.set_local_time(k_sys_time2);
    using namespace Catch::literals;
    //    auto k_matcher = Catch::Approx(5.1);

    REQUIRE(my_t.work_duration(my_t2).count() == (20.583_a).epsilon(0.01));
    SECTION("time works durtion") {
      k_sys_time1 = chrono::local_days(2021_y / 7 / 21_d) + 10h + 45min + 30s;
      k_sys_time2 = chrono::local_days(2021_y / 7 / 27_d) + 16h + 20min + 30s;
      my_t.set_local_time(k_sys_time1);
      my_t2.set_local_time(k_sys_time2);
      REQUIRE(my_t.work_duration(my_t2).count() == (36.583_a).epsilon(0.01));
    }
    SECTION("ond day time") {
      k_sys_time1 = chrono::local_days(2021_y / 6 / 23_d) + 17h + 8min + 48s;
      k_sys_time2 = chrono::local_days(2021_y / 6 / 23_d) + 20h + 8min + 48s;
      my_t.set_local_time(k_sys_time1);
      my_t2.set_local_time(k_sys_time2);
      REQUIRE(my_t.work_duration(my_t2).count() == (0.86_a).epsilon(0.01));
    }
  }
}

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
TEST_CASE("test create metadata", "[server][metadata]") {
  using namespace doodle;
  std::string user{
      "宦倩冰 邰小溪 孔佳晨 顾柏文 乌书竹 邱媛女 冷梓颖"
      " 乔平良 闻爱萍 堵乐然 钭如之 屠宵月 农曦秀 苍佳洁 索优悦 黎晶晶"
      " 杜珠玉 苏若美 宦修美 弓博敏 巴沛蓝 邵河灵 靳佩兰 张子丹 鱼绮丽"
      " 胡怀亦 幸艳芳 衡韶丽 薛友绿 耿丝琪 杜俊慧 双芮悦 欧妙菡 陆梅雪"
      " 寿江红 益小凝 燕洮洮 古逸雅 宁梦华 扶倩愉 国宇丞 魏梅雪 冯诗蕊"
      " 刘秋双 宰怡丞 须奇文 蓟言文 邹心诺 陈曦秀 谢绣文 充靖柔 红凡梦"
      " 冷霞绮 郏怡若 庄书萱 谭布侬 罗芷蕾 籍子琳 吕向莉 贺颜落 蔚元瑶"
      " 冷绮云 家密思 钱云琼 养茵茵 鄂曼吟 璩静恬 步幸瑶 仰陶宁 秦芸静"
      " 温玉兰 潘华楚 金初瑶 孔蔚然 朱诗晗 相夏槐 秦小之 焦念薇 陆丽英"
      " 程香洁 万陶然 浦迎荷 隆智敏 潘蔓蔓 曾子芸 乌滢滢 古笑雯 顾半烟"
      " 宫津童 彭灵波 翟诗桃 须思聪 方碧玉 梁羡丽 漕闲华 韩皎月 扈晴丽"
      " 温燕平 冀冬梅 赖代容"};
  std::istringstream iss{user};
  string_list user_list{std::istream_iterator<std::string>(iss),
                        std::istream_iterator<std::string>()};
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist{1, 30};

  auto k_server = rpc_server_handle{};
  auto& set     = core_set::getSet();
  k_server.run_server(set.get_meta_rpc_port(), set.get_file_rpc_port());
  doodle_lib::Get().init_gui();

  SECTION("create project") {
    auto k_prj = make_handle();
    k_prj.emplace<project>("D:/tmp", "case_tset");
    k_prj.get<database_stauts>().set<need_save>();
    k_prj.get<root_ref>().set_root(k_prj);

    for (size_t i = 0; i < 20; ++i) {
      if (i > 15) {
        auto k_ass = make_handle();
        k_ass.emplace<assets>(fmt::format("test{}", i));
        k_ass.get<database_stauts>().set<need_save>();
        k_ass.get<root_ref>().set_root(k_prj);

      } else {
        entt::handle k_i1 = make_handle();
        k_i1.emplace<season>(std::int32_t(i % 5));
        k_i1.get<database_stauts>().set<need_save>();
        k_i1.emplace<episodes>(i);
        k_i1.get<root_ref>().set_root(k_prj);

        for (size_t k = 0; k < 100; ++k) {
          entt::handle k_i2 = make_handle();
          k_i2.emplace<season>(std::int32_t(i % 5));
          k_i2.emplace<episodes>().set_episodes(k);
          auto& k_sho = k_i2.emplace<shot>();
          k_sho.set_shot(k);
          if (i % 2 == 0) {
            k_sho.set_shot_ab(shot::shot_ab_enum::B);
          }
          using namespace chrono::literals;
          k_i2.emplace<assets_file>();
          k_i2.get<database_stauts>().set<need_save>();
          k_i2.get<time_point_wrap>().set_time(chrono::system_clock::now() - 3h * i);
          auto k_u_i = dist(mt);
          k_i2.get<assets_file>().set_user(user_list[k_u_i]);
          k_i2.get<assets_file>().set_department(magic_enum::enum_cast<department>(k_u_i % 8).value());
          k_i2.get<root_ref>().set_root(k_prj);
        }
      }
    }

    SECTION("str meta tree") {
      auto k_reg = g_reg();
      {
        auto k_v = k_reg->view<project>();
        for (auto& k : k_v) {
          std::cout << k_v.get<project>(k).get_name() << std::endl;
        }
      }
      {
        auto k_v = k_reg->view<season>();
        for (auto& k : k_v) {
          std::cout << k_v.get<season>(k).str() << std::endl;
        }
      }

      {
        auto k_v = k_reg->view<episodes>();
        for (auto& k : k_v) {
          std::cout << k_v.get<episodes>(k).str() << std::endl;
        }
      }
      {
        auto k_v = k_reg->view<shot>();
        for (auto& k : k_v) {
          std::cout << k_v.get<shot>(k).str() << std::endl;
        }
      }
      {
        auto k_v = k_reg->view<assets>();
        for (auto& k : k_v) {
          std::cout << k_v.get<assets>(k).str() << std::endl;
        }
      }
    }
    SECTION("install database") {
      auto k_f = doodle_lib::Get().get_metadata_factory();
      for (auto k : g_reg()->view<database_stauts, database>()) {
        k_f->insert_into(k);
      }
    }
  }
}
TEST_CASE("load_meta", "[metadata]") {
  using namespace doodle;
  auto k_server = rpc_server_handle{};
  auto& set     = core_set::getSet();
  k_server.run_server(set.get_meta_rpc_port(), set.get_file_rpc_port());
  doodle_lib::Get().init_gui();
  SECTION("load_meta") {
    auto& k_lib = doodle_lib::Get();
    k_lib.init_gui();
    REQUIRE(k_lib.p_project_vector.size() >= 1);
    auto k   = make_handle(k_lib.p_project_vector.front());

    auto k_f = k_lib.get_metadata_factory();
    k_f->select_indb_by_root(k);

    auto k_reg = g_reg();
    {
      auto k_v = k_reg->view<project>();
      for (auto& k : k_v) {
        std::cout << k_v.get<project>(k).get_name() << std::endl;
      }
    }
    {
      auto k_v = k_reg->view<season>();
      for (auto& k : k_v) {
        std::cout << k_v.get<season>(k).str() << std::endl;
      }
    }

    {
      auto k_v = k_reg->view<episodes>();
      for (auto& k : k_v) {
        std::cout << k_v.get<episodes>(k).str() << std::endl;
      }
    }
    {
      auto k_v = k_reg->view<shot>();
      for (auto& k : k_v) {
        std::cout << k_v.get<shot>(k).str() << std::endl;
      }
    }
    {
      auto k_v = k_reg->view<assets>();
      for (auto& k : k_v) {
        std::cout << k_v.get<assets>(k).str() << std::endl;
      }
    }
  }
}
TEST_CASE("gui action metadata", "[metadata][gui]") {
  //  using namespace doodle;
  //  using namespace doodle::chrono::literals;
  //  auto k_server = RpcServerHandle{};
  //  auto& set     = CoreSet::getSet();
  //  k_server.runServer(set.getMetaRpcPort(), set.getFileRpcPort());
  //
  //  doodle_lib::Get().init_gui();
  //  auto k_fa = std::make_shared<MetadataFactory>();
  //
  //  SECTION("export excel") {
  //    auto k_ex = std::make_shared<actn_export_excel>();
  //
  //    k_ex->sig_get_arg.connect([]() {
  //      actn_export_excel::arg k_arg{};
  //      auto k_time_b = std::make_shared<time_point_wrap>();
  //      k_time_b->set_local_time(chrono::local_days(2021_y / 6 / 1_d));
  //      auto k_time_end = std::make_shared<time_point_wrap>();
  //      k_time_end->set_local_time(chrono::local_days(2021_y / 6 / 30_d));
  //      k_arg.p_time_range = std::make_pair(k_time_b, k_time_end);
  //
  //      k_arg.date = FSys::temp_directory_path() / "doodle_tset";
  //      if (FSys::exists(k_arg.date))
  //        FSys::create_directories(k_arg.date);
  //
  //      return k_arg;
  //    });
  //
  //    (*k_ex)({}, {});
  //  }
}
// TEST(DSTD, map_netDir) {
//   NETRESOURCE resources{};
//   resources.dwType       = RESOURCETYPE_DISK;
//   resources.lpLocalName  = (LPWSTR)L"S:";
//   resources.lpProvider   = 0;
//   resources.lpRemoteName = (LPWSTR)LR"(\\192.168.10.250\public\CangFeng)";
//   DWORD r                = WNetAddConnection2(&resources, NULL, NULL,
//                                CONNECT_TEMPORARY | CONNECT_INTERACTIVE | CONNECT_COMMANDLINE | CONNECT_CRED_RESET);
//   if (r != NO_ERROR) {
//     std::cout << r << std::endl;
//   }
//   ASSERT_TRUE(r == NO_ERROR);
// }
//
// TEST(DSTD, gset_netDir_name) {
//   TCHAR szDeviceName[150];
//   DWORD dwResult, cchBuff = sizeof(szDeviceName);
//
//   dwResult = WNetGetConnection(L"V:", szDeviceName, &cchBuff);
//
//   ASSERT_TRUE(dwResult == NO_ERROR);
//
//   std::wcout << std::wstring{szDeviceName} << std::endl;
//   auto rules_n = SetVolumeLabel(L"V:\\", L"test");
//   if (rules_n == 0) {
//     auto err = GetLastError();
//     std::cout << err << std::endl;
//   }
//   // ASSERT_TRUE(rules_n != 0);
//
//   wchar_t VolumeName[80];
//   auto rules = GetVolumeInformation(L"V:\\", VolumeName, sizeof(VolumeName), NULL, NULL, NULL, NULL, 0);
//   ASSERT_TRUE(rules);
//   std::cout << VolumeName << std::endl;
// }
//
// TEST(DSTD, canclel_netDir) {
//   DWORD r = WNetCancelConnection2(L"S:", CONNECT_UPDATE_PROFILE, true);
//   if (r != NO_ERROR) {
//     std::cout << r << std::endl;
//   }
//   ASSERT_TRUE(r == NO_ERROR);
// }
