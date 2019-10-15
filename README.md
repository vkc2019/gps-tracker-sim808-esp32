# gps-tracker-sim808-esp32


## Release : V0.0.1
==============================
### replace WIFI_SSID
### replace WIFI_PASSWORD
### replace MQTT_SERVER_URL
### replace APN

## Working:
```
By Providing the above detials, this will connect the mqtt server. 

when publish any message on `gps_808/cmd` device will return the json message with gps informatino as follows
```
```json
{
  "lat": "17.42411804",
  "lng": "78.43275452",
  "spped": "0.28000000",
  "alt": 596,
  "vsat": 11,
  "usat": 8,
  "year": 2019,
  "month": 10,
  "day": 15,
  "hour": 8,
  "minute": 33,
  "second": 52
}
```



## TODO:

### set time interval to send gps data to server
### use wifi manager to use wifi if available

