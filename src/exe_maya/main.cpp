//
// Created by td_main on 2023/4/25.
//
#include <doodle_lib/facet/create_move_facet.h>
#include <doodle_lib/facet/main_facet.h>
#include <doodle_lib/facet/rpc_server_facet.h>

#include "maya/MApiNamespace.h"
#include <exe_maya/facet/cloth_sim.h>

// extern "C" int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR strCmdLine, int nCmdShow) try {
extern "C" int main(int argc, const char* const argv[]) try {
  using main_app = doodle::app_command<doodle::maya_plug::cloth_sim>;
  main_app app{argc, argv};
  try {
    return app.run();
  } catch (const std::exception& err) {
    DOODLE_LOG_WARN(boost::diagnostic_information(boost::diagnostic_information(err)));
    return 1;
  }
} catch (...) {
  return 1;
}
