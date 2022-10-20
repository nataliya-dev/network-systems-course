import math as m

print("QUESTION 1")

population = 32.0*m.pow(10.0, 6)
percent_users = 0.25
total_clients = population*percent_users
print("total_clients: ", total_clients)
file_size = 287.6*8*m.pow(10, 6)  # bits
print("file_size: ", file_size)
file_len = 60.0  # seconds

req_band = (file_size)/(file_len)*total_clients
print("req_band: ", req_band)

R = 10.0*m.pow(10, 9)  # bits/sec
print("R: ", R)
req_servers = req_band/R
print("req_servers: ", req_servers)

servers_per_datacenter = 10000.0
num_data_centers = req_servers/servers_per_datacenter
print("num_data_centers: ", num_data_centers)

RTT = 2.0/1000  # s
S = 2.3*m.pow(10, 8)  # m/s
M = RTT*S  # meters
M = M/1000.0  # km
print("M: ", M)

# could probably have smaller cities
# Toluca
# Puebla
# Cuernavaca
# Tezontepec
