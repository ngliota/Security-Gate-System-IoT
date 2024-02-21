//Made By ngliota(Agung Siregar),Ryan
//Thanks to Anam,pondadims(Alfonda),Ferdi helping me with this project
#include <WiFi.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <SPI.h>
#include <PubSubClient.h>

#define RFID_SS_PIN 21
#define RFID_RST_PIN 22
#define SERVO1_PIN 12 // Entrance gate servo pin
#define SERVO2_PIN 13 // Exit gate servo pin
#define IR_SENSOR_PIN 14

unsigned long gateOpenTime = 0;
const unsigned long gateOpenDuration = 3500;

Servo entranceGateServo; // SG90 servo for entrance gate
Servo exitGateServo;     // MG90S servo for exit gate
IPAddress ip;
WiFiClient espClient;
PubSubClient client(espClient);
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);
int checkConditions();

const int MAX_CARDS = 4;
String registeredCards[MAX_CARDS] = {
  "668620033",
  "3120017943",
  "513910611629190",
  // Add more registered card serial numbers here...
};

const char* ssid = "hiro";
const char* password = "Ptmnvdxtc!";
bool useMQTT = true;
const char* mqtt_server = "192.168.0.108";
const char* mqtt_user = "";
const char* mqtt_password = "";
const char* topic_PUBLISH_SERVO1 = "esp32/servo1";
const char* topic_PUBLISH_SERVO2 = "esp32/servo2";



void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect(); 
  entranceGateServo.attach(SERVO1_PIN); // Attach the entrance gate servo to its pin
  entranceGateServo.write(150); // Adjust the angle as per the gate mechanism
  exitGateServo.attach(SERVO2_PIN);    // Attach the exit gate servo to its pin
  exitGateServo.write(53); // Adjust the angle as per the gate mechanism
  pinMode(IR_SENSOR_PIN, INPUT);

  // Other setup routines for WiFi, MQTT, LCD, etc.
}

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 20) {
      Serial.println("\nFailed to connect to WiFi! Please check credentials.");
      return;
    }
  }
  Serial.println("\nWiFi connected");
}


void loop() { 
  handleRFID(); // Check for RFID card presence and access control
//  handleIRSensor();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int condition = checkConditions();
  Serial.println(condition);
  sendToNodeRed(condition);
  delay(1500);
}


void sendToNodeRed(int condition) {
  char msg[10];
  snprintf(msg, sizeof(msg), "%d", condition);

const char* topic = "node-red-topic";
  client.publish(topic, msg);
}

void handleRFID() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String cardSerial = getCardSerial();
    Serial.println("RFID card detected!");
    Serial.println("Card Serial Number: " + cardSerial);

    if (isCardRegistered(cardSerial)) {
      Serial.println("Card is registered.");
      openEntranceGate(); // Open the entrance gate for registered cards
    } 
    else {
      Serial.println("Card is not registered.");
      // Optionally, perform actions for unregistered cards
    }
  }
}

void handleIRSensor() {
  if (digitalRead(IR_SENSOR_PIN) == LOW) {
    Serial.println("Car detected near exit gate!");
    openExitGate();
    gateOpenTime = millis();
  }

  if (millis() - gateOpenTime >= gateOpenDuration && gateOpenTime != 0) {
    Serial.println("Closing exit gate");
    closeExitGate();
    gateOpenTime = 0; // Reset the gate open time
  }
  // Additional actions can be added here, e.g., sending MQTT messages, etc.
}

void openExitGate() {
  exitGateServo.write(150); // Open the exit gate
}

void closeExitGate() {
  exitGateServo.write(53); // Close the exit gate
}

void openEntranceGate() {
  entranceGateServo.write(53); // Adjust the angle as per the gate mechanism
  delay (3500); // kasih delay
  entranceGateServo.write(150); // Close the entrance gate
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message received on topic: " + String(topic));
  // Handle incoming messages here, if needed
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_user, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(topic_PUBLISH_SERVO1);
      client.subscribe(topic_PUBLISH_SERVO2);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publishMessage(const char* topic, const char* message) {
  Serial.println("Attempting to publish message to topic: " + String(topic)); // Pesan debug tambahan
  if (client.connected()) {
    if (client.publish(topic, message)) {
      Serial.println("Message published to topic: " + String(topic));
    } else {
      Serial.println("Failed to publish message to topic: " + String(topic));
      reconnect(); // Panggil fungsi reconnect jika gagal mem-publish
    }
  } else {
    Serial.println("Not connected to MQTT broker!"); // Pesan debug tambahan
    reconnect(); // Panggil fungsi reconnect jika tidak terkoneksi
  }
}

String getCardSerial() {
  String cardSerial = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardSerial += String(mfrc522.uid.uidByte[i]);
  }
  return cardSerial;
}

bool isCardRegistered(String cardSerial) {
  for (int i = 0; i < MAX_CARDS; i++) {
    if (registeredCards[i] == cardSerial) {
      return true;
    }
  }
  return false;
}

int checkConditions() {
  return 0;
}
