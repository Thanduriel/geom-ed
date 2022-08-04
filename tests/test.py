# comparison with a python implementation from https://github.com/flaport/fdtd

import fdtd
import numpy as np

def div(B):
	return B[1:, :-1, :-1, 0] - B[:-1, :-1, :-1, 0] + B[:-1, 1:, :-1, 1] - B[:-1, :-1, :-1, 1] + B[:-1, :-1, 1:, 2] - B[:-1, :-1, :-1, 2]

grid = fdtd.Grid(
    (32, 32, 32),
    1.0,
    1.0,
    1.0
)
grid[0, :, :] = fdtd.PeriodicBoundary(name="xbounds")
grid[:, 0, :] = fdtd.PeriodicBoundary(name="ybounds")
grid[:, :, 0] = fdtd.PeriodicBoundary(name="zbounds")
#grid[-1, :, :] = fdtd.PeriodicBoundary(name="xbounds1")
#grid[:, -1, :] = fdtd.PeriodicBoundary(name="ybounds1")
#grid[:, :, -1] = fdtd.PeriodicBoundary(name="zbounds1")

grid.E[16,16,16,0] = 1.0
print("time-step: {}".format(grid.time_step))
print(grid)
for i in range(0,128):
	print(np.sum(np.abs(div(grid.H))))
	grid.run(1, False)
	#print(grid.E[16,16,16,0])