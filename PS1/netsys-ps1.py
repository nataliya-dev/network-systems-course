import math as m

# Qn 3
print("\nQUESTION 3")

convert_rate = 65.0*1000.0
p_size = 56.0*8.0
R = m.pow(10, 6)
D = 20.0/1000.0

A = p_size/convert_rate
print("A: ", A)

B = D + p_size/R
print("B: ", B)

total = A*2 + B
print("A*2 + B: ", total)
print("total msec: ", total*1000.0)

# Qn 4
print("\nQUESTION 4")

p_size = 1500.0*8.0
M = 100*1000
R = 100.0*m.pow(10, 6)
S = 2.0*m.pow(10, 8)

print("ack: ", 320.0/R)

D = M/S
print("D: ", D)
print("D msec: ", D*1000)

Tt = p_size/R
print("Tt: ", Tt)

RTT = 2.0*(D)
print("RTT: ", RTT)
print("RTT ms: ", RTT*1000)

BDP = D*R
print("BDP: ", BDP)
N = BDP/p_size
print("N: ", N)

# Qn 5
print("\nQUESTION 5")

R = m.pow(10, 9)
M = 93.45*m.pow(10, 6)*(1609.34)
S = 3.0*m.pow(10, 8)
print("M: ", M)


D = M/S
print("D: ", D)
Tt = 1.0/R
print("Tt: ", Tt)
RTT = 2.0*(D+Tt)
print("mars RTT: ", RTT)

BDP = RTT*R
print("mars BDP: ", BDP)
print("mars BDP Gb: ", BDP/(m.pow(10, 9)))

# Qn 6
print("\nQUESTION 6")

total_f_size = 1000*8*m.pow(10, 6)
RTT = 160.0/1000.0
p_size = 1.0*1000.0*8.0
R = 4.0*8.0*pow(10, 6)

time_per_pack = p_size/R
num_packets = total_f_size/p_size
all_packs = num_packets*time_per_pack

total_time_a = RTT*2.0 + total_f_size/R + RTT/2.0
print("total_time_a: ", total_time_a)

total_time = total_time_a + num_packets*RTT
print("b total_time: ", total_time)

num_RTTs = num_packets/50.0
print("num_RTTs: ", num_RTTs)

total_time = RTT*2.0 + RTT*num_RTTs
print("c total_time: ", total_time)

num_sent = 0
for i in range(1000):
    if(num_sent >= total_f_size):
        break
    num_sent += (p_size*pow(2, i))

num_RTTs = i
print("num_RTTs: ", num_RTTs)
print("num_sent: ", num_sent)
print("total_f_size: ", total_f_size)

total_time = RTT*2.0 + RTT*num_RTTs
print("d total_time: ", total_time)

# Qn 7
print("\nQUESTION 7")
S = 2.3*m.pow(10, 8)
R = 10.0*m.pow(10, 9)
bit_width = S/R
print("bit_width ", bit_width)


# Qn 8
print("\nQUESTION 8")
M = 20000.0*1000.0
R = m.pow(10, 9)
S = 2.5*m.pow(10, 8)
BDP = (R)*(M/S)
print("BDP ", BDP)

# BDP = (R)*(M/S + 800000.0/R)
# print("BDP ", BDP)

bit_width = S/R
print("bit_width ", bit_width)

p_size = 40000.0
time_per_pack = p_size/R
num_packets = 20.0
all_packs = num_packets*(time_per_pack+M/S)*2

total_time = all_packs
print("total_time ", total_time)
print("total_time ms ", total_time*1000)

# Qn 9
print("\nQUESTION 9")
R = 10*m.pow(10, 6)
S = 2.4*m.pow(10, 8)
M = 36000.0*1000.0
D = M/S
print("D ", D)
BDP = R*D
print("BDP ", BDP)
print("BDP MB ", BDP/(m.pow(10, 6)*8))

c = R*60
print("c ", c)

# Qn 10
# long answer

# Qn 11

# Qn 12

# Qn 13

# Qn 14
