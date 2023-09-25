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

# Table of Contents:
1. This will become a table of contents.
{:toc}

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


And after all this we have the equation of our hyperplane. Python looks like this:

```py
def calc_plane_from_points(points):
  """
  Returns list of scalars S1, S2, S3 ... Sn of plane equation S1x + S2y + S3z + ... + Sn = 0
  Or None if it's impossible to calculate the plane equation
  """

  N = len(points)

  if N < 3:
    raise RuntimeError(f'Dimension should be >=3. It is {N}')

  for p in points:
    if len(p) != N:
      raise RuntimeError(f'Point {p} is not a {N}d point')

  # Calc vectors on the plane
  vectors = []
  last_point = points[-1]
  for i in range(N-1):
    pi = points[i]
    v = sub_vnvn(pi, last_point)
    vectors.append(v)

  # Ensure the points span a proper plane
  any_det_non_zero = False

  for indices in itertools.combinations(range(N), N-1):
    m = []
    for p in points[:-1]:
      m.append([p[i] for i in indices])

    if np.linalg.det(m) != 0:
      any_det_non_zero = True
      break

  if not any_det_non_zero:
      print('Not able to figure out one result plane')
      return None

  # Check the simplest case when one term is constant
  for i in range(N):
    val = points[0][i]
    all_equal = True
    for p in points:
      if p[i] != val:
        all_equal = False
        break

    if all_equal:
      # Got one constant
      result_plane = [0]*(N+1)
      result_plane[i] = 1
      result_plane[-1] = -points[0][i]
      return result_plane

  # Find the axis not laying on the plane
  non_zero_i = None

  for i in range(len(p1)):
    m = points[:-1]
    m.append([0]*N)
    m[-1][i] = 1

    if np.linalg.det(m) != 0:
      non_zero_i = i
      break

  if non_zero_i is None:
    raise RuntimeError('Could not find axis not laying on the plane')

  # Solve the system of equations
  m = [[]*(N-1)]*N
  for i in range(N):
    for row in range(N):
      if i == row:
        continue
      m[row].append(points[i])
  for row in range(N):
    m[row].append(1)

  m = np.linalg.inv(m)
  p = [p[non_zero_i] for p in points]
  result = np.matmul(np.array(m), np.array(p)).tolist()
  result = [-r for r in result]
  result.insert(non_zero_i, 1)

  return result
```

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

Now, if we have an intersection point $p'$, we calculate it using the parametric equation:

$$
\begin{align*}

  p' = e_0 + t'(e_1 - e_0)

\end{align*}
$$

### Python

Python code implementing this calculations looks like this. It's pretty self-explanatory, so we won't dive into it.

```py
def isect_segment_plane(segment_v, segment_u, plane, epsilon=1e-6):
  """
  segment_v, segment_u - two points describing segment
  plane = C1, C2, C3 ... CN, scalars describing plane of equation C1v1 + C2v2 + ... + C(n-1)v(n-1) + Cn = 0
  Returns 
    - None if there's no intersection
    - Intersection point if there's one intersection point
    - string 'Inf' if there's infinite number of intersections points (segment lies on the plane).
  """
  v = segment_v
  u = segment_u
  u_sub_v = sub_vnvn(u, v)
  Cn = plane[-1]
  alpha = -Cn

  if len(v) != len(u):
    raise Exception(f'len(segment_v) {len(v)} != {len(u)} len(segment_u)')

  sum_v = sum([plane[i]*v[i] for i in range(len(v))])
  sum_u_sub_v = sum(plane[i]*u_sub_v[i] for i in range(len(u_sub_v)))

  if abs(sum_u_sub_v) < epsilon:
    # Vector is parallel. It might lay on the plane. Check one point if it lays on the plane
    a = sum([plane[i]*v[i] for i in range(len(v))])
    a += plane[-1]
    if abs(a) < epsilon:
      # Equation worked, lays on the plane
      return 'Inf'
    else:
      return None

  t = (alpha - sum_v)/sum_u_sub_v
  if t < -epsilon or t > 1.0 + epsilon:
    # t not in range <0, 1>. No intersection
    return None

  if t > 1.0:
    t = 1.0
  if t < 0.0:
    t = 0.0

  t_u_sub_v = mul_vn_scalar(u_sub_v, t)
  isect_point = add_vnvn(v, t_u_sub_v)
  return isect_point

```


## Intersection points of the whole cube

As an output of the [previous post](/multidimensional-game/2023/04/19/multidimensional-game.html) we have created a set of vertices and edges of faces of our cube

```py
vertices, edges_of_faces = generate_vertices_and_faces(4)
```

