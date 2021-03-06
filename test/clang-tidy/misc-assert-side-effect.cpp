// RUN: $(dirname %s)/check_clang_tidy.sh %s misc-assert-side-effect %t -config="{CheckOptions: [{key: misc-assert-side-effect.CheckFunctionCalls, value: 1}, {key: misc-assert-side-effect.AssertMacros, value: 'assert,my_assert'}]}" -- -fexceptions
// REQUIRES: shell

//===--- assert definition block ------------------------------------------===//
int abort() { return 0; }

#ifdef NDEBUG
#define assert(x) 1
#else
#define assert(x)                                                              \
  if (!(x))                                                                    \
  (void)abort()
#endif

#ifdef NDEBUG
#define my_assert(x) 1
#else
#define my_assert(x)                                                           \
  ((void)((x) ? 1 : abort()))
#endif

#ifdef NDEBUG
#define not_my_assert(x) 1
#else
#define not_my_assert(x)                                                       \
  if (!(x))                                                                    \
  (void)abort()
#endif
//===----------------------------------------------------------------------===//

class MyClass {
public:
  bool badFunc(int a, int b) { return a * b > 0; }
  bool goodFunc(int a, int b) const { return a * b > 0; }

  MyClass &operator=(const MyClass &rhs) { return *this; }

  int operator-() { return 1; }

  operator bool() const { return true; }

  void operator delete(void *p) {}
};

bool freeFunction() {
  return true;
}

int main() {

  int X = 0;
  bool B = false;
  assert(X == 1);

  assert(X = 1);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: found assert() with side effect [misc-assert-side-effect]
  my_assert(X = 1);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: found my_assert() with side effect
  not_my_assert(X = 1);

  assert(++X);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: found assert() with side effect
  assert(!B);

  assert(B || true);

  assert(freeFunction());
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: found assert() with side effect

  MyClass mc;
  assert(mc.badFunc(0, 1));
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: found assert() with side effect
  assert(mc.goodFunc(0, 1));

  MyClass mc2;
  assert(mc2 = mc);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: found assert() with side effect

  assert(-mc > 0);

  MyClass *mcp;
  assert(mcp = new MyClass);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: found assert() with side effect

  assert((delete mcp, false));
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: found assert() with side effect

  assert((throw 1, false));
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: found assert() with side effect

  return 0;
}
