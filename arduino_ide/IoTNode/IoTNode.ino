#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include "time.h"
#include <string.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

#define SS_PIN 5
#define RST_PIN 0

#define DHTPIN 4    
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE); 

unsigned long sensorReadInterval = 60000;
unsigned long lastSensorReadTime = 0; 

const int ledPin = 15;
const int redPin = 2;
const char node_id[] = "T64";
struct tm timeinfo;
unsigned long readDelay = 5000; // 5 seconds
unsigned long lastReadTime = 0;
char lastSentUID[50];

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

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

  Serial.printf("%s, %s %02d %04d %02d:%02d:%02d",
               dayOfWeek(Time.tm_wday),  // Convert day of the week to string
               monthName(Time.tm_mon),   // Convert month to string
               Time.tm_mday,             // Day of the month
               Time.tm_year + 1900,      // Year
               Time.tm_hour,             // Hour
               Time.tm_min,              // Minute
               Time.tm_sec);             // Second


               return;
}

void callback(char *topic, byte *payload, unsigned int length) {


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

  if(strcmp(nodeID, node_id) != 0) return; //ignora mensagem se nao for para este nó especifico

  const char* localTimeDate = doc["localTimeDate"];
  int typeOfEvent = doc["typeOfEvent"];
  const char* cardUID = doc["cardUID"];
  const char* direction = doc["direction"];
  int answer = doc["answer"];
  const char* studentName = doc["studentName"];

  if(strcmp(nodeID, node_id) == 0 && strcmp(direction, "NB") == 0){ //ignora mensagem se originar nele mesmo
    return; 
  }


  //Serial.println("Message arrived at time: ");
  //  getLocalTime(&timeinfo);

  //printTime(timeinfo);



  // Print values
 // Serial.println();
 // Serial.print("nodeID: ");
 // Serial.println(nodeID);
 // Serial.print("localTimeDate: ");
 // Serial.println(localTimeDate);
 // Serial.print("typeOfEvent: ");
 // Serial.println(typeOfEvent);
 // Serial.print("cardUID: ");
 // Serial.println(cardUID);
  //Serial.print("direction: ");
 // Serial.println(direction);
 // Serial.print("answer: ");
 // Serial.println(answer);

  analyzeMessage(nodeID,localTimeDate, typeOfEvent, cardUID, direction, answer, studentName, 'null');
}

void sendJsonMessage(const char* nodeID, const char* localTimeDate, int typeOfEvent, const char* cardUID, const char* direction,  int answer, const char* studentName, const char* tempHumid) {
  // Create a JSON document
  StaticJsonDocument<200> doc;

 // Serial.println("MQTT Message uploading...");

  // Populate the JSON document with the provided data
  doc["nodeID"] = nodeID;
  doc["localTimeDate"] = localTimeDate;
  doc["typeOfEvent"] = typeOfEvent;
  doc["cardUID"] = cardUID;
  doc["direction"] = direction;
  doc["answer"] = answer; 
  doc["studentName"] = studentName;
  doc["tempHumid"] = tempHumid;
  //direction: NB(Node-Border) ,BN(Border-Node), BC(Border-Cloud), CB(Cloud_Border)

  // Serialize the JSON document to a string
  char jsonBuffer[256]; // Adjust the buffer size as needed
  size_t n = serializeJson(doc, jsonBuffer);

  // Publish the JSON message via MQTT
  if (client.connected()) {
      client.publish(topic, jsonBuffer, n);
      Serial.printf("(EVENT) MQTT Message of type %d sent at time: ",typeOfEvent);
        getLocalTime(&timeinfo);
      printTime(timeinfo);
  }
}

