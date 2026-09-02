#pragma once

#include "../Types.hpp"

#include <compare>
#include <iterator>
#include <new>
#include <ranges>
#include <stdexcept>

namespace Controller
{
  // P0843R14
  //
	// TODO: remove when wine-msvc catch up.
  //
  struct from_range_t
  {
    explicit from_range_t () = default;
  };

  inline constexpr from_range_t from_range {};

  namespace detail
  {
    template <typename R, typename T>
    concept container_compatible_range =
      std::ranges::input_range<R> &&
      std::convertible_to<std::ranges::range_reference_t<R>, T>;

    struct synth_three_way_fn
    {
      template <typename T, typename U>
        requires requires (const T& a, const U& b)
        {
          {a < b} -> std::convertible_to<bool>;
          {b < a} -> std::convertible_to<bool>;
        }
      constexpr auto
      operator() (const T& a, const U& b) const
      {
        if constexpr (std::three_way_comparable_with<T, U>)
          return a <=> b;
        else
        {
          if (a < b)
            return std::weak_ordering::less;

          if (b < a)
            return std::weak_ordering::greater;

          return std::weak_ordering::equivalent;
        }
      }
    };

    inline constexpr synth_three_way_fn synth_three_way {};

    template <typename T, typename U = T>
    using synth_three_way_result =
      decltype (synth_three_way (std::declval<const T&> (),
                                 std::declval<const U&> ()));

    template <typename T>
    union element
    {
      constexpr element () noexcept
        : empty {} {}

      constexpr ~element () {}

      element (const element&) = delete;
      element& operator= (const element&) = delete;

      struct {} empty;
      T value;
    };

    template <typename T, size_t N, bool = std::is_trivially_destructible_v<T>>
    struct storage
    {
      element<T> elems[N] {};
      size_t count {0};

      constexpr storage () noexcept = default;
    };

    template <typename T, size_t N>
    struct storage<T, N, false>
    {
      element<T> elems[N] {};
      size_t count {0};

      constexpr storage () noexcept = default;

      constexpr ~storage ()
      {
        for (size_t i (0); i != count; ++i)
          std::destroy_at (std::addressof (elems[i].value));
      }

      storage (const storage&) = delete;
      storage& operator= (const storage&) = delete;
    };

    template <typename T, bool Trivial>
    struct storage<T, 0, Trivial>
    {
      size_t count {0};

      constexpr storage () noexcept = default;
    };
  }

  template <typename T, size_t N>
  class inplace_vector: private detail::storage<T, N>
  {
    using base = detail::storage<T, N>;

    constexpr T*
    raw () noexcept
    {
      if constexpr (N == 0)
        return nullptr;
      else
        return std::addressof (this->elems[0].value);
    }

    constexpr const T*
    raw () const noexcept
    {
      if constexpr (N == 0)
        return nullptr;
      else
        return std::addressof (this->elems[0].value);
    }

    [[noreturn]] static void
    overflow ()
    {
      throw std::bad_alloc ();
    }

  public:
    using value_type             = T;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using iterator               = T*;
    using const_iterator         = const T*;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr inplace_vector () noexcept = default;

    constexpr explicit
    inplace_vector (size_type n)
    {
      if (n > N)
        overflow ();

      for (size_type i (0); i != n; ++i)
        std::construct_at (raw () + i);

      base::count = n;
    }

