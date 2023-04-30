---
layout: post
title:  "Open source fun #0: Recursive clang-format"
date:   2017-08-24 00:00:00 +0200
categories: 
published: true
---

# Table of Contents:
1. This will become a table of contents.
{:toc}

# Abstract

clang-format (CF) is a great tool, no doubt. I was using it on Linux for a while, for my hobby projects. Most of the time, like this: `clang-format *.cpp *.hpp` and it worked like a charm.
And then I wanted to use it on Windows. I'm sure that I wasn't the very first person on earth that had a little WTF just after running above command in the Windows' cmd.

`invalid argument`

Quick Google session and there's the tl;dr answer:

In the command `clang-format *.cpp *.hpp` Linux' shell expands the 'wildcards regexs' with matching files before the actual command is called, so in fact we're not calling `clang-format *.cpp *.hpp`. We `call clang-format main.cpp Foo.cpp Foo.hpp` etc...

What happens on windows? Nothing. Wildcard is not expanded so `clang-format *.cpp *.hpp` is called and immediately fails, because obviously we don't have `*.cpp` and `*.hpp` files in the project.

# Ready-made solutions

Over the internet, there are a couple of ready-made solutions for windows. Most people suggests writing a bash script or so. Personally, I didn't want to add an additional script to the project to just format couple of files only on windows.

After double check that CF doesn't support such feature, I did what each self-respecting C++ dev would do: 
`git clone https://github.com/llvm-mirror/clang`

# The Plan

First of all, what do I want to achieve? I'd like to pass a regular expression as a CF argument and expect that CF will expand it to the list of matching files itself.
- Q: Do I want to do this recursively?
- A: Hell yea!

- Q: Do I want this to be a new kind of command line argument?
- A: Yes, I think it's a good idea.

- Q: How it'll be called?
- A: -r?

Time to dive in the code. The Plan is simple.

1. Find how arguments are currently handled in the code.
2. Implement a new one, on which your brand new function/class method will be called.
3. Figure out how you can recursively list files. Keep in mind that LLVM and its tool projects has more code than you'll write in your whole life (work + lolhobbyprojects). I bet this problem was already addressed.
4. Implement your function/class method.
5. Bind it with the CL argument handler.
6. Profit.
7. Polish your autograph, in case Herb Sutter would want one.


# 1.

Current argument handling.

Let's just grep sources for some weird argument, maybe we'll find something. `assume-filename` looks promising, such stuff in C++ code most likely will be in a string literal - that's what we're looking for. The only C++ file listed is `tools/clang-format/ClangFormat.cpp`. Sweet, it must be that!

_NARRATOR: Let's look into the Mateusz's brilliant, absoultely brilliant mind:

- looks in the tools/clang-format folder
- realizes that there is ONLY ONE C++ file - ClangFormat.cpp
- bares in mind to not ram though an open door and ffs just look into the project folder_

# 2.

Yep. There is only one file and all arguments are handled there. Anyway, let's write our own:

```cpp
static cl::opt<bool> Recursively("r",
                                 cl::desc("Recursively search for <file>s."),
                                 cl::cat(ClangFormatCategory));
```

# 3.

I shouldn't be so hard, let's just grep for `directory_ite`... NO. Look on the code. Maybe similar case is already handled in the `ClangFormat.cpp`. 20h later (I went sleep and to work) No. It's not, soooo.. THE GREP! Let's grep for `directory_iterator`. If such feature is implemented, it's called like that. Here it is: `Basic/VirtualFileSystem.h`


# 4.
It's show time!
I mean, it'll be more like a pathetic stand-up in 'Bar Cezar', Węgorzewo.
But I'll be my stand-up!

Algorithm is rather simple.
```
for every listed file
    for every passed regex
        if file name match regex
            format shit out
```

 shit out
4.1
Find matching files.
There is a ready-made Regex class. Surprised?

`#include "llvm/Support/Regex.h"`

Now we need to iterate through the files in our and child directories. You know what? We're AGAIN lucky bastards. There is one little function `llvm::sys::fs::current_path`. I bet you think that it returns current path (the one where clang-format was called). Ye, I though so. And we're both right. It returns current path. With that enormous knowledge we're finally able to write a `findMathichFiles` function.

```cpp
bool findMatchingFiles(std::vector<std::string> &files)
{
    SmallVector<char, 256> currentPath;
    std::error_code ec;
    llvm::sys::fs::file_status fileStatus;
    llvm::sys::fs::current_path(currentPath);

    for (llvm::sys::fs::recursive_directory_iterator I(currentPath, ec), E;
         I != E; I.increment(ec)) {
        if (ec) {
            errs() << "error: error wile recursive files formatting. Error code: "
                << ec.value();
            return true;
        }

        I->status(fileStatus);

        if (fileStatus.type() == llvm::sys::fs::file_type::regular_file)
        {
            std::vector<Regex> regexs{ std::cbegin(FileNames), std::cend(FileNames) };

            for (auto& regex : regexs) {
                if (regex.match(I->path())) {
                    files.push_back(I->path());
                }
            }
        }
    }

    return false;
}
```

