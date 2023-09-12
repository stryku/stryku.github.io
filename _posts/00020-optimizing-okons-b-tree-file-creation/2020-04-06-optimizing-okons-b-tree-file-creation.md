---
layout: post_without_latex
title:  "Optimizing okon's B-tree file creation"
date:   2020-04-06 00:00:00 +0200
categories: cpp
published: true
---


# Optimizing okon's B-tree file creation

The article is about [okon](https://github.com/stryku/okon).

I wanted to optimize transformation of an original [Have I Been Pwned (HIBP)](https://haveibeenpwned.com/) password file to a B-tree file that okon understands. In this article I'm going to describe what ideas I had and how they affected performance.


# Benchmarking
Every idea has been benchmarked to see how it affects performance and if it actually helps. The approach is very simple and looks like this:

```bash
#!/bin/bash

for i in {0..150}
do
    rm -r wd
    rm test.btree
    mkdir wd
    sync; echo 3 > /proc/sys/vm/drop_caches
    /usr/bin/time -f "%e" ./okon-cli --prepare hibp_passwords.txt --output test.btree --wd wd/
done
```

`okon-cli` is run to prepare a *.btree file from a file downloaded from [HIBP site](https://haveibeenpwned.com/Passwords) (SHA-1, Version 5, ordered by prevalence). To be sure that okon always runs on a 'fresh environment' working directory and produced *.btree file are deleted. Additionally, filesystem cache is cleared before every run. Measured is wall-clock time.

Everything is repeated 150 times. Times are collected and the median is calculated.

One thing to mention, hibp_passwords.txt is not the full original HIBP file. Benchmarking every idea implementation on the full file would take too long. I decided to take first 55 milion hashes (about 10% of the original file) and benchmark on these hashes.


# Starting point
Before implementing the ideas, I went to origin/master branch and benchmarked it. It took 18.94s. I'll present results in a table like this:

```
|      Optimization phase      | time [s] | delta last [s] | % last | delta master [s] | % master |
|------------------------------|---------:|---------------:|-------:|-----------------:|---------:|
| None                         |    18.94 |            N/A |  100.0 |              N/A |    100.0 |
```
















# Caching nodes in sorted insertions

The first idea takes advantage of the fact that while preparing original file, okon creates B-tree file out of sorted values.

Let's see how it was implemented at this moment. There is a class btree_sorted_keys_inserter. It's responsible for creating a B-tree file out of sorted values. If you insert sorted values to a B-tree, you know exactly to which node you'll insert the next value. Thanks to that, you can cache nodes and write them on disk only once - when they are full. Before changes, only one node was cached - the leaf where you insert values. Here you have a visual representation:

Let's say that we have a tree of an order 2. At the beginning you have a tree with one, empty node.
```
[  ] <-- cached node
```
Insert 1 and 2.
```
[1,2] <-- cached node
```
Then you want to insert 3. The node is full, so you write it to the file. Then, you create a parent node, insert 3 there and write it to the file. Next, create another leaf node and cache that node.
```
    [3 ]
   /    \
[1,2]  [  ] <-- cached node
```
Now you insert 4 and 5 into the cached node.
```
    [3 ]
   /    \
[1,2] [4,5] <-- cached node
```
Then if you want to insert 6, the node is full, so you have to read parent node from disk, insert 6 to it, write it back and create another leaf node.
```
      [3,6]
     /  |  \
[1,2] [4,5] [  ] <-- cached node
```
Do you see what can be improvement?

The idea is to cache whole path from root node to the leaf into which we're currently inserting.

So instead of caching only the leaf, we cache the parent too:

```
    [3 ] <-- cached node
   /    \
[1,2] [4,5] <-- cached node
```
Thanks to that, when we want to insert 6 into the tree, we don't need to read and write the parent. We just insert the 6 to the cached parent. No disk operations.

It doesn't seem impressive on such a trivial example, but in scope of the original HIBP file, it's quite a good improvement. Here are number of B-tree file reads/writes before and after the change.

Before changes:
```
|        | 55 mil hashes | full file |
|--------|--------------:|----------:|
| Reads  |        53'708 |   542'261 |
| Writes |       214'787 | 2'168'523 |
```

After changes:
```
|        | 55 mil hashes | full file |
|--------|--------------:|----------:|
| Reads  |             0 |         0 |
| Writes |        53'713 |   542'266 |
```
And here's how performance changed (remember, file with 55mil hashes was benchmarked, not the full, original file).

```
|      Optimization phase      | time [s] | delta last [s] | % last | delta master [s] | % master |
|------------------------------|---------:|---------------:|-------:|-----------------:|---------:|
| None                         |    18.94 |            N/A |  100.0 |              N/A |    100.0 |
| Nodes caching                |    17.44 |          -1.50 |   92.1 |            -1.50 |     92.1 |
```
Exactly 1.5s faster, which is ~8%. Not so much, considering how many disk operations were cut off. Apparently the operating system did a very good job in filesystem caching.


# Reading chunks of file
Before changes, okon read the original file using the [std::getline function](https://en.cppreference.com/w/cpp/string/basic_string/getline). I thought that reading big chunks of file may perform better. Of course, without the getline() I had to implement searching for newline characters and handling corner cases (like when a hash is split between chunks) on my own. But I still thought that it's worth it. Here are the results:

```
|      Optimization phase      | time [s] | delta last [s] | % last | delta master [s] | % master |
|------------------------------|---------:|---------------:|-------:|-----------------:|---------:|
| None                         |    18.94 |            N/A |  100.0 |              N/A |    100.0 |
| Nodes caching                |    17.44 |          -1.50 |   92.1 |            -1.50 |     92.1 |
| Chunks read                  |    18.88 |           1.44 |  108.3 |            -0.06 |     99.7 |
```
Wait, what? It's almost as slow as the None version, before any optimizations. I had to mess up parsing so bad..

## Async reading chunks of file
I was about to throw the changes away when I thought about another idea. Now, when I have implemented reading and parsing chunks of file, I could implement asynchronous reading and parsing. Chunks of file would be read in one thread and parsing and processing in another. Here's a simple diagram:

```
                 +-----------------------------------------------------------------------------+
                 |    okon                        +----------+                                 |
                 |                          |---- | Buffer 1 |----|                            |
                 |                          |     +----------+    |                            |
                 |                          |                     |                            |
                 |                          |     +----------+    |                            |
                 |                          |---- | Buffer 2 |----|                            |
+-----------+    |    +----------------+    |     +----------+    |     +-------------------+  |
| HIBP file |----|--->| Reading thread |--->|                     |---->| Processing thread |  |
+-----------+    |    +----------------+    |     +----------+    |     +-------------------+  |
                 |                          |---- | Buffer 3 |----|                            |
                 |                          |     +----------+    |                            |
                 |                          |                     |                            |
                 |                          |     +----------+    |                            |
                 |                          |---- | Buffer 4 |----|                            |
                 |                                +----------+                                 |
                 +-----------------------------------------------------------------------------+
```
So you create N buffers (4 in current implementation). There is a thread started that is dedicated only to read chunks of the file and write them to a buffers queue. Another thread reads the chunks from the queue and processes them. Here's the performance impact:

```
|      Optimization phase      | time [s] | delta last [s] | % last | delta master [s] | % master |
|------------------------------|---------:|---------------:|-------:|-----------------:|---------:|
| None                         |    18.94 |            N/A |  100.0 |              N/A |    100.0 |
| Nodes caching                |    17.44 |          -1.50 |   92.1 |            -1.50 |     92.1 |
| Async chunks read            |    16.40 |          -1.44 |   91.7 |            -2.54 |     86.6 |
```
Cool, another 6% speedup.



# Async sorting and writing to B-tree
If asynchronous file reading helped, asynchronous writing should help too.

This is how sorting was implemented before this change.

While reading the whole HIBP file, 256 intermediate files are created. They are named '00', '01', '02' ... 'FF'. In file '00' are stored all hashes with first byte equal to 0x00. In file '01', are all hashes with first byte 0x01 and so on..

Before the change sorting was done in parallel by 4 threads. Every thread has 64 files to sort, and they were distributed like this:

```
00 |
.. |--> Thread 1
3F |

40 |
.. |--> Thread 2
7F |

80 |
.. |--> Thread 3
BF |

C0 |
.. |--> Thread 4
FF |
```
When all files are sorted, hashes from file 00 are read and inserted into B-tree, then hashes from 01 are read and inserted and so on. The files order is important. All hashes in file N are greater than hashes in file N-1 and smaller than hashes in file N+1. So, files have to be read and inserted in ascending order to create a valid B-tree.

When implementing asynchronous sorting and writing, such files per thread distribution wouldn't work well. That's because e.g. file 02 to be processed, needs to be sorted first. If thread 1 sorts files 00 .. 3F, file 02 needs to wait until files 00 and 01 are sorted. So, in fact, the files 00..3F are sorted simultaneously. So, okon eventually processes and inserts these files simultaneously too. This can be done better.

Now, files are distributed between threads like this:

```
00 --> Thread 1
01 --> Thread 2
02 --> Thread 3
03 --> Thread 1
04 --> Thread 2
05 --> Thread 3
06 --> Thread 1
...
```
Thanks to that, files 00, 01 and 02 are sorted 'at once'. This minimizes the time that okon needs to wait for a file to be sorted.

Here's the performance impact:

```
|      Optimization phase      | time [s] | delta last [s] | % last | delta master [s] | % master |
|------------------------------|---------:|---------------:|-------:|-----------------:|---------:|
| None                         |    18.94 |            N/A |  100.0 |              N/A |    100.0 |
| Nodes caching                |    17.44 |          -1.50 |   92.1 |            -1.50 |     92.1 |
| Async chunks read            |    16.40 |          -1.44 |   91.7 |            -2.54 |     86.6 |
| Async sort and write         |    15.11 |          -1.29 |   92.1 |            -3.83 |     79.8 |
```
Aaaaaand another ~7% speedup. Next, please!



# Inserting at the end of B-tree node
This one is a very simple optimization. I don't know why I didn't think of it earlier.

Previously, to insert a value into a B-tree node, btree_node::insert() method was used. It looks like this:

```cpp
uint32_t btree_node::insert(const sha1_t& sha1)
{
  // place_for() std::lower_bound() to find a place.
  const auto place = place_for(sha1);

  if (keys_count > 0) {
    const auto begin = std::begin(keys);
    std::copy_backward(std::next(begin, place), std::next(begin, keys_count),
                       std::next(begin, keys_count));
  }

  keys[place] = sha1;
  ++keys_count;

  return place;
}
```
So, it searches for a place where hash should be written. Moves all greater hashes one place to the right and sets the hash at the found place.

Again, we can take advantage of the fact that we insert sorted values into B-tree. With that in mind, we know that the inserted hash is greater than all the hashes that are already stored in the node. So we don't need to search for a place. We already know it. It's the place after the last stored hash. Instead of using insert(), I've created a method push_back() that looks like this:

```cpp
void btree_node::push_back(const sha1_t& sha1)
{
  keys[keys_count++] = sha1;
}
```
No magic here. Now, in btree_sorted_keys_inserter class, with only one line change, from:

```cpp
template <typename DataStorage>
void btree_sorted_keys_inserter<DataStorage>::insert_sorted(const sha1_t& sha1)
{
  if (current_node().is_full()) {
    split_node(sha1);
  } else {
    current_node().insert(sha1);
  }
}
```
to

```cpp
template <typename DataStorage>
void btree_sorted_keys_inserter<DataStorage>::insert_sorted(const sha1_t& sha1)
{
  if (current_node().is_full()) {
    split_node(sha1);
  } else {
    current_node().push_back(sha1); // Here
  }
}
```
we get such results:

```
|      Optimization phase      | time [s] | delta last [s] | % last | delta master [s] | % master |
|------------------------------|---------:|---------------:|-------:|-----------------:|---------:|
| None                         |    18.94 |            N/A |  100.0 |              N/A |    100.0 |
| Nodes caching                |    17.44 |          -1.50 |   92.1 |            -1.50 |     92.1 |
| Async chunks read            |    16.40 |          -1.44 |   91.7 |            -2.54 |     86.6 |
| Async sort and write         |    15.11 |          -1.29 |   92.1 |            -3.83 |     79.8 |
| Inserting at the end of node |    13.88 |          -1.23 |   91.9 |            -5.06 |     73.3 |
```
Beng! Another ~6% speedup.


# Text SHA1 to binary
(Please note that we assume that hashes are UPPERCASE)

As you probably know, HIBP files are text files. At some point the text hashes have to be converted to their binary form. Before changes, it was done like this:

```cpp
constexpr uint8_t char_to_value(char c)
{
  switch (c) {
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      c -= 'A' - ':';
      [[fallthrough]];
    default:
      c -= '0';
  }

  return static_cast<uint8_t>(c);
}

constexpr uint8_t two_first_chars_to_byte(const char* cs)
{
  uint8_t byte = static_cast<uint8_t>(char_to_value(cs[0u])) << 4u;
  byte |= char_to_value(cs[1u]);
  return byte;
}

sha1_t string_sha1_to_binary(std::string_view sha1_text)
{
  sha1_t sha1;

  for (auto i = 0u; i < 40u; i += 2u) {
    sha1[i / 2u] = two_first_chars_to_byte(sha1_text.data() + i);
  }

  return sha1;
}
```
You iterate over the whole text, two characters at a time, convert chars to values and store in the result hash. As you can see, there are 20 iterations that do very same calculations. Why not vectorize? SIMD!


# SIMD
First of, I'm a SIMD noob. I've never played with it before. If you see anything that can be improved, please let me know (:

(Currently okon assumes little endian)

I decided to use [vectorclass/version2](https://github.com/vectorclass/version2) library. It's very straightforward and provides simple interface.

The converting function looks like this:

```cpp
sha1_t simd_string_sha1_to_binary(const void* text)
{
  vcl::Vec64uc v8;
  v8.load(text);

  const auto is_alpha_mask = (v8 & 0b01000000u) != 0u;

  v8 &= vcl::Vec64uc{ 0x0fu };
  v8 = if_add(is_alpha_mask, v8, vcl::Vec64uc{ 9u });

  const auto shifted_v8 = v8 << 4u;
  const auto shifted_v16 = vcl::Vec32us{ shifted_v8 } << 8u;

  const auto v16 = vcl::Vec32us{ v8 };
  const auto result = (v16 | shifted_v16) >> 8u;

  std::aligned_storage_t<sizeof(uint8_t) * 64u, 32u> result_storage;
  auto result_storage_ptr = reinterpret_cast<uint8_t*>(&result_storage);
  result.store_a(result_storage_ptr);

  sha1_t result_sha1{};
  const auto result_storage_16_ptr = reinterpret_cast<const uint16_t*>(result_storage_ptr);
  for (auto i = 0; i < result_sha1.size(); ++i) {
    result_sha1[i] = result_storage_16_ptr[i];
  }

  return result_sha1;
}
```
Looks complicated, but it's not. Let's go through it with an example - conversion of two characters to a single byte of data.

For example, we have a string "E3" and we want to convert it to value 227 that it represents.

So, text points to memory where string "E3" is stored.

We create a variable v8 of type Vec64uc. The type name means: 'a vector of 64 unsigned chars'. So we're manipulating 64 characters at a time!

![foo](/assets/00020-optimizing-okons-b-tree-file-creation/no_you_dont.jpg)

Unfortunately, Ms. Armstrong is right.

Such instructions exist but not on the processor that I have. In fact (on my processor) `Vec64uc` to operate on 512 bits, uses two `Vec256b` values. `Vec256b`, to operate on 256 bits, uses two `__m128i` values. So, when I perform an operation on `Vec64uc` value, I perform four operations on `__m128i` values.

Nevertheless, the library API makes it completely transparent (which is great!), so I'll refer to the operations in the algorithm as if they would operate on the full hash at a time.

Ok, back to the algorithm. We have a string "E3". We load it to the variable v8:

```
// v8.load(text);
      'E'       '3'
v8: 01000101 00110011
```
Now we create a mask. A mask is something like a vector of bools. It can be used in conditional operations. So, for every byte, we perform & 0b01000000 and check whether it is 0.

```
// const auto is_alpha_mask = (v8 & 0b01000000u) != 0u;
v8:   01000101 00110011
             and
      01000000 01000000
              =
      01000000 00000000
             != 0
mask:   true    false
```
Next we & every byte of v8 with 0x0f.

```
// v8 &= vcl::Vec64uc{ 0x0fu };
v8:   01000101 00110011
             and
      00001111 00001111
              =
v8:   00000101 00000011
```
Now we use the mask add 9 to every element of v8 that contains true in its corresponding mask value.

```
// v8 = if_add(is_alpha_mask, v8, vcl::Vec64uc{ 9u });
mask:   true     false
v8:   00000101 00000011
              +
         9        0
              =
v8:   00001110 00000011
```
Next, we create a shifted version of v8:

```
// const auto shifted_v8 = v8 << 4u;
v8:           00001110 00000011
                    << 4
                      =
shifted_v8:   11100000 00110000
```
Create a vector of 16 bit values out of shifted_v8 and shift it to left (remember: little endian).

```
// const auto shifted_v16 = vcl::Vec32us{ shifted_v8 } << 8u;
shifted_v8:             11100000 00110000
Vec32us{ shifted_v8 }:  00110000'11100000
shifted_v16:            11100000'00000000
```
Next, we create a vector of 16bit values out of v8 (remember: little endian).

```
// const auto v16 = vcl::Vec32us{ v8 };
v8:   00001110 00000011
v16:  00000011'00001110
```
And now, compute the result:

```
// const auto result = (v16 | shifted_v16) >> 8u;
v16:         00000011'00001110
                     or
shifted_v16: 11100000'00000000
                     =
             11100011'00001110
                   >> 8
result:      00000000'11100011
```
Thanks to that, the result is a vector of 16 bit values that have binary value of SHA-1 hash. Of course, we need only 20 first values.

The last thing is to create an aligned storage for the result values and store the first 20 values in the result_sha1.

And here are performance impact:

```
|      Optimization phase      | time [s] | delta last [s] | % last | delta master [s] | % master |
|------------------------------|---------:|---------------:|-------:|-----------------:|---------:|
| None                         |    18.94 |            N/A |  100.0 |              N/A |    100.0 |
| Nodes caching                |    17.44 |          -1.50 |   92.1 |            -1.50 |     92.1 |
| Async chunks read            |    16.40 |          -1.44 |   91.7 |            -2.54 |     86.6 |
| Async sort and write         |    15.11 |          -1.29 |   92.1 |            -3.83 |     79.8 |
| Inserting at the end of node |    13.88 |          -1.23 |   91.9 |            -5.06 |     73.3 |
| SIMD                         |    10.59 |          -3.29 |   76.3 |            -8.35 |     55.9 |
```
~24% speedup by changing just one function \o/

(Another advantage of the current algorithm is that it works for lowercase hashes, as well)


# Overall performance improvement
After benchmarking all the changes on the 55 mil file, I've benchmarked okon on the original HIBP file. Results:

SSD:
```
| Optimization phase | time [s] | delta [s] |   %   |
|--------------------|---------:|----------:|------:|
| before             |    477.2 |       N/A | 100.0 |
| after              |    342.2 |   -134.96 |  71.7 |
```

HDD:
```
| Optimization phase | time [s] | delta [s] |   %   |
|--------------------|---------:|----------:|------:|
| before             |    624.7 |       N/A | 100.0 |
| after              |    407.6 |   -217.11 |  65.2 |
```

Not that bad. 28% faster on SSD and 35% on HDD.

The question is: why benchmarks on a smaller file result in much better speedup (it was 44%)?

I suspect disk operations, especially the B-tree file creation. If you read the original file and distribute hashes into small files for sorting, the only thing that is left to do is to create a B-tree and write it to the file. Unfortunately, I don't think that it can be parallelized. Because of that, the bigger B-tree you need to create, the bigger is disproportion of operations that you can do in parallel and the one you need to do simultaneously. This leads to the speedup drop if you operate on the original file.

Anyway, it's a lot better than it was before changes, so I consider all this as a success (:


# Links and discussion
- [okon GitHub](https://github.com/stryku/okon)

If you have any thoughts, let me know using one of these:
- [GitHub issue](https://github.com/stryku/okon/issues)
- [r/cpp](https://www.reddit.com/r/cpp/comments/fw67d3/optimizing_okons_btree_file_creation_going_down/)
- [Hacker news](https://news.ycombinator.com/item?id=22797407)


# Thanks for reading o/
