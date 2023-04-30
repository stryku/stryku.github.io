---
layout: post
title:  "Generic toString()"
date:   2017-07-29 00:00:00 +0200
categories: 
published: true
---

# Table of Contents:
1. This will become a table of contents.
{:toc}

# Abstract

I want to point that I'm aware that spdlog handles classes with overloaded `operator<<`. I'm using `toString()` not only for logging, but also to e.g. write xmls.

Recently in my hobby project there was a need to add logs. I was using spdlog which provides quite user-friendly interface to log various types. For example, you can write something like this:


```cpp
const std::string foo{ "foo" };
const auto bar{ 20. };
const auto baz{ 6 };
logger.info("Foo: {}, bar: {}, baz: {}", foo, bar, baz);
```

Generally it handles basic types, so we can get rid of boilerplates with `std::to_string()` etc.
It's enought most of the times. Ye, most of the times..

What about such simple thing as bool? I'd really like to write `logger.info("condition: {}", true/false)` and expect `"condition: true/false"`, not `"condition: 1/0"`

# Expectations

I wrote generic `toString()` in the project to meet couple of expectations:

- remove boilerplates e.g. `std::to_string(int)`, `condition ? "true" : "false"` 
- cover all primitives and widely used STL types like `std::string` 
- easily add support for my own classes e.g. `struct Pos { int x, y; };`


# bool

```cpp
std::string toString(bool b)
{
    return b ? "true" : "false";
}
```

I think there's no explanation needed. Let's move on.

# Primitives

`bool` was easy, we didn't even use SFINAE. Primitives support will be easy too. We'll take advantage of `std::to_string`, but first, we need to make sure that for given type `T` we can call mentioned `std::to_string`:

```cpp
template <typename T>
std::enable_if_t<std::is_integral_v<T>::value ||
                 std::is_unsigned_v<T>::value ||
                 std::is_floating_point_v<T>::value,
                 std::string>
    toString(const T& value)
{
    return std::to_string(value);
}
```

Looks good, ye? Well.. not exactly. We need to keep in mind that std::is_integral::value is true also for bools, so we need to handle this properly:

```cpp
template <typename T>
std::enable_if_t<!std::is_same_v<T, bool> &&
                 (std::is_integral_v<T> ||
                  std::is_unsigned_v<T> ||
                  std::is_floating_point_v<T>),
                 std::string>
    toString(const T& value)
{
    return std::to_string(value);
}
```

Now it should work like a charm, but my all three neurons that are responsible for aestetic taste are crying seeying this. Let's move it to some helper..

```cpp
namespace details
{
    template <typename T>
    constexpr auto canCallStdString = !std::is_same_v<T, bool> &&
                                      (std::is_integral_v<T> ||
                                       std::is_unsigned_v<T> ||
                                       std::is_floating_point_v<T>);
}

template <typename T>
std::enable_if_t<details::canCallStdString<T>, std::string>
    toString(const T& value)
{
    return std::to_string(value);
}
```

that's better. Moving right along.

# std::string

Now let's support `std::string` and everything that can be converted to it. Easy peasy:

```cpp
template <typename T>
std::enable_if_t<std::is_convertible_v<T, std::string>, std::string>
    toString(const T& value)
{
    return { value };
}
```



# Custom classes support

At this point we covered two of the expectations:
- + remove boilerplates e.g. `std::to_string(int)`, `condition ? "true" : "false"`
- + cover all primitives and widely used STL types like `std::string`
- - easily add support for my own classes.

In my project I made a decision that class will have a `std::string toString() const` method. With that assumption we need to simply detect if given type has such method. If so, just return `object.toString();`

```cpp
//big credits to this guy, for hints: https://stackoverflow.com/a/16824239/4175394
template <typename Type>
struct HasToString {
private:
    template<typename T>
    static constexpr auto check(T*) -> typename std::is_same<
                                                             decltype(std::declval<T>().toString()), 
                                                             std::string
                                                            >::type;

    template<typename>
    static constexpr std::false_type check(...);

    using type = decltype(check<Type>(nullptr));

public:
    static constexpr bool value = type::value;
};
```

What do we have here? We want to check if `Type` has `toString()`. We're using expression SFINAE to achieve that.

In private section we have two overloads of a template `check()` method. First one's signature is that it'd return `std::true_type`, second `std::false_type`. But how compiler 'knows' that it should prefer first one when `Type` has `toString()`? Thanks to partial ordering.

tl;dr
_(...) If more than one specialization matches, partial order rules are used to determine which specialization is more specialized. The most specialized specialization is used, if it is unique (...)_

In our case we have method that'll accept any parameters: `template <typename> static constexpr std::false_type check(...);`

and method that takes only a pointer `template <typename T> static constexpr auto check(T*)`

Thanks to the partial ordering, compiler will first try to use the one with pointer parameter. Then it tries to deduce return type

```cpp
std::is_same<
             decltype(std::declval<T>().toString()), 
             std::string
            >::type;
```

If `T` has `toString()`, decltype will succedd, then if `T::toString()` returns `std::string`, `std::is_same::type` will be a `std::true_type`, if no - `std::false_type`.

