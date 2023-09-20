---
layout: post
title:  "Multidimensional Game Next"
date:   2023-09-13 22:35:07 +0200
categories: multidimensional-game
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
\begin{align*}
  v_{1}, v_{2}, ..., v_{n} \in V \\

  \alpha_{1}*v_{1} + \alpha_{2}*v_{2} + ... + \alpha_{n}*v_{n} = \vec{0} \iff \alpha_{1} = \alpha_{2} = ... = \alpha_{n} = 0

\end{align*}
$$

meaning that there's no way to describe a $v_{i}$ as a linear combination of other vectors.

Having vectors and wondering whether they span $R^N$ we can create an NxN matrix out of them and calculate its order.

So, having our $v_{1} = (v_{1x}, v_{1y}, v_{1z})$ and $v_{2} = (v_{2x}, v_{2y}, v_{2z})$ and wondering whether they span $R^2$, we have to check whether the rank of the matrix is equal to 2

$$
\begin{align*}

  \newcommand{\MyMatrix}{
    \begin{bmatrix}
      v_{1x} & v_{1y} & v_{1z}\\
      v_{2x} & v_{2y} & v_{2z}
    \end{bmatrix}
  }

  r = rank \left( \MyMatrix \right)

\end{align*}
$$


In order to check that, we create matrices

$$
\begin{align*}

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

\end{align*}
$$

..calculate their determinants..


$$
\begin{align*}

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

\end{align*}
$$

..and check if any of them is different than zero, then our matrix have rank 2, so we know it spans a 2D plane.

If all of them are zero, we can't go further with the points. We need to figure out a different set.

## Validating points in higher dimension

In higher dimension it works analogically. We have some 4D points and check if they span a 3-dimensional hyperplane.



$$
\begin{align*}


  p_1 = (p_{1x}, p_{1y}, p_{1z}, p_{1w}) \\
  p_2 = (p_{2x}, p_{2y}, p_{2z}, p_{2w}) \\
  p_3 = (p_{3x}, p_{3y}, p_{3z}, p_{3w}) \\
  p_4 = (p_{4x}, p_{4y}, p_{4z}, p_{4w}) \\

  \\

  v_1 = p_2 - p_1 = (v_{1x}, v_{1y}, v_{1z}, v_{1w}) \\
  v_2 = p_3 - p_1 = (v_{2x}, v_{2y}, v_{2z}, v_{2w}) \\
  v_3 = p_4 - p_1 = (v_{3x}, v_{3y}, v_{3z}, v_{3w}) \\

  \\

  M_1 =
    \begin{bmatrix}
      v_{1x} & v_{1y} & v_{1z}\\
      v_{2x} & v_{2y} & v_{2z}\\
      v_{3x} & v_{3y} & v_{3z}
    \end{bmatrix}

  ,

  M_2 =
    \begin{bmatrix}
      v_{1x} & v_{1y} & v_{1w}\\
      v_{2x} & v_{2y} & v_{2w}\\
      v_{3x} & v_{3y} & v_{3w}
    \end{bmatrix}

  ,

  M_3 =
    \begin{bmatrix}
      v_{1x} & v_{1z} & v_{1w}\\
      v_{2x} & v_{2z} & v_{2w}\\
      v_{3x} & v_{3z} & v_{3w}
    \end{bmatrix}

  ,

  M_4 =
    \begin{bmatrix}
      v_{1y} & v_{1z} & v_{1w}\\
      v_{2y} & v_{2z} & v_{2w}\\
      v_{3y} & v_{3z} & v_{3w}
    \end{bmatrix}

  \\
  \\

  d_1 =
    \begin{vmatrix}
      v_{1x} & v_{1y} & v_{1z}\\
      v_{2x} & v_{2y} & v_{2z}\\
      v_{3x} & v_{3y} & v_{3z}
    \end{vmatrix}

  ,

  d_2 =
    \begin{vmatrix}
      v_{1x} & v_{1y} & v_{1w}\\
      v_{2x} & v_{2y} & v_{2w}\\
      v_{3x} & v_{3y} & v_{3w}
    \end{vmatrix}

  ,

  d_3 =
    \begin{vmatrix}
      v_{1x} & v_{1z} & v_{1w}\\
      v_{2x} & v_{2z} & v_{2w}\\
      v_{3x} & v_{3z} & v_{3w}
    \end{vmatrix}

  ,

  d_4 =
    \begin{vmatrix}
      v_{1y} & v_{1z} & v_{1w}\\
      v_{2y} & v_{2z} & v_{2w}\\
      v_{3y} & v_{3z} & v_{3w}
    \end{vmatrix}

