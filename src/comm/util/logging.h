/*
//@HEADER
// *****************************************************************************
//
//                                logging.h
//                 DARMA/comm => Communicator
//
// Copyright 2019-2024 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// * Redistributions in binary form, must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact darma@sandia.gov
//
// *****************************************************************************
//@HEADER
*/

#if !defined INCLUDED_COMM_UTIL_LOGGING_H
#define INCLUDED_COMM_UTIL_LOGGING_H

#include <fmt/format.h>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <cstdio>

namespace comm::util {

enum class Verbosity : int {
  terse = 0,
  normal = 1,
  verbose = 2
};

namespace detail {
struct ComponentState;
}

/**
 * Component handles are created by registerComponent(). A default-constructed
 * handle is invalid and never produces output.
 */
class Component {
public:
  Component() = default;

  explicit operator bool() const noexcept { return state_ != nullptr; }

  friend bool operator==(Component const&, Component const&) = default;

private:
  explicit Component(std::shared_ptr<detail::ComponentState> state)
    : state_(std::move(state))
  { }

  std::shared_ptr<detail::ComponentState> state_;

  friend Component registerComponent(std::string, bool);
  friend std::optional<Component> findComponent(std::string_view);
  friend void enable(Component const&);
  friend void disable(Component const&);
  friend bool isEnabled(Component const&);
  friend std::string_view componentName(Component const&);
  friend std::string_view componentColorName(Component const&);
};

/**
 * Register a component name and return its stable handle.
 *
 * Registration is idempotent: registering the same name again returns the
 * original handle, and the first registration determines its initial enabled
 * state. Empty names are rejected with std::invalid_argument.
 */
Component registerComponent(std::string name, bool initially_enabled = false);

/** Find a previously registered component without implicitly creating it. */
std::optional<Component> findComponent(std::string_view name);

// Built-in components use the same registry as components added by users.
Component const& communicatorComponent();
Component const& loadBalancerComponent();
Component const& clustererComponent();
Component const& visualizerComponent();
Component const& terminationComponent();

void setVerbosity(Verbosity v);
Verbosity getVerbosity();

void enableAll();
void disableAll();

void enable(Component const& c);
void disable(Component const& c);
bool isEnabled(Component const& c);

/** Enable or disable a component by registered name. Returns false if unknown. */
bool enable(std::string_view name);
bool disable(std::string_view name);
bool isEnabled(std::string_view name);

// Rank provider API
using RankProvider = int(*)();
void setRankProvider(RankProvider rp);
void clearRankProvider();
RankProvider getRankProvider();

// Helpers to print names
std::string_view componentName(Component const& c);
std::string_view verbosityName(Verbosity v);

// Color toggle
void setColorEnabled(bool enabled);
bool getColorEnabled();

// Colored helpers
std::string_view componentColorName(Component const& c);
std::string_view verbosityColorName(Verbosity v);
std::string prefixColor();

/**
 * Log a message formatted via fmt when the component is enabled and the current
 * verbosity is >= msg_verbosity. Prefix includes component, verbosity, and rank (if available).
 */
template <typename... Args>
inline void log(Component const& comp, Verbosity msg_verbosity, std::string_view fmt_str, Args&&... args) {
  if (isEnabled(comp) && static_cast<int>(getVerbosity()) >= static_cast<int>(msg_verbosity)) {
    auto const comp_str = getColorEnabled() ? componentColorName(comp) : componentName(comp);
    auto const verb_str = getColorEnabled() ? verbosityColorName(msg_verbosity) : verbosityName(msg_verbosity);
    auto const prefix = getColorEnabled() ? prefixColor() :  "COMM:";
    if (auto const rank_provider = getRankProvider()) {
      auto const r = rank_provider();
      fmt::print("{} [{}] ({}) {}: ", prefix, r, verb_str, comp_str);
    } else {
      fmt::print("{} ({}) {}: ", prefix, verb_str, comp_str);
    }
    fmt::print(fmt::runtime(fmt_str), std::forward<Args>(args)...);
    fflush(stdout);
  }
}

/** Log through a registered name. Unknown names intentionally produce no output. */
template <typename... Args>
inline void log(std::string_view component_name, Verbosity msg_verbosity, std::string_view fmt_str, Args&&... args) {
  if (auto const component = findComponent(component_name)) {
    log(*component, msg_verbosity, fmt_str, std::forward<Args>(args)...);
  }
}
} /* end namespace comm::util */

// Log with either a Component handle/expression or a registered string name.
#define COMM_LOG(component, mode, ...) \
  ::comm::util::log((component), ::comm::util::Verbosity::mode, __VA_ARGS__)

#endif /*INCLUDED_COMM_UTIL_LOGGING_H*/
