---
layout: post
title:  "ctai: compile-time assembly interpreter"
date:   2017-02-09 00:00:00 +0200
categories: 
published: true
---

Yes, you heard that. A program. A **meta**program. A metaprogram which **interprets a code**. A metaprogram which interprets an **assembly** code. A metaprogram which interprets an assembly code **at compile time**.

![foo](/assets/ctai/llvm_dragon.png)

# Table of Contents:
1. This will become a table of contents.
{:toc}

Interprets assembly code? Well, almost. Ctai is not able to interpret whole Intel's assembly syntax. In fact, there is a lot of other restrictions. Everything will be clear when you'll read the goals that I wanted to achieve.

But first: The No-Goals. Ctai's purposes weren't:
- to be a bulletproof assembly syntax parser. That's why tokens must be separated with one and only one space.
- to be a fully working assembly interpreter. That's why you can't disassemble ready program and just paste is as ctai's execution code.
- to be efficient. That's why computing a 15th Fibonacci's element takes ~three seconds on an i7.

Goals. Ctai's purpose is:
- TO BE FREAKING AWESOME. YOU CAN WRITE ALMOST-ASSEMBLY CODE AND EXECUTE IT AS A C++ METAPROGRAM. ISN'T THAT FREAKING AWESOME?

# Still, WTF?

Here's an example.

```cpp
#include "string.hpp"
#include "execute.hpp"

using code = decltype(
    "mov ebp , esp "
    "push eax " // declare four variables
    "push eax "
    "push eax "
    "push eax "
    "mov DWORD PTR [ ebp + 8 ] , 0 "
    "mov DWORD PTR [ ebp + 12 ] , 1 "
    "mov DWORD PTR [ ebp + 16 ] , 1 "
    "mov DWORD PTR [ ebp + 4 ] , 1 "
":loop_label "
    "mov eax , DWORD PTR [ ebp + 4 ] "
    "mov ebx , 15 " //we want to get 15th fibonacci element
    "cmp eax , ebx "
    "jge .end_label "
    "mov edx , DWORD PTR [ ebp + 8 ] "
    "mov eax , DWORD PTR [ ebp + 12 ] "
    "add eax , edx "
    "mov DWORD PTR [ ebp + 16 ] , eax "
    "mov eax , DWORD PTR [ ebp + 12 ] "
    "mov DWORD PTR [ ebp + 8 ] , eax "
    "mov eax , DWORD PTR [ ebp + 16 ] "
    "mov DWORD PTR [ ebp + 12 ] , eax "
    "mov eax , DWORD PTR [ ebp + 4 ] "
    "mov ebx , 1 "
    "add eax , ebx "
    "mov DWORD PTR [ ebp + 4 ] , eax "
    "jmp .loop_label "
":end_label "
    "mov eax , DWORD PTR [ ebp + 16 ] "
    "exit"_s);

int main()
{
    return ctai::execute_code<code>;
}
```

This code will parse, tokenize, assemble and execute assembly code. In compile time. Compiler will generate such code:

```asm
main:
    mov eax, 610
    ret
```

![foo](/assets/ctai/i_dont_believe_you.gif)

