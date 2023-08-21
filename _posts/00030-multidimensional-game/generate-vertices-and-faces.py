#!/usr/bin/env python3

import math
import itertools


def vertex_distance(a, b):
    s = sum([abs(ai - bi)**2 for ai, bi in zip(a, b)])
    return math.sqrt(s)


def generate_vertices_and_faces(dimension):
    N = dimension

    # Generate all vertices
    vertices = list(itertools.product([0, 1], repeat=N))
    vertices = [list(e) for e in vertices]

    # Generate all vertices of one dimension lower
    vs = list(itertools.product([0, 1], repeat=(N-1)))
    # print(vs)

    # Generate faces. Face consists of all its vertices.
    faces = []

    for i in [0, 1]:
        for pos_to_insert in range(N):
            face = []
            for v in vs:
                vertex = list(v)
                vertex.insert(pos_to_insert, i)

                face.append(vertex)
            faces.append(face)

    # Discover edges of faces. Edge consists of indices of two vertices creating edge.
    edges_of_faces = []

    for face in faces:
        edges = set()
        for i in range(len(face)):
            for j in range(len(face)):
                if vertex_distance(face[i], face[j]) == 1:
                    index_i = vertices.index(face[i])
                    index_j = vertices.index(face[j])

                    # Sort in order to remove duplicates while adding to set().
                    edge = tuple(sorted((index_i, index_j)))

                    edges.add(edge)

        edges_of_faces.append(edges)

    return vertices, edges_of_faces


vertices, edges_of_faces = generate_vertices_and_faces(4)


print('Vertices:')
for i in range(len(vertices)):
    print(f'[{i}]: {vertices[i]}')

print('\nEdges of faces:')
for i in range(len(edges_of_faces)):
    face = edges_of_faces[i]
    print(f'Face {i}:')
    for v0, v1 in face:
        print(f'  {v0} <-> {v1}')
