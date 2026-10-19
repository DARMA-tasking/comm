/*
//@HEADER
// *****************************************************************************
//
//                               comm_traits.h
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

#if !defined INCLUDED_COMM_COMM_TRAITS_H
#define INCLUDED_COMM_COMM_TRAITS_H

#include <cstddef>
#include <type_traits>
#include <utility>

#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
#define COMM_HAS_CONCEPTS 1
#else
#define COMM_HAS_CONCEPTS 0
#endif

#if COMM_HAS_CONCEPTS
#include <concepts>
// Constrains a template parameter to a communicator, e.g. template <COMM_COMMUNICATOR C>
#define COMM_COMMUNICATOR comm::Communicator
#else
#define COMM_COMMUNICATOR typename
#endif

namespace comm {

// Helpers capturing common communicator API across CommMPI and CommVT
namespace detail {

  // Dummy class and member-function pointer used to instantiate templated send<fn>(...)
  struct DummyClass {
    void dummyMethod();
  };
  inline constexpr auto dummyMemFn = &DummyClass::dummyMethod;

  // Resolve instance handle type returned by registerInstanceCollective<T>(T*)
  template <typename Comm, typename T>
  using instance_handle_t =
    decltype(std::declval<Comm&>().template registerInstanceCollective<T>(std::declval<T*>()));

#if COMM_HAS_CONCEPTS

  // Concepts for required member functions

  template <typename Comm>
  concept HasInit = requires(Comm c, int& argc, char**& argv) {
    { c.init(argc, argv) };
  };

  template <typename Comm>
  concept HasFinalize = requires(Comm c) {
    { c.finalize() };
  };

  template <typename Comm>
  concept HasNumRanks = requires(Comm const& c) {
    { c.numRanks() } -> std::integral;
  };

  template <typename Comm>
  concept HasGetRank = requires(Comm const& c) {
    { c.getRank() } -> std::integral;
  };

  template <typename Comm>
  concept HasPoll = requires(Comm c) {
    { c.poll() } -> std::convertible_to<bool>;
  };

  // registerInstanceCollective<T>(T*)
  template <typename Comm, typename T>
  concept HasRegisterInstanceCollective = requires(Comm c, T* t) {
    { c.template registerInstanceCollective<T>(t) };
  };

  // Unified send pattern: send<fn>(int dest, Handle proxy, ...)
  template <typename Comm>
  concept HasSendUnified =
    HasRegisterInstanceCollective<Comm, DummyClass> &&
    requires(Comm c) {
      { c.template send<dummyMemFn>(
          std::declval<int>(),
          std::declval<instance_handle_t<Comm, DummyClass>>()
        )
      };
    };

#else

  // Detection idiom: Op<Args...> if well-formed, otherwise nonesuch
  struct nonesuch {};

  template <typename, template <typename...> class Op, typename... Args>
  struct detector {
    using type = nonesuch;
  };

  template <template <typename...> class Op, typename... Args>
  struct detector<std::void_t<Op<Args...>>, Op, Args...> {
    using type = Op<Args...>;
  };

  template <template <typename...> class Op, typename... Args>
  using detected_t = typename detector<void, Op, Args...>::type;

  template <template <typename...> class Op, typename... Args>
  inline constexpr bool is_detected_v =
    !std::is_same_v<detected_t<Op, Args...>, nonesuch>;

  // Expressions for required member functions

  template <typename Comm>
  using init_t =
    decltype(std::declval<Comm&>().init(std::declval<int&>(), std::declval<char**&>()));

  template <typename Comm>
  using finalize_t = decltype(std::declval<Comm&>().finalize());

  template <typename Comm>
  using num_ranks_t = decltype(std::declval<Comm const&>().numRanks());

  template <typename Comm>
  using get_rank_t = decltype(std::declval<Comm const&>().getRank());

  template <typename Comm>
  using poll_t = decltype(std::declval<Comm&>().poll());

  template <typename Comm>
  using send_unified_t = decltype(
    std::declval<Comm&>().template send<dummyMemFn>(
      std::declval<int>(),
      std::declval<instance_handle_t<Comm, DummyClass>>()
    )
  );

  // Boolean equivalents of the C++20 concepts, under the same names

  template <typename Comm>
  inline constexpr bool HasInit = is_detected_v<init_t, Comm>;

  template <typename Comm>
  inline constexpr bool HasFinalize = is_detected_v<finalize_t, Comm>;

  template <typename Comm>
  inline constexpr bool HasNumRanks =
    std::is_integral_v<detected_t<num_ranks_t, Comm>>;

  template <typename Comm>
  inline constexpr bool HasGetRank =
    std::is_integral_v<detected_t<get_rank_t, Comm>>;

  template <typename Comm>
  inline constexpr bool HasPoll =
    std::is_convertible_v<detected_t<poll_t, Comm>, bool>;

  template <typename Comm, typename T>
  inline constexpr bool HasRegisterInstanceCollective =
    is_detected_v<instance_handle_t, Comm, T>;

  template <typename Comm>
  inline constexpr bool HasSendUnified =
    HasRegisterInstanceCollective<Comm, DummyClass> &&
    is_detected_v<send_unified_t, Comm>;

#endif

} // namespace detail

// Primary requirement that a communicator must satisfy
template <typename Comm>
#if COMM_HAS_CONCEPTS
concept Communicator =
#else
inline constexpr bool Communicator =
#endif
  detail::HasInit<Comm> &&
  detail::HasFinalize<Comm> &&
  detail::HasNumRanks<Comm> &&
  detail::HasGetRank<Comm> &&
  detail::HasPoll<Comm> &&
  detail::HasRegisterInstanceCollective<Comm, detail::DummyClass> &&
  detail::HasSendUnified<Comm>;

// Backward-compatible traits shim
template <typename Comm>
struct CommunicatorTraits {
  static constexpr bool has_init = detail::HasInit<Comm>;
  static constexpr bool has_finalize = detail::HasFinalize<Comm>;
  static constexpr bool has_num_ranks = detail::HasNumRanks<Comm>;
  static constexpr bool has_get_rank = detail::HasGetRank<Comm>;
  static constexpr bool has_poll = detail::HasPoll<Comm>;

  template <typename T>
  using instance_handle_t = detail::instance_handle_t<Comm, T>;

  template <typename T>
  static constexpr bool has_register_instance =
    detail::HasRegisterInstanceCollective<Comm, T>;

  static constexpr bool has_send_unified =
    detail::HasSendUnified<Comm>;

  static constexpr bool is_valid = Communicator<Comm>;
};

// Convenience alias to check conformance at compile time
template <typename Comm>
using is_comm_conformant = std::bool_constant<Communicator<Comm>>;

} /* end namespace comm */

#endif /*INCLUDED_COMM_COMM_TRAITS_H*/
