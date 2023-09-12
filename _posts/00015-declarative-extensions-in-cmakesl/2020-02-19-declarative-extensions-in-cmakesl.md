---
layout: post_without_latex
title:  "Declarative extensions in CMakeSL"
date:   2018-10-12 00:00:00 +0200
categories: cpp
published: true
---

# Declarative extensions in CMakeSL

# Teaser
A hello world executable created using declarative CMakeSL format.

![](https://media.giphy.com/media/jsZwXZwX8t5clEnB1D/giphy.gif)

# Table of contents
- [Declarative extensions in CMakeSL](#declarative-extensions-in-cmakesl)
- [Teaser](#teaser)
- [Table of contents](#table-of-contents)
- [Disclaimer](#disclaimer)
- [Why](#why)
- [What about "using one paradigm in the project"](#what-about-using-one-paradigm-in-the-project)
- [Imperative -> declarative](#imperative---declarative)
- [Declarative format](#declarative-format)
- [Paradigms integration](#paradigms-integration)
  - [add_declarative_file() and getting stuff from it](#adddeclarativefile-and-getting-stuff-from-it)
  - [add_subdirectory() with CMakeLists.dcmsl](#addsubdirectory-with-cmakelistsdcmsl)
  - [Declarative root file](#declarative-root-file)
- [Brief introduction](#brief-introduction)
  - [Builtin components](#builtin-components)
  - [forwarding_lists properties](#forwardinglists-properties)
  - [Custom components](#custom-components)
  - [Modules](#modules)
  - [Accessing old-style CMake variables](#accessing-old-style-cmake-variables)
- [Real world usage](#real-world-usage)
- [Thanks for reading](#thanks-for-reading)


# Disclaimer
None of what you'll find in this article is part of official CMake project. It's part of [CMakeSL](https://github.com/stryku/cmakesl) project.

The declarative format implementation is more of a POC of integration between two paradigms. All the concepts can change. A lot of key concepts (like conditional compilation etc.) are not implemented yet.

# Why
Declarative extensions were inspired mostly by [this comment from Craig Scott](https://gitlab.kitware.com/cmake/cmake/issues/19863#note_644504). To summarize, declarative files as leaf nodes are easy to parse and modify by tools. That means, writing a functionality that adds a source file to your project, as well as to CMakeLists is a piece of cake. 

Ok, maybe not THAT easy, but still much easier than interpreting and modifing an imperative context.

# What about "using one paradigm in the project"
You got me. Yes, I used that argument while releasing CMakeSL to the world. But, the Craig's comment convinced me to introduce declarative extensions.

# Imperative -> declarative
Why I call it 'declarative extensions'? That's because the declarative files are meant to be the leaf nodes of your CMakeLists files. There is actually one rule: When you enter a declarative file, you can NOT call/add/execute/take information from a file written in imperative CMakeSL. On the other hand, in an imperative file you can `add_subdirectory()` with a declarative file or create an executable using `add_declarative_file()` function.

So, you can _extent_ imperative files with declarative ones.

# Declarative format
The declarative format is inspired by Qbs (which uses a QML dialect). But, the format was not copied in 100%. I decided to tweak it a little to make some of its parts similar to analogous parts from the imperative format. E.g. in Qbs to assign a value to a property, you use a colon: `name: "hello_world"`. In declarative CMakeSL you use and equal: `name = "hello_world"`. In Qbs to 'derive' a component from another one, you use its name as the name of the component and you name the file as your shiny new component name. So, if you'd want to create a reusable component `MyReusableStaticLibrary`, you create a file `MyReusableStaticLibrary.qbs` where you define your component, using `StaticLibrary` as a root node:
```
StaticLibrary {
    ...
}
```
Then import it in client file and use it with `MyReusableStaticLibrary` name.

To create a reusable component in CMakeSL, you need to declare it using `component` keyword and derive it from the builtin `static_library` component:
```
component MyReusableStaticLibrary : static_library {
    // Some private dependencies that every library should link to.
    dependencies.private = [ "some", "private", "deps" ]
};
```

There are other differences which will be covered later.

# Paradigms integration
Technically, using declarative files is an alternative way of creating targets.

## add_declarative_file() and getting stuff from it
(Check out a complete example: [add_declarative_file](https://github.com/stryku/cmakesl/tree/master/examples/add_declarative_file))

If you call `add_declarative_file("some_file.dcmsl")` function, the function returns an object. The type of the object is unspecified. It depends on how the component was declared. If the type of the component derives from a `static_library`, then you get an object of some type that derives from a `cmake::library` type. If the type of the component derives from an `executable`, then you get an object of some type that derives from a `cmake::executable` type. You get the point. You can then pass this object to e.g. `cmake::executable::link_to()` method. For example, you have a declarative file `"foo.dcmsl"` that introduces a library:
```
static_library {
    name = "foo"
    files.public = [ "foo.cpp" ]
}
```
in your `CMakeLists.cmsl` you add this declarative file and link the executable with the returned library:
```cpp
int main()
{
    cmake::executable exe = ...;
    auto foo = add_declarative_file("foo.dcmsl");
    exe.link_to(foo);
}
```

Note that, the `cmake::executable::link_to()` method takes a `cmake::library` as a parameter. The `add_declarative_file("foo.dcmsl")` call returns an object of unspecified type. The type depends on the declarative file content. In our case, the object is convertible to `cmake::library`, so the right overload of `cmake::executable::link_to` is found and chosen.


## add_subdirectory() with CMakeLists.dcmsl
(Check out a complete example: [add_subdirectory_declarative](https://github.com/stryku/cmakesl/tree/master/examples/add_subdirectory_declarative))

It's a common case to have a directory that introduces a single target and does nothing more. If you don't need to do any magic there, a single declarative file is a good solution.

Let's imagine a simple project with one executable and one library. Root `CMakeLists` creates an executable and `adds_subdirectory` with the library that later on is linked to the executable. The library is introduced using declarative format.

An example folder structure:
```
├── CMakeLists.cmsl
├── main.cpp
└── some_lib
    ├── CMakeLists.dcmsl
    └── lib.cpp
```

`CMakeLists.cmsl`:
```cpp
int main()
{
    ...

    project p = project("Some Application");

    auto some_lib = add_subdirectory("some_lib");

    auto exe = p.add_executable("Application", {"main.cpp"} );
    exe.link_to(some_lib);

    ...
}
```

And the `some_lib/CMakeLists.dcmsl`:
```
static_library {
    name = "some_lib"
    files.public = [
        "lib.cpp"
    ]
}
```

That's it. No other CMakeLists has to be written to introduce the library.

## Declarative root file
(Check out a complete example: [declarative_root_cmakelists](https://github.com/stryku/cmakesl/tree/master/examples/declarative_root_cmakelists))

Declarative file can be used as the root CMakeLists. So, if you don't need to do anything fancy in the project, or just want to quickly check something out, that's the way.

# Brief introduction
## Builtin components
There are four builtin component types that you should be aware of:
* `static_library`
* `shared_library`
* `executable`
* `test_executable`

All of them derive from `product` component type. You can find available properties in the [docs](https://github.com/stryku/cmakesl/blob/master/doc/builtin/decl.cmsl#L31).

## forwarding_lists properties
You've probably spotted a usage of property like `files.public`. At this point you probably figured out what does it mean.

In CMake a lot of properties can be `PUBLIC`, `PRIVATE` or `INTERFACE`. You can add directories to an include path of a target, with `PRIVATE` keyword. That means they won't be forwarded.

The same functionality comes with properties of `product` component, that are of `forwarding_lists` type. `forwarding_lists` has three properties: 
* `public`
* `private`
* `interface`

and you can access them and assign to them like you saw earlier, e.g.:
```
static_library {
    name = "foo"

    files.public = [
        "foo.cpp"
    ]

    include_dirs.public = [
        "public/include/dir"
    ]

    inculde_dirs.private = [
        "private/include/dir"
    ]

    dependencies.interface = [
        "bar_dependency"
    ]
}
```

And so on..

## Custom components
(Check out a complete example: [custom_component](https://github.com/stryku/cmakesl/tree/master/examples/custom_component))

(Check out a real world example in CMakeSL itself: [test_utils](https://github.com/stryku/cmakesl/blob/master/cmake/test_utils.dcmsl), and usage e.g.: [test/decl_ast/CMakeLists.dcmsl](https://github.com/stryku/cmakesl/blob/master/test/decl_ast/CMakeLists.dcmsl))

There are of course cases, when you would want to create a custom component. E.g. you'd want to add a suffix to all of your libraries. You could, of course, add the suffix manually in every `static_library` declaration, but that's ugly. Instead, you can create a custom component that accumulates common functionality and properties. Later on it can be used to declare stuff. 

Let's say that we want all of our libraries to have suffix "_my_fancy_lib" and include directory "my/fancy/dir". Let's declare a custom component out of it, that derives from the builtin `static_library` component:
```
component my_fancy_lib : static_library {
    name_suffix = "_my_fancy_lib"

    include_dirs.public = [
        "my/fancy/dir"
    ]
}
```

And now it can be used to declare our fancy lib:
```
my_fancy_lib {
    name = "foo"

    include_dirs.public += [
        "another/fancy/dir"
    ]
}
```

Mind the `+=` while adding include directory. Thanks to that, the list with `another/fancy/dir` will be appended to the list in component declaration. If you'd use a plain `=`, the list would be overriden.

## Modules
(Check out a complete example: [module_import](https://github.com/stryku/cmakesl/tree/master/examples/module_import))

Imperative CMakeSL as well as the declarative one has modules support. With the example above, you most likely would want to declare `my_fancy_lib` component in a commonly accessible file and use it wherever you need.

Just create the file, let's name it `my_fancy_lib.dcmsl` and in import it the file in whchich you want to use it:
```
import "my_fancy_lib.dcmsl";

my_fancy_lib {
    name = "foo"

    include_dirs.public += [
        "another/fancy/dir"
    ]
}
```

## Accessing old-style CMake variables
(Check out a complete example: [cmake_variables_accessor](https://github.com/stryku/cmakesl/tree/master/examples/cmake_variables_accessor))

In a lot of cases you'll need to get a value of an old-style CMake variable. You can get it from an accessor named `cmake_variables`. CMakeSL is statically typed, so you need to provide information how the variable should be treated, using `as_bool`, `as_int`, `as_double`, `as_string` or `as_list`.

For example, let's say that you have a root CMakeLists written in the vanilla CMake:
`CMakeLists.txt`:
```
cmake_minimum_required(VERSION 3.14.3)

project("MyFancyProject")

set(MY_FANCY_SUFFIX "_my_fancy_suffix")

add_subdirectory(my_fancy_lib)
```

In the `my_fancy_lib` dir, you declare a library that gets its name suffix from the accessor:
`my_fancy_lib/CMakeLists.dcmsl`:
```
static_library {
    name = "my_fancy_lib"
    name_suffix = cmake_variables.MY_FANCY_SUFFIX.as_string

    ...
}
```

# Real world usage

CMakeSL using imperative and declarative format is able to build itself. Check out its files, e.g.:
* [root CMakeLists.cmsl](https://github.com/stryku/cmakesl/blob/master/CMakeLists.cmsl)
* [custom declarative test_executable component](https://github.com/stryku/cmakesl/blob/master/cmake/test_utils.dcmsl)
* [usage of importing and using custom test_executable component](https://github.com/stryku/cmakesl/blob/master/test/lexer/CMakeLists.dcmsl)
* [example binaries with installation](https://github.com/stryku/cmakesl/blob/master/examples/CMakeLists.cmsl)
* [Adding googletest subdirectory that contains 'old' CMakeLists.txt](https://github.com/stryku/cmakesl/blob/master/CMakeLists.cmsl#L48)

# Thanks for reading

As always, I'm curious what you think about all this.

* Leave a comment on [r/cpp](https://www.reddit.com/r/cpp/comments/f4ge4k/declarative_support_in_cmakesl/)
* Leave a comment on [r/cmake](https://www.reddit.com/r/cmake/comments/f4gekt/declarative_support_in_cmakesl/)
* Email me: stryku2393 aatt gmail.com
* Leave a comment on CMakeSL [issue on CMake's GitLab](https://gitlab.kitware.com/cmake/cmake/issues/19889)

(BTW, there is a discussion about introducing a declarative language in CMake, so if you like the idea and have something to say, check out [the issue](https://gitlab.kitware.com/cmake/cmake/issues/19891))



# Thanks for reading o/
 