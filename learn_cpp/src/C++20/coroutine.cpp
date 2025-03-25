#include <coroutine>
#include <fmt/ostream.h>

struct Coroutine {
  struct promise_type {
    Coroutine get_return_object() {
      return Coroutine{
          std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    // std::suspend_always yield_value(int v) {
    //   _value = v;
    //   return {};
    // }
    void return_value(int v) { _value = v; }
    auto yield_value(int v) {
      _value = v;
      return std::suspend_always{};
    }
    void unhandled_exception() {}

  public:
    int get() const { return _value; }

  private:
    int _value;
  };
  Coroutine(std::coroutine_handle<promise_type> coroutine)
      : _handle(coroutine) {}
  int get() const { return _handle.promise().get(); }
  operator bool() const { return !_handle.done(); }

private:
  std::coroutine_handle<promise_type> _handle;
};

Coroutine generator() {
  fmt::print("Start\n");
  co_yield 888;
  co_yield 999;
  co_return 666;
}

int main() {
  Coroutine c = generator();
  while (c) {
    int value = c.get();
    fmt::print("Value: {}\n", value);
    // c.resume();
  }
}