Vertices is just a list and look like this:
```
[0]: 0, 0, 0, 0
[1]: 0, 0, 0, 1
[2]: 0, 0, 1, 0
[3]: 0, 0, 1, 1
[4]: 0, 1, 0, 0
[5]: 0, 1, 0, 1
[6]: 0, 1, 1, 0
[7]: 0, 1, 1, 1
[8]: 1, 0, 0, 0
[9]: 1, 0, 0, 1
[10]: 1, 0, 1, 0
[11]: 1, 0, 1, 1
[12]: 1, 1, 0, 0
[13]: 1, 1, 0, 1
[14]: 1, 1, 1, 0
[15]: 1, 1, 1, 1
```

Edges of faces use indices of the vertices and look like this:
```py
[
  {
    (0, 1), (0, 4), (1, 5), (3, 7), (4, 6), (5, 7), (2, 3), (6, 7), (0, 2), (4, 5), (2, 6), (1, 3)
  }, 
  {
    (0, 1), (10, 11), (2, 10), (8, 10), (2, 3), (0, 2), (8, 9), (9, 11), (0, 8), (1, 3), (1, 9), (3, 11)
  }, 
  {
    (0, 1), (0, 4), (9, 13), (1, 5), (4, 12), (12, 13), (5, 13), (4, 5), (8, 9), (8, 12), (0, 8), (1, 9)
  }, 
  {
    (0, 4), (10, 14), (2, 10), (4, 6), (4, 12), (6, 14), (8, 10), (0, 2), (2, 6), (8, 12), (0, 8), (12, 14)
  }, 
  {
    (10, 11), (9, 13), (10, 14), (8, 10), (12, 13), (8, 9), (11, 15), (8, 12), (9, 11), (13, 15), (14, 15), (12, 14)
  }, 
  {
    (4, 6), (4, 12), (6, 14), (5, 7), (5, 13), (12, 13), (6, 7), (4, 5), (7, 15), (13, 15), (14, 15), (12, 14)
  }, 
  {
    (10, 11), (10, 14), (3, 7), (2, 10), (6, 14), (2, 3), (6, 7), (2, 6), (11, 15), (7, 15), (14, 15), (3, 11)
  }, 
  {
    (9, 13), (1, 5), (3, 7), (5, 7), (5, 13), (11, 15), (7, 15), (9, 11), (13, 15), (1, 3), (1, 9), (3, 11)
  }
]
```

So, we have all he pieces to calculate the intersection points of the whole cube. We iterate face by face and edge by edge, and calculate the intersection points of the edge and the hyperplane:


```py
def plane_segments_intersecting_points(plane, segments):
  """
  Returns a list of intersection points of plane and given segments
  plane = C1, C2, C3 ... CN, scalars describing plane of equation C1v1 + C2v2 + ... + C(n-1)v(n-1) + Cn = 0
  segments: list of segments: [[a1, b1], [a2, b2], ...]
  """

  isec_points = []

  for s in segments:
    v = isect_segment_plane(s[0], s[1], plane)
    if v is None:
      # No intersection points
      continue
    elif v == 'Inf':
      # Vertices are the intersection points
      isec_points.append(tuple(s[0]))
      isec_points.append(tuple(s[1]))
    else:
      # Just one intersection point
      isec_points.append(tuple(v))

  # Remove duplicates
  isec_points = list(set(isec_points))

  return isec_points

face_isec_points = []
for face_edges in edges_of_faces:
  isec_points = plane_segments_intersecting_points(plane, face_edges)
  face_isec_points.append(isec_points)
```

And that's it. We have the whole cube intersection points, grouped by face. We can start building our 3D shape for drawing.

# Building the 3D shape

2D cube (square) has four faces (edges). 3D cube has six faces (squares). 4D hypercube has eight faces (cubes).

At this point we are able to calculate intersection points of these eight faces. What we're gonna do now is to take these points and create a 3D shape out of them.

All the intersection points of one face lay on some 3D plane and they create a convex polygon on this plane. 

If you take all the convex polygons from all faces and render all of them, they will create a convex 3D polygon. And that's exactly what we're doing.

The algorithm is pretty simple
```
for face_points in face_isec_points
  triangles = triangulate(face_points)
  display_triangles_using_some_color(triangles)
```

Two things. One, we won't implement triangulation today. I used the ready-made `scipy.spatial.Delaunay`.

Two, we haven't talked about colors really. 

# Colors

All in all, we run a very simple 3D visualization program here. There is no lightning, so if we'd use a single color for the whole 3D object, it'd end up pretty flat. Instead, we're gonna use multiple colors, one for each face.

The important thing is that we're gonna assign distinct colors not to the 3D faces. We have to assign them at the very beginning, to the faces of 4D shape. Thanks to that we will be able to match the presented faces of the 3D object, with the faces of the 4D hypercube.

Assigning colors to the faces of the 4D object means we're gonna need to carry this information through all the math calculations we just talked about. This complicates the code, but just a little bit.



TODO face_isec_points with colors






# TODO0000000000000000000000000000

- headers size
- Move every math stuff to latex


# Thanks for reading o/

