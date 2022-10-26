import math as m

print("QUESTION 1")

population = 32.0*m.pow(10.0, 6)
percent_users = 0.25
total_clients = population*percent_users
print("total_clients: ", total_clients)

img = 1080*1920*24*0.1
print("img: ", img)

audio = 1.0/60.0*(m.pow(10.0, 6))*0.1
print("audio: ", audio)


frame_size = img+audio
frame_size += frame_size*0.027
print("frame_size bits: ", frame_size)
print("frame_size MB: ", frame_size/8.0/(m.pow(10.0, 6)))

fps = 30.0
pp_band = fps*frame_size
print("pp_band bits: ", pp_band/8.0/(m.pow(10.0, 6)))

req_band = pp_band*total_clients
print("frame_size bits: ", frame_size)


print("req_band: bps", req_band)
print("req_band: GBps", req_band/8.0/(m.pow(10.0, 6)))

R = 10.0*m.pow(10, 9)  # bits/sec
print("R: ", R)
req_servers = req_band/R
print("req_servers: ", req_servers)

servers_per_datacenter = 10000.0
num_data_centers = req_servers/servers_per_datacenter
print("num_data_centers: ", num_data_centers)
print("num cities: ", num_data_centers/3)


RTT = 2.0/1000.0  # s
S = 3.0*m.pow(10, 8)  # m/s
M = RTT*S  # meters
M = M/1000.0  # km
print("M: ", M)
