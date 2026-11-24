#ifndef TAG_CONTROL_HPP
#define TAG_CONTROL_HPP

#include <concepts>
#include <cstddef>
#include <utility>

namespace slave::detail {
  // тут реализован интерфейс для контролов -- модуль, что отвечает за то, какое именно поле в VariadicUnion сейчас действительно. В зависимости от структуры нашего юниона, контрол может иметь индекс действительного поля внутри себя или вообще иметь размер в 0 байт. Данный интерфейс позвяолет прописать сам этот контрол с оптимизациями а также пользователю кастомизировать свой теггированный юнион как он захочет 

  template<typename Control, std::size_t I>
  concept is_index_set_exists = requires(Control& mut_ctrl) {
    {mut_ctrl.template set<I>()} noexcept;
  };

  template<typename Control,typename Storage, std::size_t I>
  concept is_storage_set_exists = requires(Control& mut_ctrl, Storage& mut_strg) {
    {mut_ctrl.template set<I>(mut_strg)} noexcept;
  };

  template<typename Control, std::size_t... Is>
  consteval bool check_indexed(std::index_sequence<Is...>) {
    return (is_index_set_exists<Control, Is> && ...);
  };

  template<typename Control, typename Storage, std::size_t... Is> 
  consteval bool check_storaged(std::index_sequence<Is...>) {
    return (is_storage_set_exists<Control, Storage, Is> && ...);
  }

  template<typename Control, std::size_t N>
  concept all_index_sets = check_indexed<Control>(std::make_index_sequence<N>{});

  template<typename Control, typename Storage, std::size_t N>
  concept all_storage_sets = check_storaged<Control, Storage>(std::make_index_sequence<N>{});

  // вот вся вышеописанная ебня нужна для того, чтобы мы могли проверять что метод установки индекса set<> он определен для все типов в варианте

  template<typename Control>
  concept tag_control_base =
    std::is_nothrow_default_constructible_v<Control> &&
    std::is_trivially_copyable_v<Control>; // так как контрол не содержит никакой нетривиальной логики, то должен быть простым POD-классом
  
  template<typename Control, std::size_t N>
  concept indexed_tag_control = tag_control_base<Control> && all_index_sets<Control, N> &&
  requires (const Control& const_ctrl, int& ref) {
    {const_ctrl.get()} noexcept -> std::same_as<std::size_t>;
    {const_ctrl.dispatch([]<std::size_t>() noexcept {})} noexcept -> std::same_as<void>; 
    {const_ctrl.dispatch([]<std::size_t>() noexcept -> int {return 0;})} noexcept -> std::same_as<int>;
    {const_ctrl.dispatch([&ref]<std::size_t>() noexcept -> int& {return ref;})} noexcept -> std::same_as<int&>;
  };

  // метод dispatch отвечает за выполнение определенной функции (через него делается аналог std::visit) для заданного типв, который мы узнаем через шаблонный параметр индекса. dispatch должен выглядеть так
  // template<typename Func>
  // decltype(auto) dispatch(Func&& func) noexcept {
  //  здесь какая-то реализация где вызывается func. внутри реализации выбирается какую спецаилизацию func<I> выбрать
  // }
  // все три диспатча в концепте нужны чтобы проверить что ползователь написать диспатч именно через decltype, и что он возаращет тот же тип, что и сама лямбда

  template<typename Control, typename Storage, std::size_t N>
  concept storaged_tag_control = tag_control_base<Control> && all_storage_sets<Control, Storage, N> &&
  requires (const Control& const_ctrl, const Storage& const_strg, int& ref) {
    {const_ctrl.get(const_strg)} noexcept -> std::same_as<std::size_t>;
    {const_ctrl.dispatch(const_strg, []<std::size_t>() noexcept {})} noexcept -> std::same_as<void>; 
    {const_ctrl.dispatch(const_strg, []<std::size_t>() noexcept -> int {return 0;})} noexcept -> std::same_as<int>;
    {const_ctrl.dispatch(const_strg, [&ref]<std::size_t>() noexcept -> int& {return ref;})} noexcept -> std::same_as<int&>;
  };

  // c контролом без индекса все тоже самое, только в сигнатуре еще добавляется ссылка на сам юнион чтобы по нему определять индекс

  template<typename Control, typename Storage, std::size_t N>
  concept tag_control = indexed_tag_control<Control, N> || storaged_tag_control<Control, Storage, N>;

  template<typename Control, typename Storage>
  concept is_storaged_control = requires(const Control& const_ctrl, const Storage& const_strg) {
    const_ctrl.get(const_strg);
  };

  template<typename Control, typename Storage>
  constexpr std::size_t get(const Control& ctrl, const Storage& strg) noexcept {
    if constexpr (is_storaged_control<Control, Storage>) {
      return ctrl.get(strg);
    } else {
      return ctrl.get();
    }
  }

  template<std::size_t I, typename Control, typename Storage>
  constexpr void set(Control& ctrl, Storage& strg) noexcept {
    if constexpr (is_storaged_control<Control, Storage>) {
      ctrl.template set<I>(strg);
    } else {
      ctrl.template set<I>();
    }
  }

  template<typename Control, typename Storage, typename Func>
  constexpr decltype(auto) dispatch(const Control& ctrl, const Storage& strg, Func&& func) noexcept {
    if constexpr (is_storaged_control<Control, Storage>) {
      return ctrl.dispatch(strg, std::forward<Func>(func));
    } else {
      return ctrl.dispatch(std::forward<Func>(func));
    }
  }


}

#endif
