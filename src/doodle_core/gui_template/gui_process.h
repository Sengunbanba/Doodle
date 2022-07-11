//
// Created by TD on 2022/6/21.
//

#pragma once

#include <doodle_core/doodle_core_fwd.h>

#include <doodle_core/logger/logger.h>
#include <boost/asio.hpp>

namespace doodle {
enum class process_state : std::uint8_t {
  run = 1,
  succeed,
  fail
};

class process_handy_tools {
  std::function<void(process_state)> p_f;

 public:
  inline void connect(const std::function<void(process_state)>& in) {
    p_f = in;
  }

  inline void succeed() const {
    if (p_f)
      p_f(process_state::succeed);
  }
  inline void fail() const {
    if (p_f)
      p_f(process_state::fail);
  }
};

template <typename Process_t>
class process_warp_t {
 protected:
  std::any process_p{};

  enum class state : std::uint8_t {
    uninitialized = 0,
    running,
    paused,
    succeeded,
    failed,
    aborted,
    finished,
    rejected
  };
  state current{state::uninitialized};

  template <typename Target = Process_t>
  auto connect()
      -> decltype(std::declval<Target>().connect(
                      std::declval<std::function<void(process_state)>>()),
                  void()) {
    std::any_cast<Target&>(process_p).connect([this](process_state in_) {
      switch (in_) {
        case process_state::run: {
        } break;
        case process_state::succeed: {
          this->succeed();
        } break;
        case process_state::fail: {
          this->fail();
        }
        default:
          break;
      }
    });
  }

  template <typename Target = Process_t>
  auto next(std::integral_constant<state, state::uninitialized>)
      -> decltype(std::declval<Target>().init(), void()) {
    std::any_cast<Target&>(process_p).init();
  }

  template <typename Target = Process_t>
  auto next(std::integral_constant<state, state::running>)
      -> decltype(std::declval<Target>().update(), void()) {
    std::any_cast<Target&>(process_p).update();
  }

  template <typename Target = Process_t>
  auto next(std::integral_constant<state, state::succeeded>)
      -> decltype(std::declval<Target>().succeeded(), void()) {
    std::any_cast<Target&>(process_p).succeeded();
  }

  template <typename Target = Process_t>
  auto next(std::integral_constant<state, state::failed>)
      -> decltype(std::declval<Target>().failed(), void()) {
    std::any_cast<Target&>(process_p).failed();
  }

  template <typename Target = Process_t>
  auto next(std::integral_constant<state, state::aborted>)
      -> decltype(std::declval<Target>().aborted(), void()) {
    std::any_cast<Target&>(process_p).aborted();
  }

  template <typename Target = Process_t>
  auto next(...) {}

  void succeed() {
    if (alive()) {
      current = state::succeeded;
    }
  }

  void fail() {
    if (alive()) {
      current = state::failed;
    }
  }
  void pause() {
    if (current == state::running) {
      current = state::paused;
    }
  }
  void unpause() {
    if (current == state::paused) {
      current = state::running;
    }
  }

 public:
  template <typename... Args>
  explicit process_warp_t(Args&&... in_args)
      : process_p(Process_t{std::forward<Args>(in_args)...}) {}

  explicit process_warp_t(Process_t&& in_ptr)
      : process_p(std::move(in_ptr)){};

  virtual ~process_warp_t() = default;

  void abort(const bool immediately = false) {
    if (alive()) {
      current = state::aborted;

      if (immediately) {
        (*this)();
      }
    }
  }
  [[nodiscard]] bool is_uninitialized() const {
    return current == state::uninitialized;
  }

  [[nodiscard]] bool alive() const {
    return current == state::running || current == state::paused;
  }
  [[nodiscard]] bool finished() const {
    return current == state::finished;
  }
  [[nodiscard]] bool paused() const {
    return current == state::paused;
  }
  [[nodiscard]] bool rejected() const {
    return current == state::rejected;
  }
  virtual void operator()() {
    switch (current) {
      case state::uninitialized:
        next(std::integral_constant<state, state::uninitialized>{});
        current = state::running;
        break;
      case state::running:
        next(std::integral_constant<state, state::running>{});
        break;
      default:
        // suppress warnings
        break;
    }

    // if it's dead, it must be notified and removed immediately
    switch (current) {
      case state::succeeded: {
        next(std::integral_constant<state, state::succeeded>{});
        current = state::finished;
      } break;
      case state::failed: {
        next(std::integral_constant<state, state::failed>{});
        current = state::rejected;
      } break;
      case state::aborted: {
        next(std::integral_constant<state, state::aborted>{});
        current = state::rejected;
      } break;
      default:
        // suppress warnings
        break;
    }
  };
};
template <typename Gui_Process>
using gui_warp_t = process_warp_t<Gui_Process>;

template <typename Rear_Process>
using rear_warp_t = process_warp_t<Rear_Process>;

template <typename Lambda_Process, typename = void>
class lambda_process_warp_t : private Lambda_Process, public process_handy_tools {
 public:
  void connect(const std::function<void(process_state)>& in) {
  }