What if `T` doesn't have `toString()`? Like I said, compiler tries. If given type won't have `toString()`, `decltype(std::declval().toString())` will fail. Compiler won't consider it as a compilation error thanks to the SFINAE and will go to next check method.

Next method is like a devnull, it accepts everything but 'returns' `std::false_type`.

To summarize that:

- If Type has `std::string toString()`, compiler will select check which returns `std::true_type`.
- If it doesn't, compiler will select check which returns `std::false_type`

Ok. That was the hard part. Now the only thing to do is to create an alias to `check`'s return type and a public bool value which will be used in the `enable_if`.

With explained helper we are ready to write next `toString()` overload. (To remove bolierplate I've created a helper)

```cpp
namespace details
{
    template <typename Type>
    struct HasToString {
    private:
        template<typename T>
        static constexpr auto check(T*) -> typename std::is_same<
                                                                 decltype(std::declval<T>().toString()), 
                                                                 std::string
                                                                >::type;

        template<typename>
        static constexpr std::false_type check(...);

        using type = decltype(check<Type>(nullptr));

    public:
        static constexpr bool value = type::value;
    };
}

template <typename T>
constexpr auto HasToString = details::HasToString<T>::value;

template <typename T>
std::enable_if_t<HasToString<T>, std::string>
    toString(const T& value)
{
    return value.toString();
}
```

# Homework - enums

With all that knowledge, we can easily write toString() which will be able to return pretty-string enum values. Consider it as your homework (:

# Example

Go try it for yourself on Wandbox or wherever you want (:

```cpp
#include <iostream>
#include <string>
#include <type_traits>

template <typename T>
std::enable_if_t<std::is_convertible_v<T, std::string>, std::string>
    toString(const T& value)
{
    return { value };
}

namespace details
{
    template <typename T>
    constexpr auto canCallStdString = !std::is_same_v<T, bool> &&
                                      (std::is_integral_v<T> ||
                                       std::is_unsigned_v<T> ||
                                       std::is_floating_point_v<T>);
}

template <typename T>
std::enable_if_t<details::canCallStdString<T>, std::string>
    toString(const T& value)
{
    return std::to_string(value);
}

std::string toString(bool b)
{
    return b ? "true" : "false";
}

namespace details
{
    template <typename Type>
    struct HasToString {
    private:
        template<typename T>
        static constexpr auto check(T*) -> typename std::is_same<
                                                                 decltype(std::declval<T>().toString()), 
                                                                 std::string
                                                                 >::type;

        template<typename>
        static constexpr std::false_type check(...);

        using type = decltype(check<Type>(nullptr));

    public:
        static constexpr bool value = type::value;
    };
}

template <typename T>
constexpr auto HasToString = details::HasToString<T>::value;

template <typename T>
std::enable_if_t<HasToString<T>, std::string>
    toString(const T& value)
{
    return value.toString();
}


struct Pos
{
    int x, y;

    std::string toString() const
    {
        return "x: " + std::to_string(x) + ", y: " + std::to_string(y);
    }  
};


template <typename ...Args>
void print(const Args&... args)
{
    ( std::cout<< ... << (toString(args) + "\n") );
}

int main()
{
    int foo{ -20 };
    double bar{ 13.22 };
    bool baz{ false };
    std::string qux{ "qux" };
    Pos alohomora{ 20, 40 };

    print(foo, bar, baz, qux, "Pos: ", alohomora);

    return 0;
}
```

# The Review

Before releasing above art, I gave it to my collegues to review, and they were like "...hmm, yee, nice art, nice code, but generally WTF? You can simplify this even more..." and we ended up with this:

```cpp
#include <iostream>
#include <string>
#include <utility>
#include <type_traits>

auto toString(bool b)
{
    return b ? "true" : "false";
}

template <typename T>
auto toString(T&& val) -> decltype(std::to_string(std::declval<T>()))
{
    return std::to_string(val);
}

template <typename T>
auto toString(T&& val) -> decltype(std::string(std::declval<T>()))
{
    return val;
}

template <typename T>
auto toString(T&& val) -> decltype(std::declval<T>().toString())
{
    return val.toString();
}

//////////////////////////////////////////////////

struct Pos
{
    int x, y;

    std::string toString() const
    {
        return "x: " + std::to_string(x) + ", y: " + std::to_string(y);
    }
};


template <typename ...Args>
void print(const Args&... args)
{
    (std::cout << ... << (toString(args) + std::string{ "\n" }));
}

int main()
{
    int foo{ -20 };
    double bar{ 13.22 };
    bool baz{ false };
    std::string qux{ "qux" };
    Pos alohomora{ 20, 40 };

    print(foo, bar, baz, qux, "Pos: ", alohomora);

    return 0;
}
```

# Summary

Once again I learned that if you have a problem in C++ and you want to solve it with metaprogramming, you need to complicate the problem to the state when metaprogramming solution won't work out of the box. Or when it won't work out of the couple-of-hours-boxes. Only then satisfaction will prevail beauty of reading template compilation errors.



# Thanks for reading o/
