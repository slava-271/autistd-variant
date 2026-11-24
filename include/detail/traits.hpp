#ifndef VAR_TRAITS_HPP
#define VAR_TRAITS_HPP

#include <type_traits>

namespace slave {

namespace detail {

template<typename... Ts>
concept trivially_nothrow_default_constructible =
    (std::is_trivially_default_constructible_v<Ts> && ...) &&
    (std::is_nothrow_default_constructible_v<Ts> && ...);

template<typename... Ts>
concept trivially_nothrow_copy_constructible =
    (std::is_trivially_copy_constructible_v<Ts> && ...) &&
    (std::is_nothrow_copy_constructible_v<Ts> && ...);

template<typename... Ts>
concept trivially_nothrow_move_constructible =
    (std::is_trivially_move_constructible_v<Ts> && ...) &&
    (std::is_nothrow_move_constructible_v<Ts> && ...);

template<typename... Ts>
concept trivially_nothrow_copy_assignable =
    (std::is_trivially_copy_assignable_v<Ts> && ...) &&
    (std::is_nothrow_copy_assignable_v<Ts> && ...);

template<typename... Ts>
concept trivially_nothrow_move_assignable =
    (std::is_trivially_move_assignable_v<Ts> && ...) &&
    (std::is_nothrow_move_assignable_v<Ts> && ...);

template<typename... Ts>
concept trivially_nothrow_destructible =
    (std::is_trivially_destructible_v<Ts> && ...) &&
    (std::is_nothrow_destructible_v<Ts> && ...);

template<typename T, typename... Ts>
concept nothrow_constructible = 
  (std::is_nothrow_constructible_v<T, Ts...>);

template<typename T>
concept nothrow_destructible = std::is_nothrow_destructible_v<T>;

}

}

#endif

