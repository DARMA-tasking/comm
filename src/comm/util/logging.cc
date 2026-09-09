/*
//@HEADER
// *****************************************************************************
//
//                                logging.cc
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
#include <comm/util/logging.h>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace comm::util {

// ANSI colors
constexpr std::string_view RESET = "\033[0m";
constexpr std::string_view FG_BLUE = "\033[34m";
constexpr std::string_view FG_GREEN = "\033[32m";
constexpr std::string_view FG_YELLOW = "\033[33m";
constexpr std::string_view FG_MAGENTA = "\033[35m";
constexpr std::string_view FG_CYAN = "\033[36m";
constexpr std::string_view FG_RED = "\033[31m";
constexpr std::string_view FG_BD_GREEN = "\033[32;1m";

namespace detail {

struct ComponentState {
  ComponentState(std::string in_name, std::string in_color_name, bool in_enabled)
    : name(std::move(in_name)), color_name(std::move(in_color_name)), enabled(in_enabled)
  { }

  std::string const name;
  std::string const color_name;
  bool enabled;
};

} // namespace detail

namespace {

std::string colorizeComponent(std::string const& name) {
  // FNV-1a (Fowler–Noll–Vo) makes a component's color deterministic and
  // independent of the order in which components happen to register.
  std::uint32_t hash = 2166136261u;
  for (auto const character : name) {
    hash ^= static_cast<unsigned char>(character);
    hash *= 16777619u;
  }

  static constexpr std::array<std::string_view, 6> colors = {
    FG_BLUE, FG_GREEN, FG_YELLOW, FG_MAGENTA, FG_CYAN, FG_RED
  };
  return std::string(colors[hash % colors.size()]) + name + std::string(RESET);
}

enum class BuiltinComponent : std::size_t {
  communicator,
  load_balancer,
  clusterer,
  visualizer,
  termination
};

struct BuiltinComponentSpec {
  std::string_view name;
  bool initially_enabled;
};

static constexpr std::array<BuiltinComponentSpec, 5> builtin_components = {{
  {"Communicator", false},
  {"LoadBalancer", true},
  {"Clusterer", true},
  {"Visualizer", true},
  {"Termination", false}
}};

class ComponentRegistry {
public:
  std::shared_ptr<detail::ComponentState> registerComponent(
    std::string name, bool initially_enabled
  ) {
    if (name.empty()) {
      throw std::invalid_argument("logging component name must not be empty");
    }

    if (auto const iter = components_by_name_.find(name); iter != components_by_name_.end()) {
      return iter->second;
    }

    return addComponent(std::move(name), initially_enabled);
  }

  std::shared_ptr<detail::ComponentState> find(std::string_view name) const {
    auto const iter = components_by_name_.find(std::string{name});
    return iter == components_by_name_.end() ? nullptr : iter->second;
  }

  void setAll(bool enabled) {
    for (auto const& component : components_) {
      component->enabled = enabled;
    }
  }

  static ComponentRegistry& instance() {
    static ComponentRegistry registry;
    return registry;
  }

private:
  ComponentRegistry() {
    for (auto const& component : builtin_components) {
      addComponent(std::string{component.name}, component.initially_enabled);
    }
  }

  std::shared_ptr<detail::ComponentState> addComponent(
    std::string name, bool initially_enabled
  ) {
    auto color_name = colorizeComponent(name);
    auto component = std::make_shared<detail::ComponentState>(
      std::move(name), std::move(color_name), initially_enabled
    );
    components_.push_back(component);
    components_by_name_.emplace(component->name, component);
    return component;
  }

  std::unordered_map<std::string, std::shared_ptr<detail::ComponentState>> components_by_name_;
  std::vector<std::shared_ptr<detail::ComponentState>> components_;
};

Verbosity g_verbosity = Verbosity::normal;
RankProvider g_rank_provider = nullptr;
bool g_color_enabled = true;

template <BuiltinComponent builtin>
Component const& builtinComponent() {
  static constexpr auto spec = builtin_components[static_cast<std::size_t>(builtin)];
  static Component const component = registerComponent(
    std::string{spec.name}, spec.initially_enabled
  );
  return component;
}

} // anonymous namespace

Component registerComponent(std::string name, bool initially_enabled) {
  return Component{
    ComponentRegistry::instance().registerComponent(std::move(name), initially_enabled)
  };
}

std::optional<Component> findComponent(std::string_view name) {
  if (auto component = ComponentRegistry::instance().find(name)) {
    return Component{std::move(component)};
  }
  return std::nullopt;
}

// Convenience for built-in components:

Component const& communicatorComponent() {
  return builtinComponent<BuiltinComponent::communicator>();
}

Component const& loadBalancerComponent() {
  return builtinComponent<BuiltinComponent::load_balancer>();
}

Component const& clustererComponent() {
  return builtinComponent<BuiltinComponent::clusterer>();
}

Component const& visualizerComponent() {
  return builtinComponent<BuiltinComponent::visualizer>();
}

Component const& terminationComponent() {
  return builtinComponent<BuiltinComponent::termination>();
}

// << End of convenience for built-in components.

void setVerbosity(Verbosity v) {
  g_verbosity = v;
}

Verbosity getVerbosity() {
  return g_verbosity;
}

void enableAll() {
  ComponentRegistry::instance().setAll(true);
}

void disableAll() {
  ComponentRegistry::instance().setAll(false);
}

void enable(Component const& c) {
  if (c.state_) {
    c.state_->enabled = true;
  }
}

void disable(Component const& c) {
  if (c.state_) {
    c.state_->enabled = false;
  }
}

bool isEnabled(Component const& c) {
  return c.state_ && c.state_->enabled;
}

bool enable(std::string_view name) {
  if (auto const component = findComponent(name)) {
    enable(*component);
    return true;
  }
  return false;
}

bool disable(std::string_view name) {
  if (auto const component = findComponent(name)) {
    disable(*component);
    return true;
  }
  return false;
}

bool isEnabled(std::string_view name) {
  if (auto const component = findComponent(name)) {
    return isEnabled(*component);
  }
  return false;
}

void setRankProvider(RankProvider rp) {
  g_rank_provider = rp;
}

void clearRankProvider() {
  g_rank_provider = nullptr;
}

RankProvider getRankProvider() {
  return g_rank_provider;
}

void setColorEnabled(bool enabled) {
  g_color_enabled = enabled;
}

bool getColorEnabled() {
  return g_color_enabled;
}

std::string_view componentName(Component const& c) {
  static constexpr std::string_view unknown = "Unknown";
  return c.state_ ? std::string_view{c.state_->name} : unknown;
}

std::string_view verbosityName(Verbosity v) {
  switch (v) {
    case Verbosity::terse: return "terse";
    case Verbosity::normal: return "normal";
    case Verbosity::verbose: return "verbose";
    default: return "unknown";
  }
}

std::string prefixColor() {
  return std::string(FG_BD_GREEN) + std::string("COMM:") + std::string(RESET);
}

std::string_view componentColorName(Component const& c) {
  static const std::string unknown_plain = "Unknown";
  static const std::string unknown_color = std::string(FG_MAGENTA) + "Unknown" + std::string(RESET);
  if (!c.state_) {
    return getColorEnabled() ? std::string_view{unknown_color} : std::string_view{unknown_plain};
  }
  return getColorEnabled() ? std::string_view{c.state_->color_name} : std::string_view{c.state_->name};
}

// Precomputed names for verbosity (plain and colored)
static const std::array<std::string, 4>& verbosityNamesPlain() {
  static const std::array<std::string, 4> names = []{
    std::array<std::string, 4> arr{};
    arr[static_cast<size_t>(Verbosity::terse)]   = std::string(verbosityName(Verbosity::terse));
    arr[static_cast<size_t>(Verbosity::normal)]  = std::string(verbosityName(Verbosity::normal));
    arr[static_cast<size_t>(Verbosity::verbose)] = std::string(verbosityName(Verbosity::verbose));
    arr[3] = "unknown";
    return arr;
  }();
  return names;
}

static const std::array<std::string, 4>& verbosityNamesColor() {
  static const std::array<std::string, 4> names = []{
    std::array<std::string, 4> arr{};
    arr[static_cast<size_t>(Verbosity::terse)]   = std::string(FG_GREEN)  + std::string(verbosityName(Verbosity::terse))   + std::string(RESET);
    arr[static_cast<size_t>(Verbosity::normal)]  = std::string(FG_YELLOW) + std::string(verbosityName(Verbosity::normal))  + std::string(RESET);
    arr[static_cast<size_t>(Verbosity::verbose)] = std::string(FG_MAGENTA)+ std::string(verbosityName(Verbosity::verbose)) + std::string(RESET);
    arr[3] = std::string(FG_MAGENTA) + "unknown" + std::string(RESET);
    return arr;
  }();
  return names;
}

std::string_view verbosityColorName(Verbosity v) {
  const auto idx = static_cast<size_t>(v);
  const auto& names = getColorEnabled() ? verbosityNamesColor() : verbosityNamesPlain();
  return std::string_view{ names[idx < names.size() ? idx : names.size()-1] };
}

} /* end namespace comm::util */
