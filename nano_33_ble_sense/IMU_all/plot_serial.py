import matplotlib.pyplot as plt
import numpy as np

points = []

with open("serial_out_0.txt", "r") as f:
    for line in f:
        line_array_str = line.strip().split(" ")
        line_array = [float(i) for i in line_array_str] #6 values
        points.append(line_array)

points_np = np.array(points)


plt.plot(points_np[:,0])
plt.plot(points_np[:,1])
plt.plot(points_np[:,2])

plt.show()