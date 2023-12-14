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
