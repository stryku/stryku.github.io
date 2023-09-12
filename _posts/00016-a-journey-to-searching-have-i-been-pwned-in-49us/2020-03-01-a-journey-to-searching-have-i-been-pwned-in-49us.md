---
layout: post_without_latex
title:  "A journey to searching Have I Been Pwned database in 49μs"
date:   2020-03-01 00:00:00 +0200
categories: cpp
published: true
---

# The Problem

I knew about existence of [Have I Been Pwned (HIBP)](https://haveibeenpwned.com/) for a long time. I never used it, though, until recently. I've always used a couple of non-generated passwords. One of them was used more than once in a trash-email and a couple of account on weird sites. Well, was, because the email got hacked. And to be honest, I'm thankful for the hacker because this event triggered me to rethink my passwords, how I use and store them.

Of course, I changed passwords in every account that used the hacked one. Then, just out of curiosity, I wanted to check whether the hacked password is present in HIBP database. I didn't want to use the site, so I downloaded the database (pwned-passwords-sha1-ordered-by-count-v5). The databse is quite impressive. It's a 22,8G text file that contains a set of SHA-1 hashes, one in every line with a counter, how many times a password with a given hash has been seen in breaches. I calculated SHA-1 of my hacked password and attempted to search for it.

# [G]old grep

The file is a text file with one SHA-1 per line, so grep seems to be the best option for the task.

```
grep -m 1 '^XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX' pwned-passwords-sha1-ordered-by-count-v5.txt
```

The password was not the best password in the world, so the line appeared immediately (with a count of more than 1500, so the password pretty much sucked).

But hey, not all of us use weak passwords. I wanted to check how long it would take to search for the worst case - the last hash in the file:


```time grep -m 1 '^4541A1E4605EEBF3F4C166329C18502DF75D348A' pwned-passwords-sha1-ordered-by-count-v5.txt```

Result: `33,35s user 23,39s system 41% cpu 2:15,35 total`

I was upset. Since my email got hacked I wanted to check for all of my old passwords in the database and check every new one too. But, two-minute grepping is just not convenient. Of course, I could make a script, run it and go for a walk, but that's not the way. I wanted a better solution and to learn something, so I decided to invent the solution.


# Trie

The first idea was to use a trie data structure. The structure seems perfect for storing SHA-1 hashes. The alphabet is small, so one node would be small too. Thanks to that the result file shouldn't be to big. Maybe it'll even fit in RAM? Searching for a key would be super fast.

So I implemented it. Then, I took the first 1'000'000 hashes of the original database to build a result file and test whether all of them are findable in the created file.

Yep, I was able to find all of them in the file, so my structure worked well. But, there was another problem.

The result file was of size 2283686592B (2,2G). That's worrying. Let's do the math and see what's going on. A node is a simple structure of sixteen 32 bit values. The values are 'pointers' to next nodes, at given character of the SHA-1 hash. So, one node takes 16 * 4B = 64B. Not that much, huh? Not so fast. One node represents one character in the hash. So, in the worst case, SHA-1 hash takes 40 * 64B = 2560B. That's not so well, comparing to e.g. text representation of the hash, which takes just 40B.

The power of the trie structure lies in nodes reusing. If you have two words: aaa and abb, the node representing the first characters is reused because the characters are the same - a.

Back to our problem. Let's calculate how many nodes are stored in the result file: file_size / node_size = 2283686592 / 64 = 35682603

Now let's see how many nodes would be created in the worst case out of 1'000'000 SHA-1 hashes: 1000000 * 40 = 40000000

So, trie reuses only 40000000 - 35682603 = 4317397 nodes which is 10.8% of the worst case scenario.

With such numbers, a result file for the whole HIBP database would take 1421513361920B (1.02T). I don't even have that big HDD to test the speed of searching keys in a such trie.

That day I learned: Trie structure sucks if you have pretty random words.

Let's figure out a different solution.


# Binary search

SHA-1 hashes have two nice traits: * They are comparable * They are all the same size.

Thanks to that, we can process the original HIBP database and create a file that consists of sorted SHA-1 values.

But, how to sort a 22G file?

> Q: Why do you need to sort the original file? HIBP provides a file with lines already sorted by hashes.
> A: I simply missed that. At this point I didn't know about the sorted file.


## Sorting

Sorting all the hashes in RAM is not an option - I don't have that much RAM. The solution was: 1. Divide the big file into smaller ones, that fit in RAM 2. Load data of small files, sort in RAM and write back to files. 3. Merge all small, sorted files into one big.

With a sorted big file, you can use binary search to search for your hash. Hard drive access matters. Let's calculate how many accesses are needed while binary searching: log2(555278657) = 29.0486367039, so 30 accesses. Not that bad.

In the first step an optimization can be made. Transform text hashes to binary data. This cuts result data size by half, which results in storing not 22G of data, but 11G instead. Nice.

## Why merge back?

At this point I realized that this can be done smarter. What if we don't merge small files into a big one and binary search in the small sorted ones, in RAM? The problem here is how to find the file where we should search for out key? Solution is very simple. New approach:

1. Create 256 files named "00" .. "FF".
2. While reading hashes from big file, write hashes starting with "00.." to file named "00", hashes starting "01.." to file "01" and so on..
3. Load data of small files, sort in RAM and write back to files.

Simple as that. Additionally, it gives us one more option to optimize. If a hash is stored in file "00", you know that it starts with "00". If a hash is stored in file "F2", you know that it starts with "F2". Sweet. While writing hashes to small files, we can omit the first byte of every hash! That's 5% of the data. In scope of all the hashes, you need to store 555MB less data.


## Parallelism

Dividing into smaller files gives us one more opportunity to optimize. The files are independent of each other, so we can sort them in parallel. Remember, all of your CPUs like to sweat at once ;)


