
#include <ArduinoJson.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>


#define TINY_GSM_MODEM_SIM808

#define SerialMon Serial

#define SerialAT Serial1

#define TINY_GSM_DEBUG SerialMon


#define TINY_GSM_YIELD() { delay(2); }

#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

#define GSM_PIN ""


//const char apn[] = "jionet";
const char apn[] = "APN";
const char gprsUser[] = "";
const char gprsPass[] = "";

const char wifiSSID[] = "WIFI_SSID";
const char wifiPass[] = "WIFI_PASSWORD";
const char* broker = "MQTT_SERVER_URL";
const char* pub_topic = "device/gps_808/data";
const char* sub_topic = "gps_808/cmd";



TinyGsm modem(SerialAT);

TinyGsmClient client(modem);

PubSubClient mqtt(client);

#define RXD2 16
#define TXD2 17

long lastReconnectAttempt = 0;

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  SerialMon.print("Message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();


  float lat, lng, speed;
  int alt, vsat, usat;
  modem.getGPS(&lat, &lng, &speed, &alt, &vsat, &usat);
  int year, month, day, hour, minute, second = 0;
  modem.getGPSTime(&year, &month, &day, &hour, &minute, &second);

  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& JSONencoder = jsonBuffer.createObject();
  JSONencoder["lat"] = String(lat, 8);
  JSONencoder["lng"] = String(lng, 8);
  JSONencoder["spped"] = String(speed, 8);
  JSONencoder["alt"] = alt;
  JSONencoder["vsat"] = vsat;
  JSONencoder["usat"] = usat;
  JSONencoder["year"] = year; 
  JSONencoder["month"] = month;
  JSONencoder["day"] = day;
  JSONencoder["hour"] = hour;
  JSONencoder["minute"] = minute;
  JSONencoder["second"] = second;

  char JSONmessageBuffer[1024];
  
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  mqtt.publish(pub_topic, JSONmessageBuffer);

}

boolean mqttConnect() {
  SerialMon.print("Connecting to ");
  SerialMon.print(broker);

  // Connect to MQTT Broker
  boolean status = mqtt.connect("GSMCLIENT", "gps_808", "hello");

  // Or, if you want to authenticate MQTT:
  //boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");

  if (status == false) {
    SerialMon.println(" fail");
    return false;
  }
  SerialMon.println(" success");
  mqtt.publish(pub_topic, "gps808 is started");
  mqtt.subscribe(sub_topic);
  return mqtt.connected();
}


void setup() {
  SerialMon.begin(115200);
  
  delay(10);
  
  SerialMon.println("Waiting...");

  SerialAT.begin(9600, SERIAL_8N1, RXD2, TXD2, false);

  delay(3000);

  SerialMon.println("Initializing modem...");

  modem.init();
  
  String modemInfo = modem.getModemInfo();
  
  SerialMon.print("Modem Info: ");
  
  SerialMon.println(modemInfo);

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(F("success"));


  bool res = modem.isGprsConnected();
  DBG("GPRS status:", res ? "connected" : "not connected");

  String ccid = modem.getSimCCID();
  SerialMon.println("CCID:" + ccid);

  String imei = modem.getIMEI();
  SerialMon.println("IMEI:" + imei);

  String cop = modem.getOperator();
  SerialMon.println("Operator:" + cop);

  IPAddress local = modem.localIP();
  SerialMon.println("Local IP:" + local.toString());

  int csq = modem.getSignalQuality();
  SerialMon.println("Signal quality:" + csq);


  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);
  if (modem.enableGPS()) {
    SerialMon.println("GPS Enabled");
  }
}

void publishGPSData()
{
  float lat, lng, speed;
  int alt, vsat, usat;
  modem.getGPS(&lat, &lng, &speed, &alt, &vsat, &usat);
  String msg = String(lat) + "--" + String(lng) + "--" + String(speed);
  mqtt.publish(pub_topic, msg.c_str());
}

unsigned char gps_data_delay = 0;
void loop() {

  if (!mqtt.connected()) {
    SerialMon.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    unsigned long t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (mqttConnect()) {
        lastReconnectAttempt = 0;
      }
    }
    delay(100);
    return;
  }
  mqtt.loop();
}
