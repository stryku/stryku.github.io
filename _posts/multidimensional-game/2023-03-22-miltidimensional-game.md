---
layout: post
title:  "Multidimensional Game"
date:   2023-03-22 20:35:17 +0100
categories: game
published: true
this_includes: 2023-03-22-miltidimensional-game
---

Some time ago I thought that it would be cool to one day be able to say: "I created a 4D game". So, I decided to create one. During the process of thinking I figured out that 4+D game actually sounds even cooler. And here we are.

This is my first and hopefully not last post about me creating a multidimensional game. There will most likely be a lot of math, rendering pipelines, bugs, weird objects that I don't know if they look correct because we're unable to judge that because we don't think in more than 3D and a lot of fun after actually creating the game.

I want to blog about it because I already see that I forget how math and stuff work. I'm writing this to be able to come back and see. Maybe if will become useful for someone in the future.

In this very first post I'll explain my journey to generating hypercubes (cubes in usually more than three dimensions) in a specific way, which allows me to present it nicely in 3D.

# I don't know all this

Before we start I want to say that I don't know this stuff. I'm learning and I will make mistakes. If I make one and you care enough please ping me on email or something. Always keen to learn more.

# Baby steps

So, like I said, I don't know much about making games and multidimensional math. Before committing to making such a game, I wanted to see if I can wrap my mind around math and present something that makes sense. Wanted to generate a hypercube of any (3+) dimensions, be able to move and rotate it and present it using 3D tools. I didn't want to use full-blown gaming engine and ready made libraries for multidimensional math. The first goal was to create as much from scratch as possible. So, decided to go with Python, pygraph and a small help with scipy.

Python because it's Python and it just works. Pygraph because it can display mesh with coloured faces and that's exactly what I wanted. Scipy because it can triangulate and that's exactly what I don't want to learn at this point - if I write mesh and coloring faces from scratch, I can live with scipy triangulating stuff for me.

# Presenting 3D cube

Presenting a 3D cube is quite trivial these days. There are two ways that I considered - presenting a cube mesh using one color with shadows, or coloring every of the cube faces with its own color. It turned out that pygraph doesn't support shadows, so I went the colorful way. Which complicates stuff quite a lot with 4+D as I learned :)

## 3D cube mesh

We'll focus on cubes with edges of length 1, originated in the point `0, 0, 0`. Vertices of a 3D cube are quite simple. You just take all the permutations of zeros and ones of length 3, so:
```
x, y, z
-------
0, 0, 0
0, 0, 1
0, 1, 0
0, 1, 1
1, 0, 0
1, 0, 1
1, 1, 0
1, 1, 1
```

What's not so trivial is logically creating faces of the cube mesh - the triangles that are going to be presented by OpenGL on the screen. It's not trivial, but in 3D it's still not much work to do it manually, which we're going to do now just to explicitly show what the code needs to do.

Let's keep all the vertices in an array:
```
    x, y, z
    -------
[0] 0, 0, 0
[1] 0, 0, 1
[2] 0, 1, 0
[3] 0, 1, 1
[4] 1, 0, 0
[5] 1, 0, 1
[6] 1, 1, 0
[7] 1, 1, 1
```

So, the cube looks like this:
TODO IMAGE CUBE EDGES

Every face of the cube consists of two triangles. The triangles are represented by indices of three vertices. E.g. the front face can be represented by these two:
```
face 0.0: 0, 2, 4
face 0.1: 6, 2, 4
```

TODO IMAGE FRONT FACE

Doing the same with other faces we end up with:
```
face 0.0: 0, 2, 4
face 0.1: 6, 2, 4

face 1.0: 0, 2, 1
face 1.1: 3, 2, 1

face 2.0: 4, 5, 6
face 2.1: 7, 5, 6

face 3.0: 1, 3, 5
face 3.1: 7, 3, 5

face 4.0: 2, 3, 6
face 4.1: 7, 3, 6

face 5.0: 0, 1, 4
face 5.1: 5, 1, 4
```

Now, in order to nicely display the cube we need to assign colors to faces. One color for two triangles of given face:

```
face 0.0: 0, 2, 4 - red
face 0.1: 6, 2, 4 - red

face 1.0: 0, 2, 1 - green
face 1.1: 3, 2, 1 - green

face 2.0: 4, 5, 6 - blue
face 2.1: 7, 5, 6 - blue

face 3.0: 1, 3, 5 - yellow
face 3.1: 7, 3, 5 - yellow

face 4.0: 2, 3, 6 - cyan
face 4.1: 7, 3, 6 - cyan

face 5.0: 0, 1, 4 - magenta
face 5.1: 5, 1, 4 - magenta
```


Having all this information we finally can create mesh data, color it and display. Full listing of a simple Qt window displaying such a cube:
```py
{% include_relative multidimensional-game.py %}
```

We get this:

![cool image](/assets/miltidimensional-game/cube3d.gif)

a 

# Presenting 4D cube


