#include <WiFi.h>
#include "time.h"
#include <string.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "NOS-5000";
const char* password = "ZMVTZ3QG";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;


// MQTT Broker
const char *mqtt_broker = "192.168.1.84";
const char *topic = "smartClassRoom";
const char *mqtt_username = "miguel";
const char *mqtt_password = "iot";
const int mqtt_port = 1883;


WiFiClient espClient;
PubSubClient client(espClient);


const char* dayOfWeek(int dow) {
  switch (dow) {
    case 0:
      return "Sunday";
    case 1:
      return "Monday";
    case 2:
      return "Tuesday";
    case 3:
      return "Wednesday";
    case 4:
      return "Thursday";
    case 5:
      return "Friday";
    case 6:
      return "Saturday";
    default:
      return "Unknown";
  }
}

const char* monthName(int month) {
  switch (month) {
    case 0:
      return "January";
    case 1:
      return "February";
    case 2:
      return "March";
    case 3:
      return "April";
    case 4:
      return "May";
    case 5:
      return "June";
    case 6:
      return "July";
    case 7:
      return "August";
    case 8:
      return "September";
    case 9:
      return "October";
    case 10:
      return "November";
    case 11:
      return "December";
    default:
      return "Unknown";
  }
}


void printTime(struct tm Time){

  char buffer[100];

  sprintf(buffer,"%s, %s %02d %04d %02d:%02d:%02d",
               dayOfWeek(Time.tm_wday),  // Convert day of the week to string
               monthName(Time.tm_mon),   // Convert month to string
               Time.tm_mday,             // Day of the month
               Time.tm_year + 1900,      // Year
               Time.tm_hour,             // Hour
               Time.tm_min,              // Minute
               Time.tm_sec);             // Second

  Serial.println(buffer);
  //client.publish(topic, buffer);


               return;
}

void callback(char *topic, byte *payload, unsigned int length) {

    struct tm timeinfo;
    getLocalTime(&timeinfo);

    Serial.print("Message arrived at time: ");
    printTime(timeinfo);

    // Parse JSON payload
  StaticJsonDocument<200> doc; // Adjust the buffer size as needed
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Extract values from JSON
  const char* nodeID = doc["nodeID"];
  const char* localTimeDate = doc["localTimeDate"];
  int typeOfEvent = doc["typeOfEvent"];
  const char* cardUID = doc["cardUID"];
  const char* direction = doc["direction"];
  int answer = doc["answer"];


  // Print values
  Serial.print("nodeID: ");
  Serial.println(nodeID);
  Serial.print("localTimeDate: ");
  Serial.println(localTimeDate);
  Serial.print("typeOfEvent: ");
  Serial.println(typeOfEvent);
  Serial.print("cardUID: ");
  Serial.println(cardUID);
  Serial.print("direction: ");
  Serial.println(direction);
  Serial.print("answer: ");
  Serial.println(answer);


}

void sendJsonMessage(const char* nodeID, const char* localTimeDate, int typeOfEvent, const char* cardUID, const char* direction,  int answer, const char* direction, const char* tempHumid) {
  // Create a JSON document
  StaticJsonDocument<200> doc;

  // Populate the JSON document with the provided data
  doc["nodeID"] = nodeID;
  doc["localTimeDate"] = localTimeDate;
  doc["typeOfEvent"] = typeOfEvent;
  doc["cardUID"] = cardUID;
  doc["direction"] = direction;
  doc["answer"] = answer; 
  doc["tempHumid"] = tempHumid; 


  //direction: NB(Node-Border) ,BN(Border-Node), BC(Border-Cloud), CB(Cloud_Border)

  // Serialize the JSON document to a string
  char jsonBuffer[256]; // Adjust the buffer size as needed
  size_t n = serializeJson(doc, jsonBuffer);

  // Publish the JSON message via MQTT
  if (client.connected()) {
    client.publish(topic, jsonBuffer, n);
  }
}



void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ArduinoClient")) {
      Serial.println("connected");
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() 
{
  Serial.begin(9600);   // Initiate a serial communication
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");

  //get current time

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  //establish mqtt connection

    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "BR01"; //border router 01
        //client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public EMQX MQTT broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }

    client.subscribe(topic);

}


}
void loop() 
{
   if (!client.connected()) {
    reconnect();
  }

  client.loop();
} 
