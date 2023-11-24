import math
import numpy as np

#generate samples of one period of sin

Nsamples = 7
phase = Nsamples/4 #in terms of samples
points = np.linspace(0, Nsamples-1, Nsamples)

sin_samples = np.sin(2*np.pi/Nsamples * (points - phase))


#scallling
low_limit, high_limit = 0, 7
sin_scaled_first = 0.5*(sin_samples+1)
sin_scaled = (high_limit-low_limit)* sin_scaled_first + low_limit

#round
sin_rounded = np.round(sin_scaled)
sin_rounded = sin_rounded.astype(np.int64)

#print
print(points)
print(sin_samples)
print(sin_scaled)
print(repr(sin_rounded))