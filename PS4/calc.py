import math
import numpy as np


def hash1(key: int) -> int:
    x = (key+5)*(key-3)
    x = int(x/7)+key
    x = x % 13
    return x


key_arr = np.array([17, 22, 73, 56, 310, 100, 230, 12, 42, 18, 19, 24, 49])

i = 0
for key in key_arr:
    x = hash1(key)
    print("i: {} key: {} x: {} ".format(i, key, x))
    i = i+1


m = 100.0
n = 10.0
k = 5.0

p = math.pow(1.0-math.pow((1.0-1.0/m), k*n), k)
print("p: ", p)

F = 20.0*1024.0
us = 30.0
d = 2.0

N = 100.0
u = 2.0
print("\n")
print("Client Server")
val1 = N*F/us
val2 = F/d

print("val1: ", val1)
print("val2: ", val2)

print("\n")
print("P2P")

val1 = F/us
val2 = F/d
val3 = (N*F)/(us+N*u)

print("val1: ", val1)
print("val2: ", val2)
print("val3: ", val3)
