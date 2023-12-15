import requests
import argparse
from datetime import datetime, timedelta

def get_weather(city, day):
    url = "https://api.tomorrow.io/v4/timelines"
    end_date = datetime.utcnow() + timedelta(days=day)
    start_date = end_date - timedelta(days=1)
    
    querystring = {
        "location": city,
        "fields": ["temperature", "cloudCover"],
        "units": "imperial",
        "timesteps": "1d",
        "startTime": start_date.isoformat() + "Z",
        "endTime": end_date.isoformat() + "Z",
        "apikey": "rbv146AnfUsuHquseBxaCZkvdvGWSall"
    }

    response = requests.request("GET", url, params=querystring)
    data = response.json().get("data", {}).get("timelines", [])
    
    if data:
        weather_data = data[0].get("intervals", [])
        result = [{"StartTime": interval["startTime"], "Values": interval["values"]} for interval in weather_data]
        return result

    return []

def main():
    parser = argparse.ArgumentParser(description="Get weather information for a city and day")
    parser.add_argument("--city", type=str, help="City name", required=True)
    parser.add_argument("--day", type=int, help="Number of days", required=True)
    args = parser.parse_args()

    weather_data = get_weather(args.city, args.day)
    print(args.city, "Weather in ",args.day,"days")
    for interval in weather_data:
        print(interval)

if __name__ == "__main__":
    main()