Well, [here's](https://github.com/stryku/ctai/blob/master/all_in_one_fibonacci.cpp) the all-in-one file. Whole ctai code with above example. Paste it to some online compiler (e.g. godbolt.org) and see for yourself (compiled with clang-4.0 and -std=c++1z -O3 flags).


# "I'm probably too stupid :/"

There is a kind of stereotype in C++ community, that templates and metaprogramming are difficult, only experts write and understand these things etc. And at this point there is probably couple of people that think they won't be able to understand this poem. I can assure you that you will. I'm not an expert. In fact, I'm very far from an experts level. So far, that if we'd want to measure that, we'd need to use unit so big, that if we'd omit the calculation error, me and my ferret would be at the same point. I mean, if someone won't understand it, I'll consider this as my personal fail because I didn't explain it correctly. If you'll understand The Basic Trick, explained below, you'll understand whole ctai.

# "So what I need to know?"
Basics of variadic templates, structs and its specializations. All that is in The Basic Trick. That's all.

# The Basic Trick

(If you want the meat, go to: [Tokenizing the code](/2017/02/08/ctai-compile-time-assembly-interpreter-v1.0.html#tokenizing-the-code))


First things first. Before we'll examine actual ctai code I'll present you couple of fundamental tricks. In fact, whole ctai is built from these concepts, merged together.

Let's say that we have a structure `values_container`, defined as follows:
```cpp
template <size_t ...values>
struct values_container
{};
```

And now, we would want to determine whether given `values_container` starts with 1, 2, 3. To do so, let's create another struct called `starts_with_123` , which will accept one template parameter - `values_container`:
```cpp
template <typename container>
struct starts_with_123
{
    static constexpr bool value = ...;
}
```

With only one template parameter - the container, it's hard to initialize `bool value`. We need to know what values the container actually contains. To do so, let's create another struct, which will also accept one template parameter,

```cpp
template <typename>
struct starts_with_123_impl;
```

now, we will specialize that structure to get information about contained values:

```cpp
template <size_t ...values>
struct starts_with_123_impl<values_container<values...>>
{};
```

and finally specialization for our case - when values in values_container starts with 1, 2 and 3:

```cpp
template <size_t ...rest_of_values>
struct starts_with_123_impl<values_container<1, 2, 3, rest_of_values...>>
{};
```

And now we just need to define member bool:

```cpp
template <size_t ...rest_of_values>
struct starts_with_123_impl<values_container<1, 2, 3, rest_of_values...>>
{
     static constexpr bool value = true;
};

template <size_t ...values>
struct starts_with_123_impl<values_container<values...>>
{
    static constexpr bool value = false;
};
```

With such helper, `starts_with_123` could be actually just constexpr bool instead of a structure.

```cpp
template <typename container>
constexpr auto starts_with_123 = starts_with_123_impl<container>::value
```

Whole code would look like this (I've added `details` namespace to get slightly more readable code ):

```cpp
namespace details
{
    template <typename>
    struct starts_with_123_impl;

    template <size_t ...values>
    struct starts_with_123_impl<values_container<1, 2, 3, values...>>
    {
         static constexpr bool value = true;
    };

    template <size_t ...values>
    struct starts_with_123_impl<values_container<values...>>
    {
         static constexpr bool value = false;
    };
}

template <typename container>
constexpr auto starts_with_123 = details::starts_with_123_impl<container>::value;
```

# ctai::string

In this section I'll explain basic structures used in ctai, e.g. string. I know that there are a lot of articles which explain these methods, but I want this poem to be complete and it won't be without string etc. At first, it can look boring, but if you'll understand methods explained here, you'll understand methods used in 90% of ctai.

What is a string? Basically string is a set of characters and ctai's string is not an exception, but method of storing chars differs from that used in e.g. std::string. ctai::string stores its chars as a template parameters. It looks like this:

```cpp
template <char ...chars>
struct string
{
    static constexpr auto size = sizeof...(chars);
};
```

As you can see there is also `static constexpr size`, to easy get known how many characters string contains.

Because of characters stored in the template arguments, each different string is in fact a new type. String "ctai", would be represented by type `ctai::string<'c', 't', 'a', 'i'>`. Yes. By type. Not an object. Type. Every string, every Thing (well almost. We'll get to this) is a type. And that's why ctai is freaking awesome

## String operations

### to_string

```cpp
namespace details
{
    template <typename str>
    struct to_string_impl;

    template <char ...chars>
    struct to_string_impl<string<chars...>>
    {
        using type = string<chars...>;
    };
}

template <typename s>
using to_string = typename details::to_string_impl<s>::type;
```

At first glance it may look useless. You give the string, and receive an alias to the very same string. Why all that? TBH just for more readable and easier to write code (IDE will display hint for `string::size` member). The code is also is less error-prone. With `to_string` you explicitly tell to the compiler that you want the template parameter to be the string and the string only. If you'd pass some type other than the string, compiler will say "Ain't that stupid bro." and hit you with two-A4-page error.

- Q: "But, but.. If you'd pass something other than the string, compilation would fail also, because this other type won't have static constexpr value member."
- A: "Yes you're right. But what if it would have such member? I prefer to play safe, you Jackass."

Anyway, here's a simple use case. You want to write a structure that takes two strings as template parameters, adds its sizes and stores this value.

```cpp
template <typename string1, typename string2>
struct sum_chars_count
{
    static constexpr auto sum = to_string<string1>::size + to_string<string2>::size;
};
```

### string_front

In this case we need to get information about the first character. We can do this by specializing `string_front_impl` in such way.

```cpp
namespace details
{
    template <typename>
    struct string_front_impl;

    template <char front, char ...chars>
    struct string_front_impl<string<front, chars...>>
    {
        static constexpr auto value = front;
    };
}

template <typename s>
constexpr auto string_front = details::string_front_impl<s>::value;
```

From now, I'll drop the boiler plates and explain only code that actually does something. Operations `string_front`, `string_append`, `string_merge` and `string_pop_front` work in a similar way as string_front.


### string_to_int

Declaration looks like this:

```cpp
template <typename, int sign = 1, int current_val = 0>
struct string_to_int_impl;
```

First parameter is passed string, second sign (can be 1 or -1) and the last one is value during conversion.

There are three specializations. First, for an empty string:

```cpp
template <int sign, int current_val>
struct string_to_int_impl<string<>, sign, current_val>
{
    static constexpr auto value = sign * current_val;
};
```

Second, for unsigned value. We know that it's unsigned because first character of the string is not specialized as '-'. You may wonder, why won't we specialize sign to be simply 1? That's because specialization for signed value will pass there a -1. You'll later see an example explaining this.

```cpp
template <int sign, int current_val, char current_char, char ...chars>
struct string_to_int_impl<string<current_char, chars...>, sign, current_val>
{
    static constexpr auto nex_val = current_val * 10 + (current_char - '0');
    static constexpr auto value = string_to_int_impl<string<chars...>, sign, nex_val>::value;
};
```

Aaand the specialization for signed value. As you can see, we're explicitly saying that string needs to have '-' at the beginning. In the body we're computing next `current_val` and recursively instantiating `string_to_int_impl` struct with string without the '-', -1 as a sign and that next value.

```cpp
template <int sign, int current_val, char current_char, char ...chars>
struct string_to_int_impl<string<'-', current_char, chars...>, sign, current_val>
{
    static constexpr auto nex_val = current_val * 10 + (current_char - '0');
    static constexpr auto value = string_to_int_impl<string<chars...>, -1, nex_val>::value;
};
```

Example: Let's say that we need to convert "-263". Here's the flow:

```cpp
string_to_int_impl< string< '-', '2', '6', '3' >, 1, 0> // this will hit 3rd specialization
string_to_int_impl< string< '6', '3' >, -1, 2> // this will hit the 2nd
string_to_int_impl< string< '3' >, -1, 26> // this will hit the 2nd
string_to_int_impl< string<>, -1, 263> // this will hit first one
```

and first one will have const value equal to sign * current value, which is -1 * 263 => -263

### string_from_int


- Q: Wait, what? Why string_FROM_INT not int_TO_STRING?
- A: Because, I wanted to stick to naming convention `structure_operation`

Declaration:

```cpp
template <int value, bool end, typename sign_str, typename current_string = string<>>
struct string_from_int_impl;
```

In parameters we have value to convert, and current string, during conversion. We need `end` to detect if it is the end of the conversion. Specialization with `0` as a `value` is not enough, it will be ambiguous to compiler with the second specialization. sign_str stores as you probably know: the sign. I can be `string<'-'>` or `string<>`. `current_string` stores current converted value without the sign.

Here we need to be a bit more clever. Before actual converting, we need to detect if value is signed or not and then convert rest of chars, also passing proper sign string. Sign can be a '-' or can simply not exist. We're storing it in string struct, just to easily merge it with actual converted value. And that happens in first specialization.

First specialization - this one for end of conversion, merges `sign_str` with `current_string` as a result of conversion. As you can see, it's specialized with `value` as `0` and `end` as `true`. In fact, it'd work also without specialization for 0, but it's more readable.

```cpp
template <typename sign_str, typename current_string>
struct string_from_int_impl<0, sign_str, true, current_string>
{
    using type = string_merge<sign_str, current_string>;
};
```

Specialization for normal convert iteration looks like this:

```cpp
template <int value, typename sign_str, char ...chars>
struct string_from_int_impl<value, sign_str, false, string<chars...>>
{
    using type = typename string_from_int_impl<
            value/10,
            sign_str,
            value/10 == 0,
            string<value % 10 + '0', chars...>>::type;
};
```

We have here `end` as false, so we know that it's not the end of conversion.

At the end of Adventure With The String, here's previous mentioned sign detection.

```cpp
template <int value>
struct prepare_sign_and_convert
{
    using type = typename std::conditional_t<
            value < 0,
            string_from_int_impl<value * -1, string<'-'>>,
            string_from_int_impl<value, string<>>
            >::type;
};
```

We have used advantages of [std::conditional](https://en.cppreference.com/w/cpp/types/conditional). I'll briefly explain what `std::conditional` is, if you need more detailed description check the cppreference. `std::conditional` takes three parameters, first is a bool condition, second and third - two types. If condition is true, `std::conditional::type` (or `std::conditional_t`) will be an alias to the first of two types, if it's false, alias will refer to the second one.

Now the meat. As `std::conditional` condition we're passing `value < 0`. If value is less than zero, std::conditional will choose first type, which is `string_from_int_impl<value * -1, value == 0, string<'-'>>` Here we're removing the sign from value, we know, that it isn't the end, so we pass false as an `end`, and `string<'-'>` as a `sign_str`. But if value is greater or equal to zero it'll choose the second type, where we pass an empty sign string.

After all that we get the `std::conditional::type`, which will be our converted string

## ctai::tuple

```cpp
template <typename ...args>
struct tuple
{};
```

Tuple is a container that can store objects of various types. In ctai, tuple is used to store types e.g. parsed code tokens as a strings. It could have a lot of operations but in ctai were needed just two: `tuple_append` and `tuple_merge`. You can find them here. I won't explain them because they work similarly as string operations.

## ctai::values_container

```cpp
template <size_t ...values>
struct values_container
{};
```

`values_container` is container similar to string and tuple, but it stores `size_t` values. It's used to store instructions opcodes. It has three operations - `values_append`, `values_merge` and `values_drop`. `values_append` and `values_merge` are very simple and similar to those from string and tuple. We will focus more on `values_drop`.

### values_drop

This operation is still similar to string's and tuple's operations, but it wasn't explained yet. `values_drop` is suppossed to drop couple of first values. Its declaration looks like this:

```cpp
template <size_t, bool, typename>
struct values_drop_impl;
```

first parameter is a number of values to drop, second determines if it's the end of operation and third will eventually be a container type after operation.

First specialization is for case when operation is done. It specializes count as a 0, and end as 'true' It contains an alias to the result container:

```cpp
template <size_t ...left>
struct values_drop_impl<0, true, values_container<left...>>
{
    using type = values_container<left...>;
};
```

Second specialization is for normal iteration step. Here `end` is specialized as `false`, and container is specialized with two parameters: `to_drop` and `left...`. To recursive type defining, we then pass container without `to_drop` argument, so eventually, step by step, we're getting rid of first `count` elements.

```cpp
template <size_t count, size_t to_drop, size_t ...left>
struct values_drop_impl<count, false, values_container<to_drop, left...>>
{
      using type = typename values_drop_impl<count-1,
                                           count == 1,
                                           values_container<left...>>::type;
};
```

Whole values_drop looks like this:

```cpp
namespace details
{
    template <size_t, bool, typename>
    struct values_drop_impl;

    template <size_t ...left>
    struct values_drop_impl<0, true, values_container<left...>>
    {
        using type = values_container<left...>;
    };

    template <size_t count, size_t to_drop, size_t ...left>
    struct values_drop_impl<count, false, values_container<to_drop, left...>>
    {
        using type = typename values_drop_impl<count-1,
                                               count == 1,
                                               values_container<left...>>::type;
    };
}

template <size_t count, typename values>
using values_drop = typename details::values_drop_impl<count, count==0, values>::type;
```

## ctai::machine_state

```cpp
template <typename stack_type, typename flags_type, typename registers_state_type>
struct machine_state
{
    using stack_t = stack_type;
    using flags_t = flags_type;
    using registers_state_t = registers_state_type;

    static constexpr to_flags<flags_t> flags{};
    static constexpr to_register_state <registers_state_t> reg_state{};
};
```

Machine state is a simple structure which stores current stack, flags and registers states. During an instruction execution, some machine state is passed. Instruction executor, creates new machine state, based on instruction type, operands and previus machine state. Then this new state will be passed to next instruction executor and so on.

# Tokenizing the code

ctai assumes that tokens in code are separated with one space from each other. Tokenizer is not able to work with such code `move eax, DWORD PTR [ebx+128]`. It needs to be written in such way: `mov eax , DWORD PTR [ ebx + 128 ]`. Also if there is a token that ctai doesn't know, it assumes that this is a number.


## tokens

Tokens are simply aliases to `ctai::string`, with proper characters as a template parameters. To simplify the code, we're taking advantages of user defined literal `_s`, which converts `const char[]` to `ctai::string` type. Declaring aliases looks like this:


```cpp
namespace cai
{
    namespace tokens
    {
        using tok_empty = decltype(""_s);

        using tok_exit = decltype("exit"_s);

        using tok_mov = decltype("mov"_s);
        using tok_add = decltype("add"_s);
        using tok_push = decltype("push"_s);
        ...
```

There are also two things related with tokens, which are useful later - `is_reg_token` and `token_to_reg_opcode`

`is_reg_token` checks if passed token is a register e.g. eax, ebx etc. It uses `is_type_in_v` to check if passed token type (which is a type of `string<>`) is equal to one of defined registers tokens types.

`is_type_in_v` looks like this:


```cpp
template <typename type, typename ...types>
constexpr auto is_type_in_v = ((std::is_same<type, types>::value) || ...);
```

`token_to_reg_opcode` converts passed register token to its opcode.

```cpp
namespace details
{
    template <typename>
    struct token_to_reg_opcode_impl;

    template <> struct token_to_reg_opcode_impl<tokens::tok_eax> { static constexpr auto value = regs::to_size<regs::id_t::EAX>; };
    template <> struct token_to_reg_opcode_impl<tokens::tok_ebx> { static constexpr auto value = regs::to_size<regs::id_t::EBX>; };
    ... 
}

template <typename token>
constexpr auto token_to_reg_opcode = details::token_to_reg_opcode_impl<token>::value;
```

## tokenizer

Tokenizing is divided into three steps:

- iterate through characters
    - if character is not a space append it to current token value
    - if it is a space, save current token value to tuple of tokens, and continue.

In code there are two operations - `get_token`, which will extract next token from string and `tokenize`, which will use `get_token` to extract all tokens.


## get_token

This struct will eventually contain aliases to extracted token and rest of the string. E.g. if string would be "abc def g" these aliases would be:

```cpp
using result_token = string<'a', 'b', 'c'>;
using rest_of_string = string<'d', 'e', 'f', ' ', 'g'>;
```

Declaration:

```cpp
template <typename str, typename curr_token>
struct get_token_impl;
```

`str` parameter is code characters and second - current token characters.

First specialization is for an empty string. It contains earlier mentioned `result_token` and empty string as a `rest_of_string`

```cpp
template <typename current_token>
struct get_token_impl<string<>, current_token>
{
    using result_token = current_token;
    using rest_of_string = string<>;
};
```

next specialization is for case when next character is a space. Then we know that it is the end of current token. We're dropping the space and store an alias to the current token.

```cpp
template <char ...str_chars, typename current_token>
struct get_token_impl<string<' ', str_chars...>, current_token>
{
    using result_token = current_token;
    using rest_of_string = string<str_chars...>;
};
```

and the last specialization for normal iteration step. We're getting the first character and appending it to the current token chars. Aliases are `current_token` with appended char and `rest_of_string` without that character

```cpp
template <char curr_char, char ...str_chars, typename current_token>
struct get_token_impl<string<curr_char, str_chars...>, current_token>
{
    using string_without_token_char = string<str_chars...>;

    using result_t = get_token_impl<string_without_token_char, string_append<current_token, curr_char>>;

    using result_token = typename result_t::result_token;
    using rest_of_string = typename result_t::rest_of_string;
};
```

## tokenize

`tokenize` implementation is rather simple. It just recursively extracts tokens one after another. There is also specialization with empty code characters - case when there are no more tokens.

```cpp
namespace details
{
    template <typename s, typename current_tokens = tuple<>>
    struct tokenize_impl;

    template <typename current_tokens>
    struct tokenize_impl<string<>, current_tokens>
    {
        using tokens = current_tokens;
    };

    template <char ...str_chars, typename current_tokens>
    struct tokenize_impl<string<str_chars...>, current_tokens>
    {
        using str = string<str_chars...>;
        using get_token_t = get_token<str>;

        using next_tokens = tuple_append<current_tokens, typename get_token_t::result_token>;
        using next_string = typename get_token_t::rest_of_string;

        using tokens = typename tokenize_impl<next_string, next_tokens>::tokens;
    };
}

template <typename s>
using tokenize = typename details::tokenize_impl<s>::tokens;
```

# Labels

Labels are very useful in assembly. Without them we would need to know exact offset to the instruction where we would like to jump. Fortunately for normal people, we can write code with labels. Unfortunately for compiler implementers - they need to write smart compilers, which will be able to detect labels and substitute every its reference to actual instruction pointers/offsets. And that is exactly what ctai does. It detects the labels, stores them with its instruction pointers, removes them from actual code and substitutes every reference to it with a number - the instruction pointer.

## Example

```asm
  pop eax
  pop ecx
  cmp eax, 0
  jg end
  mov ebx, 0
loop:
  add ebx, ecx
  cmp ecx, 0 
  jg loop

  mov eax, ebx
end:
```

for sake of this example let's say that these instructions have such sizes: 

* pop register: 2 
* mov register, value: 3 
* add register, register: 3 
* cmp register, value: 3 
* jg label: 2 
* mov register register: 3

now what ctai needs to do is to detect label's declarations and its instruction pointers. Let's write instruction pointers of each instruction:

```asm
  pop eax ; 0
  pop ecx ; 2
  cmp eax, 0 ; 4
  jg end ; 7
  mov ebx, 0 ; 9
loop:
  add ebx, ecx ; 12
  cmp ecx, 0 ; 15
  jg loop ; 18

  mov eax, ebx ; 20
end:
  ret ; 23
```

The `loop` label will point to `12` instruction opcode and `end` to `23`. Ctai will substitute label's references with its pointers, so code will eventually become this:

```asm
pop eax
pop ecx
cmp eax, 0
jg 23
mov ebx, 0
add ebx, ecx
cmp ecx, 0
jg 12
mov eax, ebx
ret
```

Such code has everything what is needs. It's ready to execute.

## instruction_match

Before we'll see how ctai is collecting and substituting labels, we need to know how to detect instruction from tokens and count its instruction pointer. That's the purpose of `instruction_match` struct existence. `instruction_match` itself is just an alias to `matcher_impl<>;` struct. It takes just one template parameter: tuple with string tokens. All juicy stuff is done in matcher_impl body. Every code instruction in ctai is matched/recognized with various of `matcher_impl` specializations. To see how it's done in actual code, let's look at the simplest instruction: `exit`. It's composed of just one token, as everyone, including my ferret, probably know - "exit". Matcher specialization for `exit` looks like this:

```cpp
template <typename ...rest_of_tokens>
struct matcher_impl<tuple<tokens::tok_exit, rest_of_tokens...>>
{
  using instruction = values_container<inst::to_size<inst::id_t::EXIT>>;
  static constexpr auto eip_change = get_eip_change<inst::id_t::EXIT>;

  using instruction_tokens = tuple<tokens::tok_exit>;
  using rest_of_tokens_t = tuple<rest_of_tokens...>;
};
```

It's specialized with tuple, which on the first parameter type has `tokens::tok_exit` and some other tokens in which we aren't interested in this matcher. Important for us is that, the "exit" token must be on the first place. When compiler will hit matcher specialization, it does couple of things. First, it creates an alias instruction which is an alias to `values_container`, with this instruction opcodes. In `exit` case it contains just one opcode - `size_t` representation of `inst::id_t::EXIT` enum value. Then it gets size of this instruction. In other words - of how many opcodes this instruction is composed. In this case just one. Next alias is to tuple, which contains this instruction tokens. In exit's case it's just an "exit". And the last one - alias to tuple which contains rest of the code tokens.

Ok. `exit` was the easy one - just fine for my ferret. Now let's examinate one more, slightly more complicated example: `mov eax , DWORD PTR [ ecx + 4 ]`

Now fasten your seat belts, hold your mom, squeeze your teddy bear in the second hand and hide behind your dad. This will be freaking deadly.

Jk. In ctai everything is simple like my ferret's life.

mov matcher is just longer ..

```cpp
template <typename reg_token, typename mem_size_token, typename mem_ptr_reg, typename mem_ptr_const, typename plus_minus_token, typename ...rest_of_tokens>
struct matcher_impl<tuple<
    tokens::tok_mov,
    reg_token,
    tokens::tok_comma,
    mem_size_token,
    tokens::tok_ptr,
    tokens::tok_square_bracket_open,
    mem_ptr_reg,
    plus_minus_token,
    mem_ptr_const,
    tokens::tok_square_bracket_close,
    rest_of_tokens...>>
{
    static constexpr auto instruction_type = is_plus<plus_minus_token> ? inst::id_t::MOV_REG_MEM__mem_eq_reg_plus_const
                                                                       : inst::id_t::MOV_REG_MEM__mem_eq_reg_minus_const;

    using instruction = values_container<
        inst::to_size<instruction_type>,
        token_to_reg_val<reg_token>,
        token_to_reg_val<mem_ptr_reg>,
        string_to_int<mem_ptr_const>,
        memory::to_size<mem_size_decoder<mem_size_token>>>;

    static constexpr auto eip_change = get_eip_change<instruction_type>;
    using instruction_tokens = tuple<
        tokens::tok_mov,
        reg_token,
        tokens::tok_comma,
        mem_size_token,
        tokens::tok_ptr,
        tokens::tok_square_bracket_open,
        mem_ptr_reg,
        plus_minus_token,
        mem_ptr_const,
        tokens::tok_square_bracket_close>;

    using rest_of_tokens_t = tuple<rest_of_tokens...>;
};
```

.. and less readable . But let's try.

First: template arguments not explicitly specialized.

```cpp
template <typename reg_token, 
          typename mem_size_token, 
          typename mem_ptr_reg, 
          typename mem_ptr_const, 
          typename plus_minus_token, 
          typename ...rest_of_tokens>
```

- `reg_token` - token of register to which we want to move. In this case `eax`
- `mem_size_token` - token of value size which we want to get from memory. It can be `DWORD`, `WORD` or `BYTE`
- `mem_ptr_reg` - token of register which is used to compute memory pointer. This one in [] brackets. In this example `ecx`
- `mem_ptr_const` - value which is used to compute memory pointer - again, this one in [] brackets. In this example '4'
- `plus_minus_token` - token of '+' or '-', this one i... Ya know where. Here it's '+'.

Now we know everything. Let's look at matcher specialization and how it's related to the tokens:

```cpp
struct matcher_impl<tuple<
        tokens::tok_mov,                  // mov
        reg_token,                        // eax
        tokens::tok_comma,                // ,          Yes. Comma is separate token
        mem_size_token,                   // DWORD 
        tokens::tok_ptr,                  // PTR
        tokens::tok_square_bracket_open,  // [          YES. Oppening square bracket is also separate token
        mem_ptr_reg,                      // ecx
        plus_minus_token,                 // +
        mem_ptr_const,                    // 4
        tokens::tok_square_bracket_close, // ]          Ye, ye...
        rest_of_tokens...>>
```

In struct body everything is similar to the exit's case, but before that there is one additional step. It needs to determine actual instruction opcode. It checks if in [] there is plus or minus and select proper opcode.

And that's it. There is no other matcher where is done something groundbreaking is done.

## Collecting and removing the labels

these two operations are done in one phase - extract_labels. Its declaration looks like this:

```cpp
template <typename, size_t current_ip = 0,  typename final_tokens = tuple<>, typename final_labels = tuple<>>
struct extract_labels_impl;
```

first parameter is tuple with code tokens, second: current instruction pointer, third: final tokens, without labels declarations and the last one: extracted labels and its instruction pointers.

`extract_labels_impl` iterates through tokens, collects labels and will eventually contain two aliases: 
- `tokens` - tokens without labels declarations 
- `labels` - extracted labels with its instruction pointers

And excatly these aliases store first specialization - this one for empty tokens input:

```cpp
template <size_t ip, typename final_tokens, typename final_labels>
struct extract_labels_impl<tuple<>, ip, final_tokens, final_labels>
{
    using tokens = final_tokens;
    using labels = final_labels;
};
```

It has two more specializations - for label match and for normal token match.

When token is a label declaration, we know that its first character will be ':'. So we specialize the tokens tuple with first string's first character as a ':'.

```cpp
template <size_t current_ip, char ...label_chars, typename ...rest_of_tokens, typename ...current_tokens, typename ...current_labels>
struct extract_labels_impl<
    tuple<string<':', label_chars...>, rest_of_tokens...>,
    current_ip,
    tuple<current_tokens...>,
    tuple<current_labels...>>
{
  using extracted = extract_labels_impl<
      tuple<rest_of_tokens...>,
        current_ip,
        tuple<current_tokens...>,
        tuple<current_labels..., label_metadata<string<'.', label_chars...>, current_ip>>>;

  using tokens = typename extracted::tokens;
  using labels = typename extracted::labels;
};
```

In the body it recursively defines an alias for itself with: 
* `tuple<rest_of_tokens...>` - tokens for next iteration 
* `current_ip` - this case is for label declaration match, and doesn't count as instructions thus we don't increase instruction pointer 
* `tuple<current_tokens...>` - result tokens without current token, because current token is a label declaration, which we want to erase 
* `tuple<current_labels..., label_metadata<string<'.', label_chars...>, current_ip>>` - this one is more interesting. Here we append labels with label we just discovered. We're creating tuple with labels that we've already detected plus this one that we just found.

And now normal iteration, without label declaration. We know that if next token is not a label declaration, it's an instruction. So we can take advantage of instruction_match possibilities. As you remember, instruction_match will eventually contain four things: instruction opcodes, instruction size, instruction tokens and the rest of code tokens after this instruction. At this point we don't really care about the opcodes, but the rest will be useful.

Specialization looks like this:

```cpp
template <size_t current_ip, typename current_token, typename ...rest_of_tokens, typename ...result_tokens, typename result_labels>
struct extract_labels_impl<
        tuple<current_token, rest_of_tokens...>,
        current_ip,
        tuple<result_tokens...>,
        result_labels>
{
    using instruction = instruction_match<tuple<current_token, rest_of_tokens...>>;
    using next_tokens = tuple_merge<tuple<result_tokens...>, typename instruction::instruction_tokens>;
    static constexpr auto nex_ip = current_ip + instruction::eip_change;

    using extracted = extract_labels_impl<
            typename instruction::rest_of_tokens_t,
            nex_ip,
            next_tokens,
            result_labels>;

    using tokens = typename extracted::tokens;
    using labels = typename extracted::labels;
};
```

It's pretty much similar to the previous one but here, we need to handle previous mentioned instruction.
- `using instruction = instruction_match<tuple<current_basic_token, basic_tokens...>>;` - we're matching the instruction
- `using next_tokens = tuple_merge<tuple<current_tokens...>, typename instruction::instruction_tokens>;` - next tokens are current tokens plus just matched instruction tokens
- `static constexpr auto nex_ip = current_ip + instruction::eip_change;` - next instruction pointer, which will points to the next instruction, is current ip + just matched instruction size

Then just recursively create `extract_labels_impl` alias type with just gathered information.

## Labels sustitution

Next and the last step with labels is to actually substitute them with the instruction pointers. To achieve this, we need to, again, iterate through tokens and replace them with instruction pointers that they refer to.

`substitute_labels_impl` has three specializations, but I'll focus only on one - that with label match. Other two are very similar to everything that has been explained earlier.

```cpp
template <char ...label_chars, typename ...rest_of_tokens, typename labels, typename ...current_tokens>
struct substitute_labels_impl<
    tuple<string<'.', label_chars...>, rest_of_tokens...>,
    labels,
    tuple<current_tokens...>>
{
  static constexpr auto ip = labels_get_ip<labels, string<'.', label_chars...>>;
  using str_ip = string_from_int<ip>;

  using substitued = substitute_labels_impl<
      tuple<rest_of_tokens...>,
      labels,
      tuple<current_tokens..., str_ip>>;

  using tokens = typename substitued::tokens;
};
```

first template parameter is tuple with tokens. Like in `extract_labels`, we specialize it with first character of first token, but in this case it's a '.'

- `static constexpr auto ip = labels_get_ip<labels, string<'.', label_chars...>>;` - here we get label instruction point from tuple of labels metadata.
- `using str_ip = string_from_int<ip>;` - aaand converting it to string to easily append it to the tokens.

## Assembling

Fear no more. Assemble only sounds scary. In fact, it's easy. You won't even learn anything here because it uses mechanism already explained - instruction_match. As you already know `instruction_match` besides the others, stores alias to `values_container<>` with instruction opcodes.

With that, token assembling is a piece of cake. We just need to iterate through tokens, match instructions, one after the another, take its opcodes and append them to the output opcodes.

```cpp
namespace details
{
    template <typename tokens, typename instructions_t = values_container<>>
    struct assemble_impl;

    template <typename curr_instructions>
    struct assemble_impl<tuple<>, curr_instructions>
    {
        using instructions_result = curr_instructions;
    };

    template <typename tokens, typename curr_instructions>
    struct assemble_impl
    {
        using instruction = get_instruction<tokens>;

        using next_instructions = values_merge<curr_instructions, typename instruction::instruction>;
        using next_tokens = typename instruction::res_of_tokens;

        using instructions_result = typename assemble_impl<next_tokens, next_instructions>::instructions_result;
    };
}

template <typename tokens>
using assemble = typename details::assemble_impl<tokens>::instructions_result;
```

# Executing

Concept of code executing is really simple. We have set of instructions. We take first one and execute it. Instruction in some way affects the machine state. We take next instruction, execute it, and so on..

Basically in ctai there are three steps: 1. Get current instruction, based on the eip register 2. Execute it. 3. Go to 1. That's it. Finitio. Arrivederci Roma.

Almost. There is some magic done in second step. But don't worry kid, I'll be your Willy Wonka.



## ex_instruction

This lil homie is responsible for single instruction executing. I mean, its specializations. This lil homie is only the declaration. That's why it's so lil.
But what the instruction executing really is? It's just machine state change. Push some value to the stack. Set value in the memory, set register value etc.

```cpp
template<typename state, size_t ...opcodes>
struct ex_instruction;
```

It has at least one parameter. First is current machine state and others - instruction opcodes.

Let's see a simple example: `mov eax , 2`

```cpp
template<typename state_t, size_t reg1, size_t val, size_t ...rest_of_opcodes>
struct ex_instruction<state_t, inst::to_size<inst::id_t::MOV_REG_VAL>, reg1, val, rest_of_opcodes...>
{
    using state = to_machine_state<state_t>;

    using new_regs_state = set_reg<typename state::registers_state_t, regs::to_id<reg1>, static_cast<uint32_t>(val)>;
    using final_regs_state = adjust_eip<new_regs_state, inst::id_t::MOV_REG_VAL>;

    using next_machine_state = machine_state<typename state::stack_t, typename state::flags_t, final_regs_state>;
};
```

Here, we specialize ex_instruction with opcodes: 
* `inst::to_size<inst::id_t::MOV_REG_VAL>` - first opcode is always an instruction type. 
* `reg1` - this one determines to which register we will move value 
* `val` - it's just value we will move to register

In the body you can see that we use `set_reg` alias. It takes current registers state, register id and value to set. It sets the value to passed register and eventually become new registers state struct, with new value in register.
Then we need to adjust instruction pointer register - `eip`. So we pass new register state - this one with the new value, and instruction type which is now executed. `adjust_eip<>` will get info about this instruction size and will increase `eip` register with this value. Thanks to that, `eip` will point to the next instruction.

Last thing is to create an alias to new machine_stete, with adjusted registers.

And that's it. Instruction executed. Go get a beer. You deserve it.

Get back you lazy.. you. You need to execute the next one. But first, go stretch yourself. We will jump now `jge ip`

```cpp
template<typename state_t, size_t ip, size_t ...rest_of_opcodes>
struct ex_instruction<state_t, inst::to_size<inst::id_t::JGE>, ip, rest_of_opcodes...>
{
  using state = to_machine_state<state_t>;

  static constexpr auto flags_v = typename state::flags_t{};

  static constexpr bool should_jmp = ((static_cast<int>(flags_v.SF) ^ static_cast<int>(flags_v.OF)) == 0);
  using final_regs_state = std::conditional_t<
      should_jmp,
      set_reg<typename state::registers_state_t, regs::id_t::EIP, static_cast<uint32_t>(ip)>,
      adjust_eip<typename state::registers_state_t, inst::id_t::JGE>>;

  using type = machine_state<typename state::stack_t, typename state::flags_t, final_regs_state>;
};
```

`JGE` should jump if xored flags `sf` and `of` are equal to zero. So we get this information and pass as a condition to the `std::conditional`. Here conditional will choose from two `registers_state<>` types:
First one, if condition passed, with new eip value, that one passed as a label
second - if we don't need to jump, just adjust the eip like in normal instruction.

There is a lot of instructions. I won't explain them all. If you're here, still reading this poem, you'll probably get how they works.

## execute_impl
`execute_impl` has three parameters. Current machine state. All opcodes of the program and current opcodes.

- Q: What is the difference between `all_opcodes` and current opcodes?
- A: Let's say that our program is composed of only two instructions:

```asm
pop eax
add eax, 2
```

Now let's say that opcodes of these instructions look like this:

```
1 12
4 12 2
```

So all opcodes of this program are: `1, 12, 4, 12, 2`

and now let's say that we've already executed first one and now eip has value 2 thus it points to the second instruction. So now `all_opcodes` still are: `1, 12, 4, 12, 2` but current opcodes are: `4, 12, 2`. And these current opcodes will be passed to `ex_instruction`.

`execute_impl` has only two specializations. One especially for exit instruction:

```cpp
template <typename curr_machine_state, typename all_opcodes, size_t ...rest_of_instructions>
struct execute_impl<curr_machine_state, 
                    all_opcodes, 
                    values_container<inst::to_size<inst::id_t::EXIT>, rest_of_instructions...>>
{
  using regs_state = typename to_machine_state<curr_machine_state>::registers_state_t;
  static constexpr auto ret_val = get_reg<regs_state, regs::id_t::EAX>;
};
```

In this case we know that this will be the end of the program, so we get the eax value and assign it to `ret_val`.

In normal iteration step we execute the current instruction and get the machine state after execution. Then we get next instruction opcodes. `get_next_instruction<>` will get them from `all_opcodes` based on the eip value. Then just recursively create an alias to itself with new machine state and next instruction opcodes and get its `ret_val`.

```cpp
template <typename curr_machine_state, typename all_opcodes, typename opcodes = all_opcodes>
struct execute_impl
{
  using next_machine_state = execute_instruction<curr_machine_state, opcodes>;
  using next_opcodes = get_next_instruction<next_machine_state, all_opcodes>;
  static constexpr auto ret_val = execute_impl<next_machine_state, all_opcodes, next_opcodes>::ret_val;
};
```

## prepare_and_execute

Finally. The Special One.
As you can read in one of the first Tolkien's novel drafts:

```
"One Struct to rule them all, One Struct to find them,
One Struct to bring them all and in the constexpr bind them
In the Template Masturbation where the typename lie."
```

And then he started to write about some rings. That's a shame. Those could be quite good books.

`prepare_and_execute` brings everything together and creates complete, healthy working frankenstein. It gets only one parameter, code as a `ctai::string` and define one interesting thing, the value which was returned by the program. Ehh.. Just read.

```cpp
template <typename code>
struct prepare_and_execute
{
  using tokens = tokenize<code>;
  using extract_labels_result = extract_labels<tokens>;
  using tokens_without_labels = typename extract_labels_result::tokens;
  using labels_metadata = typename extract_labels_result::labels;
  using tokens_after_labels_substitution = substutite_labels<tokens_without_labels, labels_metadata>;
  using opcodes = assemble<tokens_after_labels_substitution>;
  using machine_state = startup_machine_state;

  static constexpr auto ret_val = execute_impl<machine_state, opcodes>::ret_val;
};
```

Beatifull, isn't it?

# Conclusion
ctai is the most useless project I've ever wrote. Beside that it was really fun to fight with countless pages of unreadable errors and see that it really works. TBH I though that it will be much harder to write. From time perspective I see that ctai is a very simple program. Maybe even a little boring.


# Thanks for reading o/