\end{align*}
$$


## Finding plane equation

At this point we know that our points really do span a 3-dimensional hyperplane. We need to find its equation.

### Checking simplest case

Firs of, we can check one thing - if any of the terms is constant in all the specified points, the plane equation is trivial. Consider e.g. points like this:


$$
\begin{align*}

  p_1 = (2,8,44,12) \\
  p_2 = (2,3,2,1) \\
  p_3 = (2,-2,2,-2) \\
  p_4 = (2,0,0,0) \\

\end{align*}
$$

then all the terms but $x$ aren't important. They don't matter. So, the plane equation is

$$
\begin{align*}
x = 2
\end{align*}
$$


And that's it. This is the whole 3-dimensional plane equation. It's not the most interesting of the hyperplanes, but it is a hyperplane, for sure.

If none of the terms is constant, we need to actually calculate the plane equation.

### Finding axis

If the above is not the case, we go axis by axis until we find one that doesn't lay on our hyperplane. In order to do this, we check every of the vectors $(1,0,0,0), (0,1,0,0), (0,0,1,0)$ and $(0,0,0,1)$ whether it lays outside of our plane, in other words

$$
\begin{align*}
  (1,0,0,0) \notin Lin(\{v_1, v_2, v_3\}) \\
  ... \\
  (0,0,0,1) \notin Lin(\{v_1, v_2, v_3\}) \\
\end{align*}
$$

E.g. if $(1,0,0,0) \notin Lin(\{v_1, v_2, v_3\})$, then the plane equation is $x = \alpha y + \beta z + \gamma w + \delta$. If $(0,0,1,0) \notin Lin(\{v_1, v_2, v_3\})$ then $z = \alpha x + \beta y + \gamma w + \delta$, and so on.

We can check that by calculating determinant $d$ of such matrices and checking if it's different than zero.

$$
\begin{align*}

  d_x =
    \begin{vmatrix}
      v_{1x} & v_{1y} & v_{1z} & v_{1w}\\
      v_{2x} & v_{2y} & v_{2z} & v_{2w}\\
      v_{3x} & v_{3y} & v_{3z} & v_{3w}\\
      1 & 0 & 0 & 0
    \end{vmatrix}

  ,

  d_y =
    \begin{vmatrix}
      v_{1x} & v_{1y} & v_{1z} & v_{1w}\\
      v_{2x} & v_{2y} & v_{2z} & v_{2w}\\
      v_{3x} & v_{3y} & v_{3z} & v_{3w}\\
      0 & 1 & 0 & 0
    \end{vmatrix}

  \\\\

  d_z =
    \begin{vmatrix}
      v_{1x} & v_{1y} & v_{1z} & v_{1w}\\
      v_{2x} & v_{2y} & v_{2z} & v_{2w}\\
      v_{3x} & v_{3y} & v_{3z} & v_{3w}\\
      0 & 0 & 1 & 0
    \end{vmatrix}

  ,

  d_w =
    \begin{vmatrix}
      v_{1x} & v_{1y} & v_{1z} & v_{1w}\\
      v_{2x} & v_{2y} & v_{2z} & v_{2w}\\
      v_{3x} & v_{3y} & v_{3z} & v_{3w}\\
      0 & 0 & 0 & 1
    \end{vmatrix}

\end{align*}
$$

Once we find a non-zero $d$, we know the equation of the hyperplane. We also have four points that belong to it, so we can solve a system of four variables to find out the constants.

Let's assume we found out that the X axis doesn't lay on the hyperplane, therefore the equation of the hyperplane is $x = \alpha y + \beta z + \gamma w + \delta$. So, with our points: 

$$


\begin{align*}

  p_1 = (p_{1x}, p_{1y}, p_{1z}, p_{1w}) \\
  p_2 = (p_{2x}, p_{2y}, p_{2z}, p_{2w}) \\
  p_3 = (p_{3x}, p_{3y}, p_{3z}, p_{3w}) \\
  p_4 = (p_{4x}, p_{4y}, p_{4z}, p_{4w}) \\

\end{align*}
$$

our system of equations is:

$$
\begin{align*}

  \begin{cases} 

    p_{1x} = \alpha p_{1y} + \beta p_{1z} + \gamma p_{1w} + \delta \\
    p_{2x} = \alpha p_{2y} + \beta p_{2z} + \gamma p_{2w} + \delta \\
    p_{3x} = \alpha p_{3y} + \beta p_{3z} + \gamma p_{3w} + \delta \\
    p_{4x} = \alpha p_{4y} + \beta p_{4z} + \gamma p_{4w} + \delta \\

  \end{cases} 

