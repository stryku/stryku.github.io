---
layout: post_without_latex
title:  "A journey to waking up and searching Have I Been Pwned database in 3ms (SSD) or 14ms (HDD), not 49μs"
date:   2020-03-12 00:00:00 +0200
categories: cpp
published: true
---


# A journey to waking up...

...and searching Have I Been Pwned database in 3ms (SSD) or 14ms (HDD), not 49μs

With this article I want to clarify performance of okon. I want to describe how it was previously benchmarked and why these benchmarks were bad. Later on, I'll present how it is benchmarked now and what's the current performance.

But, first of, I want to admit that the previous benchmarks were bad and okon is not able to find a hash in 49μs. I want to apologize anyone who feels that I lied/cheated. Didn't want to lie or to create a clickbait out of the previous article. I simply missed one 'small detail' in benchmarks. I really believed that okon is that fast, that's why I created an article with such title and presented the library like it's that fast. Sorry (:

# The small detail

When I wanted to benchmark okon and compare it with other possible hash-finding solutions I decided that comparing best and worst case scenario is a good idea.

At the time, other solutions were grepping and finding hash line by line in C++. The best scenario for these two is trying to find the first hash, the worst case scenario is trying to find the last one (or trying to find a hash that is not present in the file).

Writing benchmarks for okon, I wanted to benchmark the same cases. So, I wrote benchmarks that try to find the first and the last hash from the original file. I used [google/benchmark](https://github.com/google/benchmark) library. To write a benchmark using this library, you write a loop and in the loop you write code that you want to benchmark. So, in okon it was something like this (this one benchmarks the worst case):

```cpp
static void exists_btree_worst_case(benchmark::State& state)
{
  while (state.KeepRunning()) {
    const auto result = okon_exists_text(OKON_BENCHMARK_WORST_CASE, OKON_BENCHMARK_BTREE_FILE);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(exists_btree_worst_case);
```

`OKON_BENCHMARK_WORST_CASE` and `OKON_BENCHMARK_BTREE_FILE` were #defines (passed to cmake).


As you can see, in the loop okon tries to find the same hash, in the same file again and again. Such behavior is not realistic from an end-user perspective. I'm sure that none of you would use okon to find the same hash several thousand times.

Because okon tries to find the same hash, it reads the same parts of B-tree file many times. Operating system seeing that someone reads the same parts of a file so many times, decides that it's a good idea to cache them in RAM. And that's the 'small detail' that I missed. The cache.

That's why benchmark result was 49μs. And that was true. Okon was really able to find that one hash in 49μs. But, that's only because the interesting parts of the file were cached in RAM.


# Doing benchmarks right (I think)

The biggest issue in benchmarks was not caring about cache. A real use-case is to try to find a hash that was not searched for before (or at least for some time). To be specific, in the real world, okon will read parts of B-tree file that are not cached. We need to guarantee that in benchmarks too.

To clear file system cache I used `sync; echo 3 > /proc/sys/vm/drop_caches` (you can read more in [the documentation](https://www.kernel.org/doc/Documentation/sysctl/vm.txt)). It's run before every attempt to find a hash by okon (and other solutions).

Another issue was that okon searched for the same hash over and over again. Now it randomly chooses 100 hashes from the original files and searches for them in a random order.

Time of every search is measured and the benchmark result is average of the times.

You can find more info and benchmarks code in the [README](https://github.com/stryku/okon#benchmarks) and the [benchmark folder](https://github.com/stryku/okon/tree/master/benchmark).


# Result

To summarize (and make this art more complete), here's more info about the benchmarks.

Searching in passwords file version 5, form HIBP page (8e1c0f161a756e409ec51a6fceefdc63d34cea01).

Benchmarked searching for 100 random hashes from the original file and calculated average time. The same hashes are used in grep and okon benchmarks.

Benchmarks are done on my PC:

- Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz, 16GB RAM.
- SSD: Model=Crucial_CT275MX300SSD1, FwRev=M0CR031
- HDD: Model=WDC WD10EZEX-21WN4A0, FwRev=01.01A01


## okon results

```
     time [ms]
SSD  3.405
HDD  13.779
```

Well, it's not 49μs. But, 13ms on HDD is not that bad, at least for now. I'll work on the project to make it better, for sure.



## grep results
Of course, I don't want to benchmark the grep tool. I don't want to compare it to okon either. These two solutions work differently, so direct comparison is pointless. It's more of getting an idea of how much time grep needs in an average case.

```
     time [s]
SSD  26.014
HDD  70.159
```

# Preprocessing
The last thing I want to mention is preprocessing done by okon. The tool needs to preprocess a file downloaded from HIBP site. It's done only once, before the first use. Time of this operation may matter, so I wanted to explicitly point in out here. Here are the times of preprocessing version 5 file on my PC:

```
     time [mm:ss]
SSD  6:47
HDD  10:19
```


# Discussion and links
If you have any thoughts, let me know using one of these:

- [Create a GitHub issue](https://github.com/stryku/okon/issues)
- [Hacker News](https://news.ycombinator.com/item?id=22561725)


# Thanks for reading o/
 