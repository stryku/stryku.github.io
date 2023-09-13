---
layout: post
title:  "Multidimensional Game Next"
date:   2023-09-13 22:35:07 +0200
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

This was an example from 3D.

## Validating points

We'd like to take any points specified by user and compute a plane out of it.
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

So, having our $v_{1} = (v_{1x}, v_{1y}, v_{1z})$ and $v_{2} = (v_{2x}, v_{2y}, v_{2z})$ and wondering whether they span $R^2$, we have to check whether the rank of the matrix is equal to 2

$$
\begin{align}


\newcommand{\MyMatrix}{
  \begin{bmatrix}
    v_{1x} & v_{1y} & v_{1z}\\
    v_{2x} & v_{2y} & v_{2z}
  \end{bmatrix}
}



r = rank \left( \MyMatrix \right)


\end{align}
$$


In order to check that, we create matrices

$$
\begin{align}

M_1 =
\begin{bmatrix}
v_{1x} & v_{1y}\\
v_{2x} & v_{2y}
\end{bmatrix}

,

M_2 =
\begin{bmatrix}
v_{1x} & v_{1z}\\
v_{2x} & v_{2z}
\end{bmatrix}

,

M_3 =
\begin{bmatrix}
v_{1y} & v_{1z}\\
v_{2y} & v_{2z}
\end{bmatrix}

\end{align}
$$

..calculate their determinants..


$$
\begin{align}

d_1 =
\begin{vmatrix}
v_{1x} & v_{1y}\\
v_{2x} & v_{2y}
\end{vmatrix}

,

d_2 =
\begin{vmatrix}
v_{1x} & v_{1z}\\
v_{2x} & v_{2z}
\end{vmatrix}

,

d_3 =
\begin{vmatrix}
v_{1y} & v_{1z}\\
v_{2y} & v_{2z}
\end{vmatrix}

\end{align}
$$

..and check if any of them is different than zero, then our matrix have rank 2, so we know it spans a 2D plane.

If all of them are zero, we can't go further with the points. We need to figure out a different set.

## Validating points in higher dimension

In higher dimension it works analogically. We have some 4D points and check if they span a 3-dimensional hyperplane.



$$
\begin{align}


p_1 = (p_{1x}, p_{1y}, p_{1z}, p_{1w}) \\
p_2 = (p_{2x}, p_{2y}, p_{2z}, p_{2w}) \\
p_3 = (p_{3x}, p_{3y}, p_{3z}, p_{3w}) \\
p_4 = (p_{4x}, p_{4y}, p_{4z}, p_{4w}) \\

\\

v_1 = p_2 - p_1 = (v_{1x}, v_{1y}, v_{1z}, v_{1w}) \\
v_2 = p_3 - p_1 = (v_{2x}, v_{2y}, v_{2z}, v_{2w}) \\
v_3 = p_4 - p_1 = (v_{3x}, v_{3y}, v_{3z}, v_{3w}) \\

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

