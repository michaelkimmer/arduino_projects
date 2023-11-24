

import serial
import matplotlib.pyplot as plt
import numpy as np

Npoints = 1000
points = np.zeros((Npoints, 6))

with serial.Serial(port='COM7', baudrate=115200, timeout=1) as ser: #115200, 9600
    with open("serial_out.txt", "w") as f:
        #header
        for i in range(5):
            line = ser.readline().decode("utf-8")   # read a '\n' terminated line
            print(line)

        #data
        for i in range(Npoints):
            line = ser.readline().decode("utf-8")   # read a '\n' terminated line
            print(line)
            
            #file write
            f.write(line)

            #compute
            line_array_str = line.strip().split(" ")
            line_array = [float(i) for i in line_array_str]
            points[i,:] = np.array(line_array)


    ser.close()


plt.plot(points[:,0])
plt.plot(points[:,1])
plt.plot(points[:,2])
plt.show()