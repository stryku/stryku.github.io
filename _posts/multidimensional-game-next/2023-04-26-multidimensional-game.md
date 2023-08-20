---
layout: post
title:  "Multidimensional Game Next"
date:   2023-04-26 22:35:07 +0200
categories: game
published: true
---





In the last post we described the approach of displaying 4D cube in 3D.

* Q: It's not cool that you use 3D, 4D, ... You should use $\mathbb{R^3}$, $\mathbb{R^4}, ...$!
* A: Yes, I know. But I'll use 3D in the text and $\mathbb{R^3}$ in math formulas. It's easier to write posts like this. Sorry. I regret nothing.

It's about time to describe math behind it and code it. Steps to do are:
1. Define the plane that we'll intersect with our cube.
2. Find the points of intersecting the plane and the cube.
3. Build the 3D shape based on the points of intersection.

# Defining the plane

A plane can be defined in a couple of ways. I took the approach of selecting points and finding the plane equation that the points lay on. So, e.g. giving three points : `(1, 2, 3)`, `(-1, -1, 3)` and `(3, 2, 1)`, we want to find the equation of the 2D plane: `3x - 2y + 3z - 8 = 0`.

This was an example from 3D. From now on we'll work in 4D. 

## Validating points

We'd like to take any points specified by user and compute A plane out of it.
The first step is to calculate vectors spanning our plane based on these points. Let's assume we have points in 3D:
```
p1, p2, p3
```
now, we calculate vectors that _should_ span our plane:
```
v1 = p2 - p1
v2 = p3 - p1
```

So, yea, they _should_ span a 2D plane. But, there are cases where they won't. To ensure the vectors properly span a 2D plane, we need to ensure they are linearly independent. They are linearly independent when:

$$
\begin{align}
v_{1}, v_{2}, ..., v_{n} \in V \\

\alpha_{1}*v_{1} + \alpha_{2}*v_{2} + ... + \alpha_{n}*v_{n} = \vec{0} \iff \alpha_{1} = \alpha_{2} = ... = \alpha_{n} = 0

\end{align}
$$

meaning that there's no way to describe a $v_{i}$ as a linear combination of other vectors.

Having vectors and wondering whether they span $R^N$ we can create an NxN matrix out of them and calculate its order.

So, having our $v_{1} = (v_{1x}, v_{1y}, v_{1z})$ and $v_{2} = (v_{2x}, v_{2y}, v_{2z})$ and wondering whether they span $R^2$, we create matrices

$$
\begin{align}

\begin{equation}\label{eq:example_matrix}
    \mathbf{A} = \left[ \begin{array}{cc}
        1 & 2 \\
        3 & 4 \\
    \end{array} \right]
\end{equation}



\end{align}
$$



# TODOOOOO

But, it can turn out that it's impossible to create a plane out of these points. E.g. these 3D points that we'd like to describe a plane won't do good.
```
p1: (1, 1, 1)
p2: (2, 3, 4)
p3: (3, 5, 7)
```

We can be given any points. There might be a situation where the points don't define exactly one hyperplane (plane in usually more than 2D). E.g. this points could NOT be used to describe a 2D hyperplane:
```
p1: (1, 2, 3, 4)
p2: (2, 4, 6, 8)
p3: (1, 1, 1, 1)
```
because `p1` and `p2` are _linear



# TODO REMOVE

This is an inline equation: $V_{sphere} = \frac{4}{3}\pi r^3$,
followed by a display style equation:

$$
\begin{align}
V_{sphere} = \frac{4}{3}\pi r^3
\end{align}
$$


# Thanks for reading o/

