// RYLR998 LoRa Transmitter (Node A - Send Only)
// Written for IoTbhai

#include <HardwareSerial.h>
#include <DHT.h>

// --- SENSOR CONFIG ---
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// --- LORA CONFIGURATION ---
const int localAddress = 1;       
const int destinationAddress = 2; // Sending to the Gateway
const int networkID = 5;          

#define RX_PIN 16 
#define TX_PIN 17 
HardwareSerial LoRaSerial(2);

long lastSendTime = 0;
int interval = 5000; // Send every 5 seconds

void setup() {
  Serial.begin(115200);
  while (!Serial);

  LoRaSerial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  dht.begin();
  
  delay(1000);
  Serial.println("--- IoTbhai Node A (Transmit Only) Starting ---");

  // Configure LoRa module
  sendAT("AT"); 
  sendAT("AT+ADDRESS=" + String(localAddress));
  sendAT("AT+NETWORKID=" + String(networkID));
}

void loop() {
  // ONLY SEND DATA (No listening)
  if (millis() - lastSendTime > interval) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      // Create JSON payload
      String payload = "{\"temp\":" + String(t, 1) + ",\"humidity\":" + String(h, 1) + "}";
      
      Serial.println("Transmitting: " + payload);
      
      // Send the data
      String cmd = "AT+SEND=" + String(destinationAddress) + "," + String(payload.length()) + "," + payload;
      sendAT(cmd);
    }
    lastSendTime = millis();
  }
}

// Helper to send AT commands
void sendAT(String cmd) {
  LoRaSerial.print(cmd + "\r\n"); 
  delay(100); 
}