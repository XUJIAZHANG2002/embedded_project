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
data_list = []
for q in queries:
	
	response = requests.request("GET", url, params=q)
	data_list.append(response.text)
#print(data)



message = data_list[0]+data_list[1]+data_list[2]

print(type(message))
HOST = "192.168.1.10"  # The server's hostname or IP address
PORT = 7  # The port used by the server
import json



# Convert each JSON string to a dictionary and then to a string
result = "".join(
    f"StartTime:{item['startTime']} CloudCover:{item['values']['cloudCover']} Temperature:{item['values']['temperature']}"
    for data in data_list
    for item in json.loads(data)["data"]["timelines"][0]["intervals"]
)




with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(bytes(result,'utf-8'))
    data = s.recv(1024)