# 4.2

Time to format shit out.

We have files path. First we need to write a function that formats a single file. Well, no. clang-FORMAT would be a horrible tool if it wouldn't be able to format a single file. There is a ready-made shiny function `clang::format::format`, let's just use this one.

```cpp
static bool formatRecusively() {
    std::vector<std::string> files;
    if (checkMultipleFilesPreconditions() || findMatchingFiles(files)) {
        return false;
    }

    bool Error = false;

    for (unsigned i = 0; i < files.size(); ++i) {
        Error |= clang::format::format(files[i]);
    }

    return Error;
}
```

# 5.
Binding is really easy. Even I figured it out without creating a forum thread. Remember option that we've created in 2nd point? Now we just need to check it and if it's true run our `formatRecusively()`

# 6.

![foo](/assets/recursive-clang-format/profit.jpg)

# 7.

![foo](/assets/recursive-clang-format/autograph.png)

# All in one

I've added these changes to my open-source-fun repo: [https://github.com/stryku/open-source-fun](https://github.com/stryku/open-source-fun) If you don't know wtf is this - check the README (:

Don't want to go to some lame repo? Here you can see all changes that I made.

```diff
diff --git a/tools/clang-format/ClangFormat.cpp b/tools/clang-format/ClangFormat.cpp
index 37c2d8b78f..ff71b4aac8 100644
--- a/tools/clang-format/ClangFormat.cpp
+++ b/tools/clang-format/ClangFormat.cpp
@@ -22,6 +22,7 @@
 #include "clang/Rewrite/Core/Rewriter.h"
 #include "llvm/Support/CommandLine.h"
 #include "llvm/Support/FileSystem.h"
+#include "llvm/Support/Regex.h"
 #include "llvm/Support/Signals.h"

 using namespace llvm;
@@ -106,9 +107,24 @@ static cl::opt<bool>
     Verbose("verbose", cl::desc("If set, shows the list of processed files"),
             cl::cat(ClangFormatCategory));

+static cl::opt<bool> Recursively("r",
+                                 cl::desc("Recursively search for <file>s."),
+                                 cl::cat(ClangFormatCategory));
+
 static cl::list<std::string> FileNames(cl::Positional, cl::desc("[<file> ...]"),
                                        cl::cat(ClangFormatCategory));

+static bool checkMultipleFilesPreconditions() {
+  if (FileNames.size() != 1 &&
+      (!Offsets.empty() || !Lengths.empty() || !LineRanges.empty())) {
+    errs() << "error: -offset, -length and -lines can only be used for "
+              "single file.\n";
+    return true;
+  }
+
+  return false;
+}
+
 namespace clang {
 namespace format {

@@ -329,6 +345,51 @@ static bool format(StringRef FileName) {
   return false;
 }

+bool findMatchingFiles(std::vector<std::string> &files) {
+  SmallVector<char, 256> currentPath;
+  std::error_code ec;
+  llvm::sys::fs::file_status fileStatus;
+  llvm::sys::fs::current_path(currentPath);
+
+  for (llvm::sys::fs::recursive_directory_iterator I(currentPath, ec), E;
+       I != E; I.increment(ec)) {
+    if (ec) {
+      errs() << "error: error wile recursive files formatting. Error code: "
+             << ec.value();
+      return true;
+    }
+
+    I->status(fileStatus);
+
+    if (fileStatus.type() == llvm::sys::fs::file_type::regular_file) {
+      std::vector<Regex> regexs{std::cbegin(FileNames), std::cend(FileNames)};
+
+      for (auto &regex : regexs) {
+        if (regex.match(I->path())) {
+          files.push_back(I->path());
+        }
+      }
+    }
+  }
+
+  return false;
+}
+
+static bool formatRecusively() {
+  std::vector<std::string> files;
+  if (checkMultipleFilesPreconditions() || findMatchingFiles(files)) {
+    return false;
+  }
+
+  bool Error = false;
+
+  for (unsigned i = 0; i < files.size(); ++i) {
+    Error |= clang::format::format(files[i]);
+  }
+
+  return Error;
+}
+
 }  // namespace format
 }  // namespace clang

@@ -368,14 +429,16 @@ int main(int argc, const char **argv) {
     return 0;
   }

+  if (Recursively) {
+    return clang::format::formatRecusively();
+  }
+
   bool Error = false;
   if (FileNames.empty()) {
     Error = clang::format::format("-");
     return Error ? 1 : 0;
   }
-  if (FileNames.size() != 1 && (!Offsets.empty() || !Lengths.empty() || !LineRanges.empty())) {
-    errs() << "error: -offset, -length and -lines can only be used for "
-              "single file.\n";
+  if (checkMultipleFilesPreconditions()) {
     return 1;
   }
   for (const auto &FileName : FileNames) {

```

# Patch

Of course, I want to be famous so, I've submitted a patch to CF while ago. Here it is: [https://reviews.llvm.org/D29039](https://reviews.llvm.org/D29039). From time perspective I can see that code there could be better. Besides that, clang guys decided that recursive formatting it's not a good idea.

# Thanks for reading o/
