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
print("pp_band bits: ", pp_band)
print("pp_band MB/s: ", pp_band/8.0/(m.pow(10.0, 6)))

req_band = pp_band*total_clients
print("req_band: bps", req_band)
print("req_band: GBps", req_band/8.0/(m.pow(10.0, 9)))

R = 10.0*m.pow(10, 9)  # bits/sec
print("R: ", R)
req_servers = req_band/R
print("req_servers: ", m.ceil(req_servers))
print("req_servers 1hr per day balance: ", m.ceil(req_servers/24))

servers_per_datacenter = 10000.0
num_data_centers = m.ceil(req_servers)/servers_per_datacenter
print("num_data_centers: ", m.ceil(num_data_centers))
print("num cities: ", m.ceil(num_data_centers)/3)


RTT = 2.0/1000.0  # s
S = 2.0*m.pow(10, 8)  # m/s
M = RTT*S  # meters
M = M/1000.0  # km
print("M: ", M)

# could probably have smaller cities
# Toluca
# Puebla
# Cuernavaca
# Tezontepec

print("Question 3")

# starlink_h = 340*1609.34  # meters
# space_c_h = starlink_h + 100.0*1000.0  # meters,bc 100km higher than spacex
# # https://www.theregister.com/2022/09/23/starlink_broadband_speeds_slow/
# # the speeds in North America reach a median of 60Mbps
# print("space_c_h m : ", space_c_h)
# print("space_c_h km : ", space_c_h/1000.0)
# # orbital period 01:37:39.94 hh:mm:ss
# # https://keisan.casio.com/exec/system/1224665242
# orbital_radius = 7025.14  # km
# flight_v = 7.5325396748244  # km/s

# # radio waves speed 300,000 km per second.
# speed = 300000.0*1000.0  # m/s
# dist = space_c_h
# rtt = 2*dist/speed
# print("rtt s : ", rtt)
# print("rtt ms : ", rtt*1000.0)


# starlink_r = 60.0*8*m.pow(10, 6)  # bits/s
# space_c = starlink_r*1.5  # 50% more than starlink