# Not being a selfish bastard

When I implemented the above solution, I realized that I can't be the only one with similar problem. There are probably a lot of people, wasting their computing power for grepping over HIBP databases. So, I decided to share my work.

Before that, I made a step back and reviewed my approach once again. I found a couple of issues that I would want to fix before delivering code and tools to github.

First of, as an end user I would not want to use a tool that creates a lot of weird files with weird names, that I don't know what they store etc.

Ok, that can be solved by merging the "00".."FF" files into one big.

Unfortunately having one big file that needs to be kept sorted introduces another issue. What if I want to insert a hash to this file? Just one hash. That's just 20 bytes of data. Oh, the hash starts from "000000000..". Well, ok. Let me make space for it by moving 11G of other hashes..

You get the point. Inserting stuff in a middle of file in not the fastest thing in the world.

Another flaws of this approach is that we need to store the first bytes again - that's 555MB of data.

Last but not least, binary search over data stored on HDD is a lot slower than jumping in RAM. I mean, it's 30 disk reads vs 0 disk reads.


# B3

Once again. What we have and what we want to achieve.

We have 11G of binary values. All the values are comparable and of the same size. We want to find whether a given key exists in the stored data, as well as, be able to perform modifications. And we want to do all this instantly.



B-tree? B-tree! That's it.

B-tree allows you to minimize disk accesses while searching, modification etc. It has a lot more features, but we need these two.


## Sorted insertion

The first step is to transform data from original HIBP file into a B-tree. That means getting hash by hash and inserting in into the structure. It can be done using the regular insertion algorithm. But, in our case it can be done better.

Inserting a lot of initial data into a B-tree is a well known case. Wise people invented a better approach for this than the regular insertion algorithm. First of, we need to sort the data. That can be done with the approach above (divide into smaller files and sort them in RAM). Then, insert the data into tree.

In the regular insertion algorithm, if you find a leaf node where you need to insert a value, and the leaf node is full, you create a new (right) node and evenly split values over the two (left and right) nodes (plus one value goes to parent, but that's not important here). In short, all values in the left node are smaller than all values in the right node. The thing is, when you insert sorted data, after node split, you know that no more smaller values are going to be inserted to the tree, so no more values will go to the left node. The left node will remain full-empty all the time. What's more if you insert enough values, you may find out that the right node is full, so you need to move half of the values to the new right node. The split node remains half-empty, similarly to the previous case. And so on...

In result, after all insertions you'll end up with a tree that almost all the nodes are half-empty. That's not so space-efficient. We can do better.



## To split or not to split?

In a case of inserting sorting data, a small modification of insertion algorithm can be made. If the node where you need to insert a value is full, don't split it. Just create a new, empty node and insert the value to parent. Then, when you insert next values (which are greater than previous ones), you'll insert it into the fresh, empty node.

To preserve B-tree properties, after all insertions, you need to iterate over rightmost nodes in every layer of the tree (all but the root one) and evenly split values of the node and its left sibling. After that, you get the smallest tree you can get.


## HIBP tree properties

When designing a B-tree, you need to decide of what order it should be. Order of a tree tells how much values you can store in one node, as well as, how many children can the node have. So, by manipulating the order you can manipulate the tree height, binary size of the node etc.

In HIBP we have 555278657 hashes. Let's say we want the tree to have height three (which means that we need at most three disk reads to check whether a hash is present in the tree). We need to find a value M that logM(555278657) < 3. I chose 1024. That's not the smallest possible value, but it leaves possibility to insert more hashes and preserve the tree height.


## Output file

Original downloaded HIBP file takes 22.8G. Output file with B-tree takes 12.4G. Building the B-tree file takes around 11 minutes on my machine (Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz, 16GB RAM), on HDD (not SSD).


# Benchmarks

The B-tree approach is pretty neat \o/ Beng:

(EDIT: I've updated values, because the grep ones were wrong)

```
|                  |   time [μs] |           % |
|-----------------:|------------:|------------:|
|             okon |          49 |         100 |
|     grep '^hash' | 135'350'000 | 276'224'489 |
|             grep | 135'480'000 | 276'489'795 |
| C++ line by line | 135'720'201 | 276'980'002 |
```

# okon - library and CLI

As I said, I wanted to share my work with the world. I implemented a library and a command line interface binary for processing HIBP databases and fast searching for hashes in it. The searching is so fast that it can e.g. be integrated into password manager and give feedback to the user on every key stroke. There is a lot of possible use cases.

Library has a C language interface, so it can be used pretty much everywhere. CLI is, well, CLI. You can build it and just use (:

They are available on my github: [github.com/stryku/okon](https://github.com/stryku/okon).

Disclaimer: okon doesn't provide interface to insert values to already created btree yet. For now it's able to process HIBP file, create a B-tree out of it and search in it. It works, so I decided to share it and work further on inserting and other possible features.


# Links and discussion
Feel free to leave a comment/issue in any of the thread/repo:

- [Trie data structure](https://en.wikipedia.org/wiki/Trie)
- [B-tree data structire](https://en.wikipedia.org/wiki/B-tree)
- [Have I Been Pwned site](https://haveibeenpwned.com/)
- [Code repository - github.com/stryku/repo](https://github.com/stryku/okon)
- [r/cpp](https://www.reddit.com/r/cpp/comments/fc1dsi/a_journey_to_searching_have_i_been_pwned_database/)
- [r/security](https://www.reddit.com/r/security/comments/fc1drh/a_journey_to_searching_have_i_been_pwned_database/)
- [Hacker News](https://news.ycombinator.com/item?id=22459661)


# Thanks for reading o/
 