#pragma once

namespace vt {
  
  template <typename ExitCallback>
  struct ScopeGuard {
    ExitCallback exit_callback_;
    bool disable_ = false;

    ScopeGuard(ExitCallback ec)
      : exit_callback_(ec) {
    };

    ~ScopeGuard() {
      if(!disable_)
        exit_callback_();
    }
  };
}