    constexpr
    inplace_vector (size_type n, const T& value)
    {
      if (n > N)
        overflow ();

      for (size_type i (0); i != n; ++i)
        std::construct_at (raw () + i, value);

      base::count = n;
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr
    inplace_vector (I first, S last)
    {
      for (; first != last; ++first)
        emplace_back (*first);
    }

    template <detail::container_compatible_range<T> R>
    constexpr
    inplace_vector (from_range_t, R&& rg)
    {
      for (auto&& e: rg)
        emplace_back (std::forward<decltype (e)> (e));
    }

    constexpr
    inplace_vector (const inplace_vector& other)
    {
      for (const T& e: other)
        unchecked_emplace_back (e);
    }

    constexpr
    inplace_vector (inplace_vector&& other)
      noexcept (N == 0 || std::is_nothrow_move_constructible_v<T>)
    {
      for (T& e: other)
        unchecked_emplace_back (std::move (e));
    }

    constexpr
    inplace_vector (std::initializer_list<T> il)
      : inplace_vector (il.begin (), il.end ()) {}

    constexpr inplace_vector&
    operator= (const inplace_vector& other)
    {
      if (this != std::addressof (other))
        assign (other.begin (), other.end ());

      return *this;
    }

    constexpr inplace_vector&
    operator= (inplace_vector&& other)
      noexcept (N == 0 || (std::is_nothrow_move_assignable_v<T> &&
                           std::is_nothrow_move_constructible_v<T>))
    {
      if (this != std::addressof (other))
      {
        clear ();

        for (T& e: other)
          unchecked_emplace_back (std::move (e));
      }

      return *this;
    }

    constexpr inplace_vector&
    operator= (std::initializer_list<T> il)
    {
      assign (il.begin (), il.end ());
      return *this;
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr void
    assign (I first, S last)
    {
      clear ();

      for (; first != last; ++first)
        emplace_back (*first);
    }

    template <detail::container_compatible_range<T> R>
    constexpr void
    assign_range (R&& rg)
    {
      clear ();

      for (auto&& e: rg)
        emplace_back (std::forward<decltype (e)> (e));
    }

    constexpr void
    assign (size_type n, const T& u)
    {
      if (n > N)
        overflow ();

      clear ();

      for (size_type i (0); i != n; ++i)
        unchecked_emplace_back (u);
    }

    constexpr void
    assign (std::initializer_list<T> il) {assign (il.begin (), il.end ());}

    constexpr iterator
    begin () noexcept {return raw ();}

    constexpr const_iterator
    begin () const noexcept {return raw ();}

    constexpr iterator
    end () noexcept {return raw () + base::count;}

    constexpr const_iterator
    end () const noexcept {return raw () + base::count;}

    constexpr reverse_iterator
    rbegin () noexcept {return reverse_iterator (end ());}

    constexpr const_reverse_iterator
    rbegin () const noexcept {return const_reverse_iterator (end ());}

    constexpr reverse_iterator
    rend () noexcept {return reverse_iterator (begin ());}

    constexpr const_reverse_iterator
    rend () const noexcept {return const_reverse_iterator (begin ());}

    constexpr const_iterator
    cbegin () const noexcept {return begin ();}

    constexpr const_iterator
    cend () const noexcept {return end ();}

    constexpr const_reverse_iterator
    crbegin () const noexcept {return rbegin ();}

    constexpr const_reverse_iterator
    crend () const noexcept {return rend ();}

    constexpr bool
    empty () const noexcept {return base::count == 0;}

    constexpr size_type
    size () const noexcept {return base::count;}

    static constexpr size_type
    max_size () noexcept {return N;}

    static constexpr size_type
    capacity () noexcept {return N;}

    constexpr void
    resize (size_type sz)
    {
      if (sz > N)
        overflow ();

      while (base::count > sz)
        pop_back ();

      while (base::count < sz)
        unchecked_emplace_back ();
    }

    constexpr void
    resize (size_type sz, const T& c)
    {
      if (sz > N)
        overflow ();

      while (base::count > sz)
        pop_back ();

      while (base::count < sz)
        unchecked_emplace_back (c);
    }

    static constexpr void
    reserve (size_type n)
    {
      if (n > N)
        overflow ();
    }

    static constexpr void
    shrink_to_fit () noexcept {}

    constexpr reference
    operator[] (size_type n) noexcept {return raw ()[n];}

    constexpr const_reference
    operator[] (size_type n) const noexcept {return raw ()[n];}

    constexpr reference
    at (size_type n)
    {
      if (n >= base::count)
        throw std::out_of_range ("inplace_vector::at");

      return raw ()[n];
    }

    constexpr const_reference
    at (size_type n) const
    {
      if (n >= base::count)
        throw std::out_of_range ("inplace_vector::at");

      return raw ()[n];
    }

    constexpr reference
    front () noexcept {return raw ()[0];}

    constexpr const_reference
    front () const noexcept {return raw ()[0];}

    constexpr reference
    back () noexcept {return raw ()[base::count - 1];}

    constexpr const_reference
    back () const noexcept {return raw ()[base::count - 1];}

    constexpr T*
    data () noexcept {return raw ();}

    constexpr const T*
    data () const noexcept {return raw ();}

    template <typename... A>
    constexpr reference
    emplace_back (A&&... args)
    {
      if (base::count == N)
        overflow ();

      return unchecked_emplace_back (std::forward<A> (args)...);
    }

    constexpr reference
    push_back (const T& x) {return emplace_back (x);}

    constexpr reference
    push_back (T&& x) {return emplace_back (std::move (x));}

    template <detail::container_compatible_range<T> R>
    constexpr void
    append_range (R&& rg)
    {
      for (auto&& e: rg)
        emplace_back (std::forward<decltype (e)> (e));
    }

    constexpr void
    pop_back () noexcept
    {
      std::destroy_at (raw () + --base::count);
    }

    template <typename... A>
    constexpr pointer
    try_emplace_back (A&&... args)
    {
      if (base::count == N)
        return nullptr;

      return std::addressof (unchecked_emplace_back (std::forward<A> (args)...));
    }

    constexpr pointer
    try_push_back (const T& x) {return try_emplace_back (x);}

    constexpr pointer
    try_push_back (T&& x) {return try_emplace_back (std::move (x));}

    template <detail::container_compatible_range<T> R>
    constexpr std::ranges::borrowed_iterator_t<R>
    try_append_range (R&& rg)
    {
      auto first (std::ranges::begin (rg));
      const auto last (std::ranges::end (rg));

      for (; first != last && base::count != N; ++first)
        unchecked_emplace_back (*first);

      return first;
    }

    template <typename... A>
    constexpr reference
    unchecked_emplace_back (A&&... args)
    {
      assert (base::count < N);

      T* const p (std::construct_at (raw () + base::count,
                                     std::forward<A> (args)...));
      ++base::count;
      return *p;
    }

    constexpr reference
    unchecked_push_back (const T& x) {return unchecked_emplace_back (x);}

    constexpr reference
    unchecked_push_back (T&& x) {return unchecked_emplace_back (std::move (x));}

    template <typename... A>
    constexpr iterator
    emplace (const_iterator position, A&&... args)
    {
      const difference_type at (position - cbegin ());

      emplace_back (std::forward<A> (args)...);
      std::rotate (begin () + at, end () - 1, end ());
      return begin () + at;
    }

    constexpr iterator
    insert (const_iterator position, const T& x) {return emplace (position, x);}

    constexpr iterator
    insert (const_iterator position, T&& x)
    {
      return emplace (position, std::move (x));
    }

    constexpr iterator
    insert (const_iterator position, size_type n, const T& x)
    {
      const difference_type at (position - cbegin ());

      if (base::count + n > N)
        overflow ();

      for (size_type i (0); i != n; ++i)
        unchecked_emplace_back (x);

      std::rotate (begin () + at, end () - static_cast<difference_type> (n), end ());
      return begin () + at;
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr iterator
    insert (const_iterator position, I first, S last)
    {
      const difference_type at (position - cbegin ());
      const difference_type before (static_cast<difference_type> (base::count));

      for (; first != last; ++first)
        emplace_back (*first);

      std::rotate (begin () + at, begin () + before, end ());
      return begin () + at;
    }

    template <detail::container_compatible_range<T> R>
    constexpr iterator
    insert_range (const_iterator position, R&& rg)
    {
      const difference_type at (position - cbegin ());
      const difference_type before (static_cast<difference_type> (base::count));

      for (auto&& e: rg)
        emplace_back (std::forward<decltype (e)> (e));

      std::rotate (begin () + at, begin () + before, end ());
      return begin () + at;
    }

    constexpr iterator
    insert (const_iterator position, std::initializer_list<T> il)
    {
      return insert (position, il.begin (), il.end ());
    }

    constexpr iterator
    erase (const_iterator position) {return erase (position, position + 1);}

    constexpr iterator
    erase (const_iterator first, const_iterator last)
    {
      const difference_type at (first - cbegin ());
      const difference_type n (last - first);

      if (n <= 0)
        return begin () + at;

      std::move (begin () + at + n, end (), begin () + at);

      for (difference_type i (0); i != n; ++i)
        pop_back ();

      return begin () + at;
    }

    constexpr void
    swap (inplace_vector& x)
      noexcept (N == 0 || (std::is_nothrow_swappable_v<T> &&
                           std::is_nothrow_move_constructible_v<T>))
    {
      inplace_vector tmp (std::move (x));
      x = std::move (*this);
      *this = std::move (tmp);
    }

    constexpr void
    clear () noexcept
    {
      while (base::count != 0)
        pop_back ();
    }

    constexpr friend bool
    operator== (const inplace_vector& x, const inplace_vector& y)
    {
      return std::equal (x.begin (), x.end (), y.begin (), y.end ());
    }

    template <typename U = T>
    constexpr friend detail::synth_three_way_result<U>
    operator<=> (const inplace_vector& x, const inplace_vector& y)
    {
      return std::lexicographical_compare_three_way (
        x.begin (), x.end (), y.begin (), y.end (), detail::synth_three_way);
    }

    constexpr friend void
    swap (inplace_vector& x, inplace_vector& y)
      noexcept (N == 0 || (std::is_nothrow_swappable_v<T> &&
                           std::is_nothrow_move_constructible_v<T>))
    {
      x.swap (y);
    }
  };

  template <typename T, size_t N, typename U = T>
  constexpr typename inplace_vector<T, N>::size_type
  erase (inplace_vector<T, N>& c, const U& value)
  {
    const auto it (std::remove (c.begin (), c.end (), value));
    const auto n (static_cast<typename inplace_vector<T, N>::size_type> (
      c.end () - it));

    c.erase (it, c.end ());
    return n;
  }

  template <typename T, size_t N, typename P>
  constexpr typename inplace_vector<T, N>::size_type
  erase_if (inplace_vector<T, N>& c, P pred)
  {
    const auto it (std::remove_if (c.begin (), c.end (), pred));
    const auto n (static_cast<typename inplace_vector<T, N>::size_type> (
      c.end () - it));

    c.erase (it, c.end ());
    return n;
  }
}
