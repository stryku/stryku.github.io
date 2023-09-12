---
layout: post_without_latex
title:  "Mutexes are passé"
date:   2018-08-06 00:00:00 +0200
categories: cpp
published: true
---

# Mutexes are passé

And atomics. Of course. And other synchronization stuff that you know. In this art you'll read about a new, innovatory synchronization method in C++. We'll be using C++17 and clang ~8 (relying on Compiler Explorer).

Once, during regular beforesleep C++ standard reading, I've encountered such interesting paragraph in [stmt.dcl]:

> Dynamic initialization of a block-scope variable with static storage duration or thread storage duration is performed the first time control passes through its declaration; such a variable is considered initialized upon the completion of its initialization. If the initialization exits by throwing an exception, the initialization is not complete, so it will be tried again the next time control enters the declaration. If control enters the declaration concurrently while the variable is being initialized, the concurrent execution shall wait for completion of the initialization.

Once again:

> variable is considered initialized upon the completion of its initialization

> throwing an exception, the initialization is not complete

> concurrently while the variable is being initialized, the concurrent execution shall wait for completion

BENG! That's it! I've immediately forgot about mutexes, atomics etc.


# MCVE

Let's start with something simple. A multithread accumulation.


```cpp
int main()
{
  int sum{};

  auto concurrent_fun = [&sum]
  {
    sum += 3;
  };

  auto fun = [concurrent_fun]
  {
    for(auto i = 0u; i < 100000; ++i)
    {
      sync_call(concurrent_fun);
    }
  };

  auto t1 = std::thread{ fun };
  auto t2 = std::thread{ fun };

  t1.join();
  t2.join();

  std::cout << sum;
}
```

At the point of `cout``, sum should be 600000.

Now let's create a function called e.g. `sync_call` that will take any callable object and will take care of concurrent calls.

```cpp
template <typename Callable>
void sync_call(Callable&& callable)
```

And now, The Magic. First, we need static variable.

```cpp
template <typename Callable>
void sync_call(Callable&& callable)
{
  static auto _ 
```

calling our concurrent callable object should be executed during initialization of the variable => lambda for the rescue.


```cpp
template <typename Callable>
void sync_call(Callable&& callable)
{
  static auto _ = [callable]
  {
    callable();
    return 0;
  }();
}
```

now we need to prevent from actual initialization. Of course by throwing.

```cpp
template <typename Callable>
void sync_call(Callable&& callable)
{
  static auto _ = [callable]
  {
    callable();
    throw 0;
    return 0;
  }();
}
```

and of course, let's prevent from crashing by not caught exception

```cpp
template <typename Callable>
void sync_call(Callable&& callable)
{
  try
  {
    static auto _ = [callable]
    {
      callable();
      throw 0;
      return 0;
    }();
  }
  catch (...)
  {}
}
```

At this point it looks really, really good. But I can imagine a situation, when someone would want to... I don't know, e.g. throw his own exception during our object call. Now we're catching anything. Not cool. Let's create our own exception.

```cpp
namespace details
{
  class mutexes_are_passé : std::exception
  {
  public:
    const char* what() const noexcept override { return "Mutexes are passe"; }
  };
}

template <typename Callable>
void sync_call(Callable&& callable)
{
  try
  {
    static auto _ = [callable]
    {
      callable();
      throw details::mutexes_are_passé{};
      return 0;
    }();
  }
  catch (const details::mutexes_are_passé&)
  {}
}

```

Just right.

And the full code.

```cpp
#include <iostream>
#include <thread>

namespace details
{
  class mutexes_are_passé : std::exception
  {
  public:
    const char* what() const noexcept override { return "Mutexes are passé"; }
  };
}

template <typename Callable>
void sync_call(Callable&& callable)
{
  try
  {
    static auto _ = [callable]
    {
      callable();
      throw details::mutexes_are_passé{};
      return 0;
    }();
  }
  catch (const details::mutexes_are_passé&)
  {}
}

int main()
{
  int sum{};

  auto concurrent_fun = [&sum]
  {
    sum += 3;
  };

  auto fun = [concurrent_fun]
  {
    for(auto i = 0u; i < 100000; ++i)
    {
      sync_call(concurrent_fun);
    }
  };

  auto t1 = std::thread{ fun };
  auto t2 = std::thread{ fun };

  t1.join();
  t2.join();

  std::cout << sum;
}
```

# Final speech

If you read whole art, up to this point, I owe you one more thing.

This code won't compile under gcc by default.



# Thanks for reading o/
