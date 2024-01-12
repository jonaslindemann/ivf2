#!/bin/env python
#-*- encoding=utf8 -*-

import sys
from typing import Any
import numpy as np
import matplotlib.pyplot as plt

class Polyline:
    def __init__(self):
        self.points = []

    def add_point(self, x, y):
        self.points.append((x, y))

    def __call__(self, t):
        if t < 0 or t > 1:
            raise ValueError('t must be in [0, 1]')
        if len(self.points) < 2:
            raise ValueError('points must be more than 2')
        if len(self.points) == 2:
            return self.points[0][0] + (self.points[1][0] - self.points[0][0]) * t, \
                   self.points[0][1] + (self.points[1][1] - self.points[0][1]) * t
        else:
            return self._de_casteljau(t)

if __name__ == '__main__':

    pl = Polyline()
    pl.add_point(0, 0)
    pl.add_point(1, 1)
    pl.add_point(2, 1)
    pl.add_point(3, 0)
    pl.add_point(4, 0)

    print(pl(1.0))

    