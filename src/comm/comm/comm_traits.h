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

namespace comm {

// C++17 detection helpers capturing common communicator API across CommMPI and CommVT
namespace detail {

  // Dummy class and member-function pointer used to instantiate templated send<fn>(...)
  struct DummyClass {
    void dummyMethod();
  };
  inline constexpr auto dummyMemFn = &DummyClass::dummyMethod;

  // Detection idiom: Op<Args...> is well-formed, or nonesuch when it is not
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

  // Resolve instance handle type returned by registerInstanceCollective<T>(T*)
  template <typename Comm, typename T>
  using instance_handle_t =
    decltype(std::declval<Comm&>().template registerInstanceCollective<T>(std::declval<T*>()));

  // Unified send pattern: send<fn>(int dest, Handle proxy, ...)
  template <typename Comm>
  using send_unified_t = decltype(
    std::declval<Comm&>().template send<dummyMemFn>(
      std::declval<int>(),
      std::declval<instance_handle_t<Comm, DummyClass>>()
    )
  );

  // Checks for required member functions

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

} // namespace detail

// Primary check that a communicator must satisfy
template <typename Comm>
inline constexpr bool is_communicator_v =
  detail::HasInit<Comm> &&
  detail::HasFinalize<Comm> &&
  detail::HasNumRanks<Comm> &&
  detail::HasGetRank<Comm> &&
  detail::HasPoll<Comm> &&
  detail::HasRegisterInstanceCollective<Comm, detail::DummyClass> &&
  detail::HasSendUnified<Comm>;

#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
// Concept form for C++20 consumers
template <typename Comm>
concept Communicator = is_communicator_v<Comm>;
#endif

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

  static constexpr bool is_valid = is_communicator_v<Comm>;
};

// Convenience alias to check conformance at compile time
template <typename Comm>
using is_comm_conformant = std::bool_constant<is_communicator_v<Comm>>;

} /* end namespace comm */

#endif /*INCLUDED_COMM_COMM_TRAITS_H*/
