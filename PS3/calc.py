import math
a = [1.0, 2.0, 5.0, 8.0, 6.0]
b = [0.5, 0.2, 0.8, 0.5, 0.5]
RTT = [10.0/1000.0, 100.0/1000.0, 300.0/1000.0, 1000.0/1000.0, 100.0/1000.0]
p = [math.pow(10.0, -6.0), math.pow(10.0, -8.0), math.pow(10.0, -
                                                          9.0), math.pow(10.0, -4.0), math.pow(10.0, -10.0)]


for i in range(len(a)):
    num = math.sqrt(2.0-b[i])*math.sqrt(a[i])
    denom = math.sqrt(2.0*b[i])*RTT[i]*math.sqrt(p[i])
    res = num/denom
    print("i: ", i)
    print("res: ", res)