  template <typename... Args>
  explicit lambda_process_warp_t(Args&&... in_args) : Lambda_Process{std::forward<Args>(in_args)...} {};

  void update() {
    Lambda_Process::operator()();
    this->succeed();
  }
};
namespace detail {
template <typename Executor, typename Process_t>
class rear_adapter_t : public std::enable_shared_from_this<rear_adapter_t<Executor, Process_t>> {
  using value_type = process_warp_t<Process_t>;
  Executor executor;
  value_type process;

  using next_type     = std::shared_ptr<void>;
  using next_fun_type = std::function<void()>;
  next_type next_value;
  next_type* next_ptr;
  next_fun_type next_fun_value;

 public:
  template <typename Executor1, typename... Args,
            std::enable_if_t<
                std::is_convertible_v<Executor1, boost::asio::execution_context>> = true>
  explicit rear_adapter_t(Executor1 in_io, Args&&... in_args)
      : executor(in_io.get_executor()),
        process(std::forward<Args>(in_args)...),
        next_value(),
        next_ptr(&next_value),
        next_fun_value() {}

  template <typename Executor1, typename... Args,
            std::enable_if_t<
                boost::asio::execution::is_executor<Executor1>::value ||
                boost::asio::is_executor<Executor1>::value> = true>
  explicit rear_adapter_t(Executor1 in_io, Args&&... in_args)
      : executor(in_io.get_executor()),
        process(std::forward<Args>(in_args)...),
        next_value(),
        next_ptr(&next_value),
        next_fun_value() {}

  ~rear_adapter_t() override {
    if (process.alive()) {
      process.abort(true);
    }
  }

  void operator()() {
    boost::asio::post(executor, [this,
                                 self_ = this->shared_from_this()]() {
      process();
      if (process.finished()) {
        if (this->next_fun_value)
          this->next_fun_value();
      } else if (process.rejected()) {
        return;
      } else {
        (*this)();
      }
    });
  }
  template <typename Process_t1, typename Executor1, typename... Args1>
  rear_adapter_t& next(Executor1& in_io, Args1&&... in_args) {
    using rear_adapter_ptr = std::shared_ptr<rear_adapter_t<Executor1, Process_t1>>;
    rear_adapter_ptr l_ptr = std::make_shared<rear_adapter_ptr::element_type>(in_io, std::forward<Args1>(in_args)...);
    *next_ptr              = l_ptr;
    next_fun_value         = [l_ptr]() { (*l_ptr)(); };
    next_ptr               = &(l_ptr->next_value);
    return *this;
  }
  template <typename Executor1, typename Fun_t>
  rear_adapter_t& next(Executor1& in_io, Fun_t in_fun) {
    return next<lambda_process_warp_t<Fun_t>>(in_io, in_fun);
  }
};

}  // namespace detail
template <typename Executor, typename Process_t>
class process_adapter {
  using rear_adapter_ptr = std::shared_ptr<::doodle::detail::rear_adapter_t<Executor, Process_t>>;

  rear_adapter_ptr p_ptr;

 public:
  template <typename... Args>
  explicit process_adapter(Args&&... in_args)
      : p_ptr(std::make_shared<rear_adapter_ptr::element_type>(std::forward<Args>(in_args)...)) {}

  template <typename Process_t1, typename... Args>
  process_adapter& next(Args&&... in_args) {
    p_ptr->template next<Process_t1>(std::forward<Args>(in_args)...);
    return *this;
  };

  template <typename... Args>
  process_adapter& next(Args&&... in_args) {
    p_ptr->next(std::forward<Args>(in_args)...);
    return *this;
  };
  void operator()() {
    (*p_ptr)();
  }
};
}  // namespace doodle
