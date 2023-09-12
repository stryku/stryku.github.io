---
layout: post_without_latex
title:  "Conversion of hexadecimal character to its binary value"
date:   2020-04-03 00:00:00 +0200
categories: cpp
published: true
---


# Conversion of hexadecimal character to its binary value

(In the whole article I assumed that static_assert(CHAR_BIT == 8u) compiles)


# Digit character to its binary value
Recently I was writing an article about optimizations in [okon](https://github.com/stryku/okon). I was explaining an algorithm that converts a text SHA-1 hash to its binary value using SIMD instructions.

Suddenly, I realized one thing...

... but first. A small question. How would you convert a character that represents a digit (one of: 0123456789) to the value that it represents? For '0' it should produce 0, '5' -> 5 etc. In the programming world:


```cpp
char c = '3';
uint8_t value = ?
```

To not spoil the answer right away, here a picture of my fiancée's dog chasing a carrot:

![foo](/assets/00019-conversion-of-hexadecimal-character-to-its-binary-value/max_carrot_op.gif)


If you answered
```cpp
char c = '3';
uint8_t value = c - '0';
```

you're right! That's the way. And, that's the way I was using for my entire life till yesterday.

So, I was writing an article when suddenly, I realized that you can do this, as well!

```cpp
char c = '3';
uint8_t value = c & 0x0f;
```

That's because '0' character is represented by value 48, which in binary is 0011 0000. Here's the full table:
```
'0' = 48 = 0011 0000
'1' = 49 = 0011 0001
'2' = 50 = 0011 0010
'3' = 51 = 0011 0011
'4' = 52 = 0011 0100
'5' = 53 = 0011 0101
'6' = 54 = 0011 0110
'7' = 55 = 0011 0111
'8' = 56 = 0011 1000
'9' = 57 = 0011 1001
```

Not sure why, but I was as happy as a sandboy when I discovered this. This is just great. No subtraction, no magic, pure bit-wise operation.


# Hex char to its binary value


Like I said, I was working on conversion of hexadecimal SHA-1 hashes to their binary representation. After my discovery I immediately started wondering whether it can help me with characters A-F too.

Let's see how 'A'..'F' and 'a'..'f' are represented:
```
'A' = 65  = 0100 0001
'B' = 66  = 0100 0010
'C' = 67  = 0100 0011
'D' = 68  = 0100 0100
'E' = 69  = 0100 0101
'F' = 70  = 0100 0110

'a' = 97  = 0110 0001
'b' = 98  = 0110 0010
'c' = 99  = 0110 0011
'd' = 100 = 0110 0100
'e' = 101 = 0110 0101
'f' = 102 = 0110 0110
```

It's well known that lowercase and uppercase characters differ by value 32 ('A' + 32 => 'a' etc.). Thanks to that, the characters that we care about differ only at one bit.

(Note that if you do 'A' & 0x0f you get 1, not 0)

This trait can be used to convert the characters to its values without if/else! Take a look:

```cpp
char c = 'D'; // An example value.

constexpr char to_add[] = { 0, 9 };
const bool is_alpha_char = (c & 0b01000000);
const auto result = (c & 0x0f) + to_add[is_alpha_char];
```


So, we:

- is_alpha_char = (c & 0b01000000) - determine whether the character is an alpha character or digit. To do this, the 7th bit is tested.
- (c & 0x0f) - calculate a "base value" of the character. If it's a digit character, that's the result value.
- + to_add[is_alpha_char] - We add a value from the to_add table. If is_alpha_char is 0, that means the character was a digit and we don't need to add anything. So we add to_add[0], which is 0. If is_alpha_char is 1 we add 9 (remember the fact that 'A' & 0x0f produces 1?).

And that's it. No more magic.


# Performance

How performance of the branchless code compares to the 'old' way of calculations?

By the 'old' way I mean something like this:

```cpp
char c = 'D'; // An example value.

if (c >= 'a') {
    c -= 'a' - ':';
} else if (c >= 'A') {
    c -= 'A' - ':';
}

const auto result = c - '0';
```

Here are results on my computer (Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz):

```
-----------------------------------------------------------------|-------
Benchmark                     Time             CPU   Iterations  |   %
-----------------------------------------------------------------|-------
OldRustyCalculations    9534118 ns      9534318 ns           73  |  100%
ShinyBranchless         6170208 ns      6170305 ns          115  |  ~65%
```

- Here you can try it on your own on, [on QuickBench](http://quick-bench.com/dd6O0-6ETBVBGxSosSatSKNIxA8)
- Here you can find a [gist with the benchmark code](https://gist.github.com/stryku/b8177a66f79540f912ca8b7909ffccda)

# Conclusion
I know that this solution is probably well known. But, it was really fun to 'discover' and fool around with it. I didn't search for it on the Internet to not spoil the fun. If you know anything more about the topic, let me know!

I know too that the conversion can be simplified to an array of result value per every char value. But, I still think that there are use cases when bit-wise operations are useful, e.g. when you vectorize using SIMD instructions.

Because of the performance boost, I'll definitely try this in okon.


# Links and discussion
- [QuickBench benchmark](http://quick-bench.com/dd6O0-6ETBVBGxSosSatSKNIxA8)
- [Benchmark code](https://gist.github.com/stryku/b8177a66f79540f912ca8b7909ffccda)
If you have any thoughts, let me know using one of these:

- [r/cpp](https://www.reddit.com/r/cpp/comments/fty8us/conversion_of_hexadecimal_character_to_its_binary/)
- [Hacker news](https://news.ycombinator.com/item?id=22765343)


# Thanks for reading o/
 