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
template <typename IO_Context, typename Process_t>
class rear_adapter_t : public std::enable_shared_from_this<rear_adapter_t<IO_Context, Process_t>> {
  using value_type = process_warp_t<Process_t>;
  IO_Context& io_con_p;
  value_type process;

  using next_type     = std::shared_ptr<void>;
  using next_fun_type = std::function<void()>;
  next_type next_value;
  next_type* next_ptr;
  next_fun_type next_fun_value;

 public:
  template <typename... Args>
  explicit rear_adapter_t(IO_Context& in_io, Args&&... in_args)
      : io_con_p(in_io),
        process(std::forward<Args>(in_args)...),
        next_value(),
        next_ptr(&next_value),
        next_fun_value() {}

  void operator()() override {
    boost::asio::post(io_con_p, [this,
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
  template <typename Process_t1, typename IO_Context1, typename... Args1>
  rear_adapter_t& next(IO_Context1& in_io, Args1&&... in_args) {
    using rear_adapter_ptr = std::shared_ptr<rear_adapter_t<IO_Context1, Process_t1>>;
    rear_adapter_ptr l_ptr = std::make_shared<rear_adapter_ptr::element_type>(in_io, std::forward<Args1>(in_args)...);
    *next_ptr              = l_ptr;
    next_fun_value         = [l_ptr]() { (*l_ptr)(); };
    next_ptr               = &(l_ptr->next_value);
  }
  template <typename IO_Context1, typename Fun_t>
  rear_adapter_t& next(IO_Context1& in_io, Fun_t in_fun) {
    return next<lambda_process_warp_t<Fun_t>>(in_io, in_fun);
  }
};
}  // namespace detail





namespace detail {

template <typename IO_Context, typename Rear_Process>
class gui_to_rear_warp_t {
  IO_Context& io_con_p;
  rear_warp_t<Rear_Process> warp_process{};
  std::future<process_state> future_;

  std::function<process_state()> fun_sub{};

 public:
  template <typename... Args>
  explicit gui_to_rear_warp_t(IO_Context& in_io, Args&&... in_args)
      : io_con_p(in_io),
        warp_process(std::forward<Args>(in_args)...),
        future_(),
        fun_sub() {}

  void init() {
    fun_sub = [this]() mutable {
      auto l_s = this->warp_process();
      switch (l_s) {
        case process_state::run: {
          future_ = boost::asio::post(io_con_p, std::packaged_task<process_state()>{this->fun_sub});
          break;
        }
        case process_state::succeed:
        case process_state::fail:
        default:
          break;
      }
      return l_s;
    };
    future_ = boost::asio::post(io_con_p, std::packaged_task<process_state()>{fun_sub});
  }

  process_state update() {
    process_state l_state{process_state::run};
    /// \brief 过程指针有效, 状态为未初始化, 未来无效
    if (future_.valid()) {
      switch (future_.wait_for(0ns)) {
        case std::future_status::ready: {
          try {
            l_state = future_.get();
          } catch (const doodle_error& error) {
            DOODLE_LOG_ERROR(error.what());
            l_state = process_state::fail;
          }
        } break;
        default:
          break;
      }
    }
    return l_state;
  }
};

struct gui_process_wrap_handler;
using instance_type  = std::unique_ptr<void, void (*)(void*)>;
using next_type      = std::unique_ptr<gui_process_wrap_handler>;
using abort_fn_type  = void(gui_process_wrap_handler&, bool);
using update_fn_type = bool(gui_process_wrap_handler&);
struct gui_process_wrap_handler {
  instance_type instance;
  abort_fn_type* abort;
  update_fn_type* update;
  next_type next;
};
template <typename Gui_Process2>
static void delete_gui_process_t(void* in_ptr) {
  delete static_cast<Gui_Process2*>(in_ptr);
};
template <typename Gui_Process2>
static void abort(gui_process_wrap_handler& handler, const bool immediately) {
  static_cast<Gui_Process2*>(handler.instance.get())->abort(immediately);
}
template <typename Gui_Process2>
static bool update(gui_process_wrap_handler& handler) {
  auto&& l_process = *static_cast<Gui_Process2*>(handler.instance.get());

  switch (l_process()) {
    case process_state::run: {
    } break;
    case process_state::succeed: {
      if (handler.next) {
        handler = std::move(*handler.next);
      }
    } break;
    case process_state::fail:
    default:
      return true;
      break;
  }
  return false;
}

}  // namespace detail
class gui_process_t {
 private:
  using instance_type            = detail::instance_type;
  using next_type                = detail::next_type;
  using gui_process_wrap_handler = detail::gui_process_wrap_handler;

  next_type handle;
  next_type* auxiliary_next;

  template <typename type_t, typename... Args>
  gui_process_t& _post_(Args&&... in_args) {
    auto l_next = instance_type{
        new type_t{std::forward<Args>(in_args)...},
        &detail::delete_gui_process_t<type_t>};
    auxiliary_next->reset(new gui_process_wrap_handler{
        std::move(l_next),
        &detail::abort<type_t>,
        &detail::update<type_t>,
        nullptr});
    auxiliary_next = &((*auxiliary_next)->next);
    return *this;
  };

 public:
  explicit gui_process_t()
      : handle{nullptr},
        auxiliary_next(&handle) {}

  template <typename type_t, typename... Args>
  gui_process_t& then(Args&&... in_args) {
    return _post_<gui_warp_t<type_t>>(std::forward<Args>(in_args)...);
  };
  template <typename Func>
  gui_process_t& then(Func&& func) {
    return _post_<gui_warp_t<lambda_process_warp_t<Func>>>(std::forward<Func>(func));
  };
  template <typename type_t, typename IO_Context, typename... Args>
  gui_process_t& post(IO_Context& in_io, Args&&... in_args) {
    return _post_<rear_warp_t<detail::gui_to_rear_warp_t<
        IO_Context, type_t>>>(in_io, std::forward<Args>(in_args)...);
  };
  template <typename IO_Context, typename Func>
  gui_process_t& post(IO_Context& in_io, Func&& func) {
    return _post_<rear_warp_t<detail::gui_to_rear_warp_t<
        IO_Context, lambda_process_warp_t<Func>>>>(in_io, std::forward<Func>(func));
  };
  // 提交时的渲染过程
  bool operator()() {
    return handle->update(*handle);
  }
  void abort(bool in_abort = false) {
    handle->abort(*handle, in_abort);
  }
};
}  // namespace doodle
