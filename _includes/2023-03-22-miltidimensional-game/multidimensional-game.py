#!/usr/bin/env python3

"""
requirements.txt I used:
PyQt5==5.15.1
pyqtgraph
pyopengl
numpy
"""

import sys
import pyqtgraph.opengl as gl
import numpy as np

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *


app = QApplication(sys.argv)
window = QWidget()
window.setMinimumSize(QSize(400, 400))

layout = QGridLayout()
window.setLayout(layout)

w = gl.GLViewWidget()
w.opts['distance'] = 5

axis = gl.GLAxisItem()
axis.scale(2, 2, 2)
w.addItem(axis)

vertices = [
    [0, 0, 0],
    [0, 0, 1],
    [0, 1, 0],
    [0, 1, 1],
    [1, 0, 0],
    [1, 0, 1],
    [1, 1, 0],
    [1, 1, 1]
]

faces = [
    [0, 2, 4],
    [6, 2, 4],

    [0, 2, 1],
    [3, 2, 1],

    [4, 5, 6],
    [7, 5, 6],

    [1, 3, 5],
    [7, 3, 5],

    [2, 3, 6],
    [7, 3, 6],

    [0, 1, 4],
    [5, 1, 4]
]

# The 4th values of 1 are for alpha
colors = [
    [1, 0, 0, 1],
    [1, 0, 0, 1],

    [0, 1, 0, 1],
    [0, 1, 0, 1],

    [0, 0, 1, 1],
    [0, 0, 1, 1],

    [1, 1, 0, 1],
    [1, 1, 0, 1],

    [0, 1, 1, 1],
    [0, 1, 1, 1],

    [1, 0, 1, 1],
    [1, 0, 1, 1]
]

mesh_data = gl.MeshData(
    vertexes=vertices, faces=faces, faceColors=colors)

cube_item = gl.GLMeshItem(
    meshdata=mesh_data, drawEdges=True, edgeColor=(0, 0, 0, 1), smooth=False)

w.addItem(cube_item)

layout.addWidget(w)

window.show()
sys.exit(app.exec_())
