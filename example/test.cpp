/******************************************************************************

Welcome to GDB Online.
GDB online is an online compiler and debugger tool for C, C++, Python, Java, PHP, Ruby, Perl,
C#, OCaml, VB, Swift, Pascal, Fortran, Haskell, Objective-C, Assembly, HTML, CSS, JS, SQLite, Prolog.
Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <iostream>
#include <vector>

template <typename T>
class uninit_allocator : public std::allocator<T> {
  typedef std::allocator<T> A;
  typedef std::allocator_traits<A> a_t;

 public:
  using A::A;

  explicit uninit_allocator(const A& a) : A{a} {}
  explicit uninit_allocator(A&& a) : A{std::move(a)} {}
  uninit_allocator(const uninit_allocator&) = default;
  uninit_allocator(uninit_allocator&&) noexcept = default;
  uninit_allocator& operator=(uninit_allocator&&) noexcept = default;
  uninit_allocator& operator=(const uninit_allocator&) = default;

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


int main()
{
  std::vector<char, uninit_allocator<char>> data(10);
  std::vector<char> data2(10);

  std::cout << data.size() << std::endl;

  for (size_t i = 0; i < data.size(); ++i) {
    std::cout << (int)data[i] << " " << (int)data2[i] << std::endl;
  }

  return 0;
}