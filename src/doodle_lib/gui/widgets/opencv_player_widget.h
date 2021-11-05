//
// Created by TD on 2021/11/04.
//

#pragma once

#include <doodle_lib/doodle_lib_fwd.h>
#include <doodle_lib/gui/base_windwos.h>

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/euml/euml.hpp>
#include <boost/msm/front/euml/state_grammar.hpp>

namespace doodle {
namespace opencv_player_ns {
namespace msm = boost::msm;
using namespace boost::msm::front::euml;

struct play {};
struct stop {};
struct pause {};
struct open_file {};

struct player_ : public msm::front::state_machine_def<player_> {
  struct empty : public msm::front::state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM&) {
      DOODLE_LOG_INFO("empty");
    }
    template <class Event, class FSM>
    void on_exit(Event const&, FSM&) {
      DOODLE_LOG_INFO("empty");
    }
    struct internal_guard_fct {
      template <class EVT, class FSM, class SourceState, class TargetState>
      bool operator()(EVT const& evt, FSM&, SourceState&, TargetState&) {
        std::cout << "Empty::internal_transition_table guard\n";
        return false;
      }
    };
    struct internal_action_fct {
      template <class EVT, class FSM, class SourceState, class TargetState>
      void operator()(EVT const&, FSM&, SourceState&, TargetState&) {
        std::cout << "Empty::internal_transition_table action" << std::endl;
      }
    };
    // Transition table for Empty
    struct internal_transition_table
        : boost::mpl::vector<
              // clang-format off
              //                   Start      Event         Next      Action                Guard
              //                   +---------+-------------+---------+---------------------+----------------------+
              msm::front::Internal<           open_file,              internal_action_fct,  internal_guard_fct>
              // clang-format on

              > {};
  };

  struct stopped : public msm::front::state<> {
  };

  struct playing : public msm::front::state<> {
  };

  struct paused : public msm::front::state<> {
  };
  ///  必须定义初始状态
  using initial_state = empty;

  /// 开始定义转换动作
  struct start_playback {
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM&, SourceState&, TargetState&) {
      std::cout << "player::start_playback" << std::endl;
    }
  };
  struct open_drawer {
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM&, SourceState&, TargetState&) {
      std::cout << "player::open_drawer" << std::endl;
    }
  };
  struct close_drawer {
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM&, SourceState&, TargetState&) {
      std::cout << "player::close_drawer" << std::endl;
    }
  };
  struct store_cd_info {
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM& fsm, SourceState&, TargetState&) {
      std::cout << "player::store_cd_info" << std::endl;
      fsm.process_event(play());
    }
  };
  struct stop_playback {
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM&, SourceState&, TargetState&) {
      std::cout << "player::stop_playback" << std::endl;
    }
  };
  struct pause_playback {
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM&, SourceState&, TargetState&) {
      std::cout << "player::pause_playback" << std::endl;
    }
  };
  struct resume_playback {
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM&, SourceState&, TargetState&) {
      std::cout << "player::resume_playback" << std::endl;
    }
  };
  struct stop_and_open {
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM&, SourceState&, TargetState&) {
      std::cout << "player::stop_and_open" << std::endl;
    }
  };
  struct stopped_again {
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM&, SourceState&, TargetState&) {
      std::cout << "player::stopped_again" << std::endl;
    }
  };

  //定义守卫

  struct dummy_guard {
    template <class EVT, class FSM, class SourceState, class TargetState>
    bool operator()(EVT const& evt, FSM& fsm, SourceState& src, TargetState& tgt) {
      return true;
    }
  };
  struct good_disk_format {
    template <class EVT, class FSM, class SourceState, class TargetState>
    bool operator()(EVT const& evt, FSM&, SourceState&, TargetState&) {
      // to test a guard condition, let's say we understand only CDs, not DVD
      if (false) {
        std::cout << "wrong disk, sorry" << std::endl;
        return false;
      }
      return true;
    }
  };
  struct always_true {
    template <class EVT, class FSM, class SourceState, class TargetState>
    bool operator()(EVT const& evt, FSM&, SourceState&, TargetState&) {
      return true;
    }
  };
  struct internal_guard {
    template <class EVT, class FSM, class SourceState, class TargetState>
    bool operator()(EVT const& evt, FSM&, SourceState&, TargetState&) {
      std::cout << "Empty::internal guard\n";
      return false;
    }
  };

  /// 开始定义转换表
  // struct transition_table
  //     : public boost::mpl::vector<
  //           // clang-format off
  //           //              Start      Event         Next      Action                         Guard
  //           //+------------+----------+-------------+---------+------------------------------+--------------
  //           msm::front::Row<empty,     open_file,    playing,  msm::front::none,              msm::front::none>,

  //           //+------------+----------+-------------+---------+------------------------------+--------------
  //           // msm::front::Row<stopped,   play,         playing,  msm::front::ActionSequence_<
  //           //                                                       boost::mpl::vector<
  //           //                                                          start_playback>>,        dummy_guard     >,
  //           msm::front::Row<stopped,   stop,         stopped,  msm::front::none,              msm::front::none>,
  //           //+------------+----------+-------------+---------+------------------------------+--------------
  //           msm::front::Row<playing,   play,         playing,  msm::front::none,              msm::front::none>,
  //           msm::front::Row<playing,   stop,         stopped,  msm::front::none,              msm::front::none>,
  //           msm::front::Row<playing,   pause,        paused,   msm::front::none,              msm::front::none>,

  //           //+------------+----------+-------------+---------+------------------------------+--------------
  //           msm::front::Row<paused,    stop,         stopped,  msm::front::none,              msm::front::none>,
  //           msm::front::Row<paused,    play,         playing,  msm::front::none,              msm::front::none>
  //           // clang-format on
  //           > {};

  // 替换默认的无转换响应
  template <class FSM, class Event>
  void no_transition(Event const& e, FSM&, int state) {
    std::cout << "no transition from state " << state
              << " on event " << typeid(e).name() << std::endl;
  }
};
using player = msm::back::state_machine<player_>;

}  // namespace opencv_player_ns

class DOODLELIB_API opencv_player_widget : public base_widget {
 public:
  opencv_player_widget();
  void frame_render() override;
};
}  // namespace doodle