void analyzeMessage(const char* nodeID, const char* localTimeDate, int typeOfEvent, const char* cardUID, const char* direction,  int answer, const char* studentName, const char* tempHumid){


  if (typeOfEvent == 1){ //1- request attendance 


    if (answer == -3)
    {
      Serial.println();
      Serial.printf("(EVENT) Denied attendance registration at time: ");
        getLocalTime(&timeinfo);
      printTime(timeinfo);
      Serial.println();

      Serial.printf("Reason: Student (%s) with card UID: (%s) already registered!", studentName, cardUID);
      Serial.println("");

             digitalWrite (redPin, HIGH);	// turn on the LED
      delay(250);	// wait for half a second or 500 milliseconds
      digitalWrite (redPin, LOW);	// turn off the LED
      delay(250);	// wait for half a second or 500 milliseconds
    }

    if (answer == -2)
    {
      Serial.println();
      Serial.printf("(EVENT) Denied attendance registration for student (%s) with card UID (%s) registration at time: ",studentName,cardUID);
        getLocalTime(&timeinfo);
      printTime(timeinfo);
      Serial.println();

      Serial.println("Reason: No class was found in this room!");
      Serial.println("");

             digitalWrite (redPin, HIGH);	// turn on the LED
      delay(250);	// wait for half a second or 500 milliseconds
      digitalWrite (redPin, LOW);	// turn off the LED
      delay(250);	// wait for half a second or 500 milliseconds
    }

    if (answer == -1)
    {
        Serial.println();
        Serial.printf("(EVENT) Denied attendance registration for student (%s) with card UID (%s) at time: ",studentName,cardUID);
        getLocalTime(&timeinfo);
        printTime(timeinfo);
        Serial.println();
        Serial.println("Reason: Student not registered in this class!");

               digitalWrite (redPin, HIGH);	// turn on the LED
      delay(250);	// wait for half a second or 500 milliseconds
      digitalWrite (redPin, LOW);	// turn off the LED
      delay(250);	// wait for half a second or 500 milliseconds
    }

        if (answer == -4)
    {
        Serial.println();
        Serial.printf("(EVENT) Denied attendance registration for student (%s) with card UID (%s) at time: ",studentName,cardUID);
        getLocalTime(&timeinfo);
        printTime(timeinfo);
        Serial.println();
        Serial.println("Reason: Card not registered!");

       digitalWrite (redPin, HIGH);	// turn on the LED
      delay(250);	// wait for half a second or 500 milliseconds
      digitalWrite (redPin, LOW);	// turn off the LED
      delay(250);	// wait for half a second or 500 milliseconds
    }
    

    if (answer == 1)
    {
      Serial.println();
       Serial.printf("(EVENT) Authorized attendance registration for student (%s) with card UID (%s) at time: ", studentName,cardUID);
         getLocalTime(&timeinfo);
       printTime(timeinfo);
       Serial.println();

       digitalWrite (ledPin, HIGH);	// turn on the LED
      delay(250);	// wait for half a second or 500 milliseconds
      digitalWrite (ledPin, LOW);	// turn off the LED
      delay(250);	// wait for half a second or 500 milliseconds
    }     
  }
}

const char* tmToString(const tm &timeInfo) {
    static char buffer[15]; // Static buffer to hold the formatted string
    sprintf(buffer, "%02d%02d%02d%02d%02d%04d",
            timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec,
            timeInfo.tm_mday, timeInfo.tm_mon + 1, timeInfo.tm_year + 1900);
    return buffer; // Return the buffer
}

void restartWiFi() {
  // Disconnect from current network
  WiFi.disconnect();
  delay(1000); // Wait for the disconnection to complete

  // Reconnect to the network
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting to reconnect to WiFi...");
    WiFi.begin(ssid, password); // Replace ssid and password with your network credentials
    delay(5000); // Wait for connection
  }

  Serial.println("WiFi reconnected.");
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
      restartWiFi();
      delay(5000);
    }
  }
  return;
}

