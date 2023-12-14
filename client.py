import socket
import requests

url = "https://api.tomorrow.io/v4/timelines"

querystring = {
"location":"33, -84",
"fields":["temperature", "cloudCover"],
"units":"imperial",
"timesteps":"1d",
"apikey":"rbv146AnfUsuHquseBxaCZkvdvGWSall"}

response = requests.request("GET", url, params=querystring)
print(response.text)
print(type(response.text))
message = response.text
HOST = "192.168.1.10"  # The server's hostname or IP address
PORT = 7  # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b(message))
    data = s.recv(1024)

print(f"Received {data!r}")
