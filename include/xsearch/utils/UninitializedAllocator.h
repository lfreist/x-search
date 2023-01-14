#include <vector>

namespace xs::utils {

template <typename T>
class just_allocator : public std::allocator<T> {
  typedef std::allocator<T> A;
  typedef std::allocator_traits<A> a_t;

 public:
  using A::A;

  explicit just_allocator(const A& a) : A{a} {}
  explicit just_allocator(A&& a) : A{std::move(a)} {}
  just_allocator(const just_allocator&) = default;
  just_allocator(just_allocator&&) noexcept = default;
  just_allocator& operator=(just_allocator&&) noexcept = default;
  just_allocator& operator=(const just_allocator&) = default;

  // overload the construct template with default initialization
  template <typename U>
  void construct(U* ptr) noexcept(
      std::is_nothrow_default_constructible<U>::value) {
    ::new (static_cast<void*>(ptr)) U;
  }
  // initialization with parameters, the construction is then performed by
  // std::allocator<T>.
  template <typename U, typename... Args>
  void construct(U* ptr, Args&&... args) {
    a_t::construct(static_cast<A&>(*this), ptr, std::forward<Args>(args)...);
  }
};

}  // namespace xs::utils