void SendSensorData((const char* nodeID, const char* localTimeDate, int typeOfEvent, const char* cardUID, const char* direction,  int answer, const char* studentName, const char* tempHumid)) {
  // Create a JSON document
  StaticJsonDocument<200> doc;

 // Serial.println("MQTT Message uploading...");

  // Populate the JSON document with the provided data
  doc["nodeID"] = nodeID;
  doc["localTimeDate"] = localTimeDate;
  doc["typeOfEvent"] = typeOfEvent;
  doc["cardUID"] = cardUID;
  doc["direction"] = direction;
  doc["answer"] = answer; 
  doc["studentName"] = studentName;
  doc["tempHumid"] = tempHumid;
  //direction: NB(Node-Border) ,BN(Border-Node), BC(Border-Cloud), CB(Cloud_Border)
  


  // Serializar a mensagem JSON
  char jsonBuffer[256];
  size_t n = serializeJson(doc, jsonBuffer);

  // Enviar a mensagem JSON para o broker MQTT
  if (client.connected()) {
    client.publish(topic, jsonBuffer, n);
    Serial.println("Sensor data sent to MQTT broker");
  }
}  

void setup() 
{
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  pinMode (ledPin, OUTPUT); //led
  pinMode (redPin,OUTPUT);

  dht.begin();
  //connect to wifi 

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
  getLocalTime(&timeinfo);



    //establish mqtt connection

    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "R62";
        //client_id += String(WiFi.macAddress());
        Serial.printf("The client %s is attempting to connect to the Border Router MQTT broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Border Router BR01 broker connected");
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

     if (!client.connected()) { //ensure MQTT connectivity
    reconnect();
    }

    // Process incoming MQTT messages
    client.loop();

    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent()) 
    {
      return;
    }

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) 
    {
      return;
    }

    unsigned long currentTime = millis();


    String content= "";
    byte letter;

    for (byte i = 0; i < mfrc522.uid.size; i++) 
    {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    content.toUpperCase();

    const char* timeString = tmToString(timeinfo);

    content = content.substring(1); //remove first character
    const char* UID = content.c_str();

    if (lastReadTime == 0){
      Serial.println();
      Serial.printf("(EVENT) Card is read with UID (%s) at time: ",content);
      getLocalTime(&timeinfo);
      printTime(timeinfo);
      Serial.println();

      strncpy(lastSentUID, UID, sizeof(lastSentUID) - 1);
      lastSentUID[sizeof(lastSentUID) - 1] = '\0'; // Ensure null termination

      sendJsonMessage(node_id,timeString,1,UID,"NS",0,"null","null");
      lastReadTime = currentTime;

    }


    if (currentTime - lastReadTime >= readDelay && strcmp(lastSentUID, UID) == 0){ //caso tempo passado seja menor que o delay definido e o cartao seja o mesmo
      Serial.println();
      Serial.printf("(EVENT) Card is read with UID (%s) at time: ",content);
      getLocalTime(&timeinfo);
      printTime(timeinfo);
      Serial.println();

      strncpy(lastSentUID, UID, sizeof(lastSentUID) - 1);
      lastSentUID[sizeof(lastSentUID) - 1] = '\0'; // Ensure null termination

      sendJsonMessage(node_id,timeString,1,UID,"NS",0,"null","null"),;
      lastReadTime = currentTime;
    }

        
    if (strcmp(UID, lastSentUID) != 0){ //caso cartao seja diferente independetmente do delay
      Serial.println();
      Serial.printf("(EVENT) Card is read with UID (%s) at time: ",content);
      getLocalTime(&timeinfo);
      printTime(timeinfo);
      Serial.println();

      strncpy(lastSentUID, UID, sizeof(lastSentUID) - 1);
      lastSentUID[sizeof(lastSentUID) - 1] = '\0'; // Ensure null termination

      sendJsonMessage(node_id,timeString,1,UID,"NS",0,"null");
      lastReadTime = currentTime;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastSensorReadTime >= sensorReadInterval) {
    // Ler e enviar os dados do sensor
    float temperature = dht.readTemperature(); // Ler a temperatura
    float humidity = dht.readHumidity(); // Ler a umidade
    SendSensorData(node_id,timeString,0,"null","NS","null","null",tempHumid);
    lastSensorReadTime = currentTime;
  }


} 