\end{align*}

$$

and we can solve it e.g. using matrices:

$$
\begin{align*}

  \begin{bmatrix}
    p_{1x} \\
    p_{2x} \\
    p_{3x} \\
    p_{4x} \\
  \end{bmatrix}

  =

  \begin{bmatrix}
    p_{1y} & p_{1z} & p_{1w} & 1 \\
    p_{2y} & p_{2z} & p_{2w} & 1 \\
    p_{3y} & p_{3z} & p_{3w} & 1 \\
    p_{4y} & p_{4z} & p_{4w} & 1 \\
  \end{bmatrix}

  *

  \begin{bmatrix}
    \alpha \\
    \beta  \\
    \gamma \\
    \delta \\
  \end{bmatrix}

  \\\\\\

  \begin{bmatrix}
    \alpha \\
    \beta  \\
    \gamma \\
    \delta \\
  \end{bmatrix}

  =

  \begin{bmatrix}
    p_{1y} & p_{1z} & p_{1w} & 1 \\
    p_{2y} & p_{2z} & p_{2w} & 1 \\
    p_{3y} & p_{3z} & p_{3w} & 1 \\
    p_{4y} & p_{4z} & p_{4w} & 1 \\
  \end{bmatrix}^{-1}

  *

  \begin{bmatrix}
    p_{1x} \\
    p_{2x} \\
    p_{3x} \\
    p_{4x} \\
  \end{bmatrix}

\end{align*}
$$


And after all this we have the equation of our hyperplane.

# Find the points of intersecting the hyperplane and the cube

## Edge and hyperplane intersection

In order to get all the points where our cube intersects a plane, we can grab all the edges of our cube and calculate their intersection points with the hyperplane. The result points will create a convex 3D shape. We could triangulate it and present. But, if we would really understand what we see we could present the shape a bit better.

## Edges of faces

As I mentioned in [on of my earlier posts](/multidimensional-game/2023/04/19/multidimensional-game.html), what we will do is we're gonna assign a color to every face of 4D cube and maintain the colors in the result 3D shape.

In order to do that, it's best to not just grab all the edges of the cube, but consider the edges face by face, with the colors in mind. Thanks to that we'll be able to assign colors to edges and hyperplane intersection points and have a colorful 3D object in the end.

In the same post I described how to generate the edges of faces of a simple 4D cube. We will continue from that.


## Intersection of hyperplane and segment 

(The idea described here was inspired by section 5 of the `On the Hyperbox – Hyperplane Intersection Problem` paper)

Edges of our cube are just segments specified by two vertices of the edge. Let's call the edge $e$ and its vertices $e_0 = (e_{01}, e_{02}, e_{03}, e_{04})$ and $e_1 = (e_{11}, e_{12}, e_{13}, e_{14})$. We can write parametric representation of our edge

$$
\begin{align*}

  e(t) = e_0 + t(e_1 - e_0) \\
  where \\
  t \in [0,1] \\

\end{align*}
$$

If the edge intersects with the plane, we should be able to find the intersection point by solving for $t$ and using the plane equation.

$$
\begin{align*}

  t' = \frac{ \alpha - \sum_{i=1}^{4} e_{0i} } 
            { \sum_{i=1}^{4} { ( e_{1i} - e_{0i} ) } }


\end{align*}
$$

There are three possible cases:
- $t' \in [0,1]$ then we have exactly one point of intersection.
- $t' \notin [0,1]$, then there is no intersection of the edge and hyperplane. The line on which the edge lies DO intersect the plane, but the intersection point is outside of the edge.
- It's impossible to calculate $t'$ because $\sum_{i=1}^{4} { ( e_{1i} - e_{0i} ) }$ is equal to zero. In this case, the edge is parallel to the hyperplane, so there are two sub-cases:
  - The edge lies exactly on the hyperplane. There are infinite number of intersection points.
  - The edge doesn't lie on the hyperplane - no intersection points.

We can check the last case by simply checking whether one of the vertices solves the plane equation. If it does, we have infinite number of points, otherwise we have zero.
If we have infinite number of points, we just consider the two vertices of the edge as intersection points. It is enough for drawing the result 3D shape.





# TODO0000000000000000000000000000

- headers size


# Thanks for reading o/

