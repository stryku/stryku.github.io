---
layout: post
title:  "Multidimensional Game Next"
date:   2023-04-26 22:35:07 +0200
categories: game
published: false
---



This is an inline equation: $$V_{sphere} = \frac{4}{3}\pi r^3$$,<br>
followed by a display style equation:

$$V_{sphere} = \frac{4}{3}\pi r^3$$


In the last post we described the approach of displaying 4D cube in 3D. It's about time to describe math behind it and code it. Steps to do are:
1. Define the plane that we'll intersect with our cube.
2. Find the points of intersecting the plane and the cube.
3. Build the 3D shape based on the points of intersection.

# Defining the plane

A plane can be defined in a couple of ways. I took the approach of selecting points and finding the plane equation that the points lay on. So, e.g. giving three points : `(1, 2, 3)`, `(-1, -1, 3)` and `(3, 2, 1)`, we want to find the equation of the 2D plane: `3x - 2y + 3z - 8 = 0`.

This was an example from 3D. From now on we'll work in 4D. 

## Validating points

We can be given any points. There might be a situation where the points don't define exactly one hyperplane (plane in usually more than 2D). E.g. this points could NOT be used to describe a 3D hyperplane:
```
(1, 2, 3, 4)
(2, 4, 6, 8)
(1, 1, 1, 1)
```




# Thanks for reading o/

