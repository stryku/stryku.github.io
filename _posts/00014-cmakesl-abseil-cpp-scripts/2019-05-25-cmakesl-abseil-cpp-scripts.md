---
layout: post_without_latex
title:  "CMakeSL - abseil-cpp scripts"
date:   2018-10-12 00:00:00 +0200
categories: cpp
published: true
---


# Links
* [CMakeSL RSS](http://stryku.pl/rss.xml) - I'm going to reuse my page feed. I'm not going to post every article at `r/cpp` or `r/cmake`, so if you want to be notified about new CMakeSL arts, the RSS is the only reliable way.
* [CMakeSL repo](https://github.com/stryku/cmakesl)

# CMakeSL - abseil-cpp scripts

This article is meant to present what's going on with the CMakeSL project. If you don't know what the CMakeSL is, check out this [gist](https://gist.github.com/stryku/50903f147c899a84c20515a6bc5fab97) or the [readme](https://github.com/stryku/cmakesl).

# CMakeSL changes

Actually, not much happened in the CMakeSL itself. Travis and a couple of bug fixes.

# abseil-cpp scripts

I've recently focused on creating CMakeSL scripts for abseil-cpp project. Reason for such an action was that I wanted to make scripts for a bigger project and see how CMakeSL fits there. Other (and probably more important) purpose was to see what needs to be implemented in the language itself, as well as, what builtin types and functions are still missing. Those scripts are for now a kind of roadmap for CMakeSL development. CMakeSL is not able to handle those scripts yet.

You can find abseil-cpp repo fork on my [github](https://github.com/stryku/abseil-cpp). CMakeSL scripts are the CMakeLists with a `.cmsl` extension.

## Stuff that needs to be implemented

There is a lot of things that still need to be implemented to make abseil-cpp scripts interpretable by CMakeSL. Here is a couple of them:

### Language core
* Modules. Not much to say here. Extracting code to separate files is a must-have.
* Namespaces. Dealing with external libraries or even internally, in a complex build system, namespaces can help with the code structure.
* Enums. In the abseil-cpp scripts, there is no enum defined but, actually, they are used. `cmake::cxx_compiler_id` is an example of such.
* Type deduction for variable and function return types.
* Ternary operator.
* Designated initializers. It becomes a extremely handy in cases like calling `absl::cc_library` or `absl::cc_test`.
* and more...

### Builtin stuff
* `cmake::find_package`. It'd return a structure with information about found (or not) package.
* `cmake::warning`, `cmake::fatal_error` etc. Like `message(WARNING/INFO/FATAL_ERROR ...)`
* `cmake::export`, `cmake::install*` and other functions helping with packaging/installation.
* `option` builtin type. Equivalent to `option(NAME "Description" ON/OFF)`
* and more...

### Things that I'm not sure about/I don't like

* `cmake::find_package*()`
CMakeSL has to give an option for finding 'required' packages and immediately fail if it was not found. I thought about providing a builtin function with a signature similar to: `find_package_result find_package( ... , presence_requirement presence, ...)` where `presence_requirement` would be an enum, e.g.:
```cpp
enum presence_requirement {
    required,
    optional
};
```
This signature is readable and provide flexibility. It could be handy in e.g. utils code in your build system. On the other hand, it's quite verbose and it'd be nice to provide a more user-friendly interface, so I thought about making `presence` parameter with default value `presence_requirement::optional` and creating one more signature: `find_package_result find_package_required(...)`. Thanks to that, end-user code is obvious just by looking at the called function name:
```cpp
auto zlib = cmake::find_package_required("zlib", ... );
```
I'm not sure if that's a good idea, though. I'll need to discuss it with someone.

* `isset(variable)`
One difference between CMakeSL and C++ is that, in CMakeSL variables can be provided from the outside, e.g. `cmake::install_prefix`. User has to have a functionality to check whether a given variable exists. I'm not sure whether `isset()` function is really what we want. It's another topic to discuss.

* `cmake::declare_library()`
CMakeSL is statically typed. If you want to use a variable, it has to exist. If you want to link library `A` to library `B`, both libraries have to be there and then you can call `B.link_to(A)`.

abseil-cpp project introduces a lot of libraries. Because of above `A-B-problem`, in the CMakeSL scripts you'll find a lot of `cmake::declare_library()` function calls to declare library variables, so other libraries can link to them.

At first glance it looks awful. You have a lot of code that just declares library variables. At second glance it looks awful too, and, in fact, it won't look any better. It is just as it is. But, at the end of the day, it's code that you can easily understand, edit and maintain. CMakeSL error reporting will help you to find bugs, indexer helps with navigation between declaration, usage and other stuff. Well, in CMakeSL you can handle such cases as you want. You could create a util function that creates a structure/map of such declared libraries and returns it, so you'd have the code in one place. You can do anything you want.

Anyway, approach for handling this problem will be discussed further.

# What's next?
I want to develop CMakeSL to the extent that it's able to build abseil-cpp project.

Then, I'm going to write CMakeSL scripts for a really big project like llvm + clang. Then develop CMakeSL to handle those scripts. I believe that, after this it'll be easier to decide whether CMakeSL is helpful and should be developed further.

# Thank you
Thanks for reading. If you have any thoughts, feel free to comment under this gist. If you prefer to send a direct email, you can find it [here](http://stryku.pl/index.php?selected=4).

Stay tuned (:


# Thanks for reading o/
 