// RYLR998 LoRa to MQTT Gateway (Node B - Receive Only)
// Written for IoTbhai

#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>

// --- WI-FI & MQTT CONFIG ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "mqtt.iotbhai.io";
const char* mqtt_topic = "iotbhai/lora_weather";

WiFiClient espClient;
PubSubClient client(espClient);

// --- LORA CONFIGURATION ---
const int localAddress = 2;       // Gateway is Address 2
const int networkID = 5;          

#define RX_PIN 16 
#define TX_PIN 17 
HardwareSerial LoRaSerial(2);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  LoRaSerial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  
  delay(1000);
  Serial.println("--- IoTbhai Node B (Receive Only Gateway) Starting ---");

  // Configure LoRa
  sendAT("AT"); 
  sendAT("AT+ADDRESS=" + String(localAddress));
  sendAT("AT+NETWORKID=" + String(networkID));

  // Connect to Network
  setupWiFi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  // Keep MQTT Connected
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // ONLY READ INCOMING LORA DATA (No sending)
  while (LoRaSerial.available()) {
    String incoming = LoRaSerial.readStringUntil('\n');
    incoming.trim(); 
    
    if (incoming.length() > 0 && incoming.indexOf("+RCV") != -1) {
       parseAndPublish(incoming);
    }
  }
}

// --- LORA PARSING & MQTT PUBLISHING ---
void parseAndPublish(String response) {
  // Expected: +RCV=1,31,{"temp":28.4,"humidity":55.2},-85,40
  int firstComma = response.indexOf(',');
  int secondComma = response.indexOf(',', firstComma + 1);
  
  if (firstComma > 0 && secondComma > 0) {
    String senderID = response.substring(5, firstComma);
    int payloadLength = response.substring(firstComma + 1, secondComma).toInt();
    
    int dataStart = secondComma + 1;
    String jsonData = response.substring(dataStart, dataStart + payloadLength);
    
    int thirdComma = dataStart + payloadLength;
    int fourthComma = response.indexOf(',', thirdComma + 1);
    String rssi = response.substring(thirdComma + 1, fourthComma);

    // Inject RSSI into JSON
    jsonData.remove(jsonData.length() - 1); 
    String finalPayload = jsonData + ",\"rssi\":" + rssi + "}";

    Serial.println("From Field Sensor ID " + senderID + " | Publishing: " + finalPayload);

    // Push to the dashboard
    client.publish(mqtt_topic, finalPayload.c_str());
  }
}

// --- HELPER FUNCTIONS ---
void setupWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

void reconnectMQTT() {
  while (!client.connected()) {
    String clientId = "IoTbhaiGateway-" + String(random(0, 1000));
    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT Connected!");
    } else {
      delay(5000); // Wait 5 seconds before retrying
    }
  }
}

void sendAT(String cmd) {
  LoRaSerial.print(cmd + "\r\n"); 
  delay(100); 
}