#ifndef VAR_UNION_HPP
#define VAR_UNION_HPP

#include <memory>
#include <type_traits>
#include <utility>
#include <cstddef>
#include "detail/error_msgs.hpp"
#include "detail/traits.hpp"


namespace slave {

namespace detail {


/* ядро варианта вариадические юнион. так как, нельзя объявить union<Args...>, то пришлось написать свою версию. суть в том, что она выглядит типа так
 union {
  type field1;
  union {
    type field2;
    union {
      type field3;
     } и дальше
  }
 }



 */

template<typename... Types> // базовый случай рекурсии  
union VariadicUnion {
public:
  constexpr VariadicUnion() noexcept = default;
  constexpr VariadicUnion(const VariadicUnion&) noexcept = default;
  constexpr VariadicUnion& operator=(const VariadicUnion&) noexcept = default;
  constexpr VariadicUnion(VariadicUnion&&) noexcept = default;
  constexpr VariadicUnion& operator=(VariadicUnion&&) noexcept = default;
  constexpr ~VariadicUnion() noexcept = default;
};

template<typename Head, typename... Tail>
union VariadicUnion<Head, Tail...> {
private:
  Head head;
  VariadicUnion<Tail...> tail;

public:

  // следующий спам с конструкторами связан с одной фичей юниона: когда одно из полей юниона не имеет тривиальный дефолтный конструктор, тривиальный деструктор, тривиальный конструктор копирования и перемещения, тривиальный оператор копирования и перемещения юнион просто возьмет и удалит у себя дефолтный конструктор, деструктор, конструктор копирования и перемещения и оператор копирования и перемещения, если те объявлены через = default. так что эти конструкторы и деструктор объявляются через default только если все поля юнионы тривиально дефолтно конструируемы, уничтожаемы, копируемы и перемещаемы в соответствующем методе
  //
  // объявлять же специализации default необходимы, чтобы, если все типы юниона были тривиальными, то и сам тип юниона тоже был тривиальным, что позволит компилятору применить оптимизации типа создания объекта через memset, передача и возвращение объекта через регистры процессора в функции
  
  constexpr VariadicUnion() noexcept
    requires (trivially_nothrow_default_constructible<Head, Tail...>) = default;

  constexpr VariadicUnion(const VariadicUnion&) noexcept
    requires (trivially_nothrow_copy_constructible<Head, Tail...>) = default; 

  constexpr VariadicUnion& operator=(const VariadicUnion&) noexcept
    requires (trivially_nothrow_copy_assignable<Head, Tail...>) = default;

  constexpr VariadicUnion(VariadicUnion&&) noexcept
    requires (trivially_nothrow_move_constructible<Head, Tail...>) = default;

  constexpr VariadicUnion& operator=(VariadicUnion&&) noexcept
    requires (trivially_nothrow_move_assignable<Head, Tail...>) = default;

  constexpr ~VariadicUnion() noexcept 
    requires (trivially_nothrow_destructible<Head, Tail...>) = default;

  // пустой дефолтный конструктор и деструктор, если не все поля юниона тривиальны нужен, потому что, как было сказано выше, если их не объявить компилятор сгенерирует деструктор по умолчанию, а так как юнион не тривиален, то потом удалит этот деструктор, что сделает создание объекта на стеке просто невозможным. дефолтный конструктор нам нужен, потому что тогда наебнется создание самого юниона в методе construct. см. ниже.
  
  constexpr ~VariadicUnion() noexcept {};

  constexpr VariadicUnion() noexcept {};

// этот конструктор необходим, чтобы создавать юнион сразу с инициализированным значением в заданном типе. он будет внутри фабричной обертки для создания варианта 

  template<std::size_t I, typename... Args>
    requires (I == 0 && std::is_nothrow_constructible_v<Head, Args...>)
  constexpr explicit VariadicUnion(std::in_place_index_t<I>, Args&&... args) noexcept 
      : head(std::forward<Args>(args)...) {};

  template<std::size_t I, typename... Args>
    requires (I > 0 && I <= sizeof...(Tail) && std::is_nothrow_constructible_v<Tail...[I - 1], Args...>)
  constexpr explicit VariadicUnion(std::in_place_index_t<I>, Args&&... args) noexcept
      : tail(std::in_place_index_t<I - 1>(), std::forward<Args>(args)...) {};
  


  template<std::size_t I, typename Self>
  constexpr decltype(auto) get(this Self&& self) noexcept {
    static_assert(I <= sizeof...(Tail), index_error_msg);
    if constexpr (I == 0) {
      return (std::forward<Self>(self).head); 
     // скобки нужны, потому что иначе вернется просто тип по значению, а так мы возвращаем lvalue или rvalue 
    } else if constexpr (I <= sizeof...(Tail)) {
      return std::forward<Self>(self).tail.template get<I - 1>();
    }
  }

  template<std::size_t I, typename... Args>
  constexpr auto& construct(Args&&... args) noexcept {
    static_assert(I <= sizeof...(Tail), index_error_msg);
    if constexpr (I == 0) {
      static_assert(nothrow_constructible<Head, Args...>, nothrow_constr_error_msg);
      return *std::construct_at(std::addressof(head), std::forward<Args>(args)...);
    } else if constexpr (I <= sizeof...(Tail)) {
      static_assert(nothrow_constructible<Tail...[I - 1], Args...>, nothrow_constr_error_msg);
      std::construct_at(std::addressof(tail)); // нужно, потому что мы спускаемся вглубь рекурсии и мы не можем создать поле в юнионе, который сам не был создан. собственно поэтому и нужен дефолтный пустой конструктор.
      return tail.template construct<I - 1>(std::forward<Args>(args)...);
    }
  }

  template<std::size_t I>
  constexpr void destroy() noexcept {
    static_assert(I <= sizeof...(Tail), index_error_msg);
    if constexpr (I == 0) {
      static_assert(nothrow_destructible<Head>, nothrow_dest_error_msg);
      std::destroy_at(std::addressof(head));
    } else if constexpr (I <= sizeof...(Tail)) {
      static_assert(nothrow_destructible<Tail...[I - 1]>, nothrow_dest_error_msg);
      tail.template destroy<I - 1>();
    }
  }

};
};
};

#endif
