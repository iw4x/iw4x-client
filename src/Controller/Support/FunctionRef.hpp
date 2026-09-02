#pragma once

#include "../Types.hpp"

namespace Controller
{
  // P0792R14
	//
	// TODO: remove when wine-msvc catch up.
  //
  template <auto V>
  struct nontype_t
  {
    explicit nontype_t () = default;
  };

  template <auto V>
  inline constexpr nontype_t<V> nontype {};

  namespace detail
  {
    template <typename>
    inline constexpr bool is_nontype = false;

    template <auto V>
    inline constexpr bool is_nontype<nontype_t<V>> = true;

    template <bool Noexcept, typename R, typename... Args>
    struct invocable_using
    {
      template <typename... T>
      static constexpr bool value = std::is_invocable_r_v<R, T..., Args...>;
    };

    template <typename R, typename... Args>
    struct invocable_using<true, R, Args...>
    {
      template <typename... T>
      static constexpr bool value = std::is_nothrow_invocable_r_v<R, T..., Args...>;
    };

    union bound_entity
    {
      const void* object;
      void (*function) ();

      constexpr bound_entity () noexcept
        : object (nullptr) {}

      constexpr explicit bound_entity (const void* o) noexcept
        : object (o) {}

      constexpr explicit bound_entity (void (*f) ()) noexcept
        : function (f) {}
    };

    template <bool Const, bool Noexcept, typename R, typename... Args>
    class function_ref_base
    {
      template <typename... T>
      static constexpr bool invocable_with =
        invocable_using<Noexcept, R, Args...>::template value<T...>;

      template <typename T>
      using cv_t = std::conditional_t<Const, const T, T>;

    public:
      template <typename F>
        requires (std::is_function_v<F> && invocable_with<F*>)
      function_ref_base (F* f) noexcept
        : entity_ (reinterpret_cast<void (*) ()> (f)),
          thunk_ ([] (bound_entity e, Args... args) noexcept (Noexcept) -> R
          {
            return std::invoke (reinterpret_cast<F*> (e.function),
                                std::forward<Args> (args)...);
          })
      {
        assert (f != nullptr);
      }

      template <typename F, typename T = std::remove_reference_t<F>>
        requires (!std::is_same_v<std::remove_cvref_t<F>, function_ref_base> &&
                  !detail::is_nontype<std::remove_cvref_t<F>> &&
                  !std::is_member_pointer_v<T> &&
                  invocable_with<cv_t<T>&>)
      constexpr function_ref_base (F&& f) noexcept
        : entity_ (static_cast<const void*> (std::addressof (f))),
          thunk_ ([] (bound_entity e, Args... args) noexcept (Noexcept) -> R
          {
            cv_t<T>& target (
              *static_cast<cv_t<T>*> (const_cast<void*> (e.object)));

            return std::invoke (target, std::forward<Args> (args)...);
          })
      {
      }

      template <auto f>
        requires (invocable_with<decltype (f)>)
      constexpr function_ref_base (nontype_t<f>) noexcept
        : entity_ (),
          thunk_ ([] (bound_entity, Args... args) noexcept (Noexcept) -> R
          {
            return std::invoke (f, std::forward<Args> (args)...);
          })
      {
        if constexpr (std::is_pointer_v<decltype (f)> ||
                      std::is_member_pointer_v<decltype (f)>)
          static_assert (f != nullptr,
                         "a function_ref bound to a null pointer has nothing to "
                         "call");
      }

      template <auto f, typename U, typename T = std::remove_reference_t<U>>
        requires (!std::is_rvalue_reference_v<U&&> &&
                  invocable_with<decltype (f), cv_t<T>&>)
      constexpr function_ref_base (nontype_t<f>, U&& obj) noexcept
        : entity_ (static_cast<const void*> (std::addressof (obj))),
          thunk_ ([] (bound_entity e, Args... args) noexcept (Noexcept) -> R
          {
            cv_t<T>& target (
              *static_cast<cv_t<T>*> (const_cast<void*> (e.object)));

            return std::invoke (f, target, std::forward<Args> (args)...);
          })
      {
      }

      template <auto f, typename T>
        requires (invocable_with<decltype (f), cv_t<T>*>)
      constexpr function_ref_base (nontype_t<f>, cv_t<T>* obj) noexcept
        : entity_ (static_cast<const void*> (obj)),
          thunk_ ([] (bound_entity e, Args... args) noexcept (Noexcept) -> R
          {
            return std::invoke (f,
                                static_cast<cv_t<T>*> (const_cast<void*> (e.object)),
                                std::forward<Args> (args)...);
          })
      {
      }

      constexpr function_ref_base (const function_ref_base&) noexcept = default;
      constexpr function_ref_base& operator= (const function_ref_base&) noexcept = default;

      template <typename T>
        requires (!std::is_same_v<std::remove_cvref_t<T>, function_ref_base> &&
                  !std::is_pointer_v<T> &&
                  !detail::is_nontype<std::remove_cvref_t<T>>)
      function_ref_base& operator= (T) = delete;

      constexpr R
      operator() (Args... args) const noexcept (Noexcept)
      {
        return thunk_ (entity_, std::forward<Args> (args)...);
      }

    private:
      using thunk_t = R (*) (bound_entity, Args...) noexcept (Noexcept);

      bound_entity entity_;
      thunk_t thunk_;
    };
  }

  template <typename... S>
  class function_ref;

  template <typename R, typename... Args>
  class function_ref<R (Args...)>
    : public detail::function_ref_base<false, false, R, Args...>
  {
    using base = detail::function_ref_base<false, false, R, Args...>;

  public:
    using base::base;
    using base::operator=;
    using base::operator();
  };

  template <typename R, typename... Args>
  class function_ref<R (Args...) const>
    : public detail::function_ref_base<true, false, R, Args...>
  {
    using base = detail::function_ref_base<true, false, R, Args...>;

  public:
    using base::base;
    using base::operator=;
    using base::operator();
  };

  template <typename R, typename... Args>
  class function_ref<R (Args...) noexcept>
    : public detail::function_ref_base<false, true, R, Args...>
  {
    using base = detail::function_ref_base<false, true, R, Args...>;

  public:
    using base::base;
    using base::operator=;
    using base::operator();
  };

  template <typename R, typename... Args>
  class function_ref<R (Args...) const noexcept>
    : public detail::function_ref_base<true, true, R, Args...>
  {
    using base = detail::function_ref_base<true, true, R, Args...>;

  public:
    using base::base;
    using base::operator=;
    using base::operator();
  };

  template <typename F>
    requires std::is_function_v<F>
  function_ref (F*) -> function_ref<F>;
}
