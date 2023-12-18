import socket
import requests

url = "https://api.tomorrow.io/v4/timelines"

querystring1 = {
"location":"New York",
"fields":["temperature", "cloudCover"],
"units":"imperial",
"timesteps":"1d",
"apikey":"rbv146AnfUsuHquseBxaCZkvdvGWSall"}

querystring2 = {
"location":"DuBai",
"fields":["temperature", "cloudCover"],
"units":"imperial",
"timesteps":"1d",
"apikey":"rbv146AnfUsuHquseBxaCZkvdvGWSall"}

querystring3 = {
"location":"Shang Hai",
"fields":["temperature", "cloudCover"],
"units":"imperial",
"timesteps":"1d",
"apikey":"rbv146AnfUsuHquseBxaCZkvdvGWSall"}

queries = [querystring1,querystring2,querystring3]
data = []
for q in queries:
	
	response = requests.request("GET", url, params=q)
	data.append(response.text)
#print(data)

for d in data:
	print(d)
	print()

message = data

print(type(message))
HOST = "192.168.1.10"  # The server's hostname or IP address
PORT = 7  # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b(message))
    data = s.recv(1024)

#print(f"Received {data!r}")
