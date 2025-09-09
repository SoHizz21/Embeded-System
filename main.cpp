#include <Arduino.h>

//Peerapat Saengphoem 6530300376 T12

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

#include <WiFi.h>
#include <PubSubClient.h>
#define MQTT_SERVER "20.2.240.18"
#define MQTT_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define MQTT_NAME "ESP32_1"
#define LED_PIN 23
#define LED_ONBOARD 5
WiFiClient client;
PubSubClient mqtt(client);

//poten && diplayer setting
#define POT 34
#define LED 25
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SW1 5
#define SCREEN_ADDRESS 0x3C
#define EEPROM_SIZE 4095
char str[100];
char buffer[100];
int var1;
int lasttime = 0;
int conv;
int pot = 0;
int data;
int address = 376;
bool status=true;

//DISPLAY OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
 
//WIFI Callback
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
 
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if(String(topic) == "/ESP32_1/status1"){
    if(messageTemp == "toggle"){
        Serial.println("toggle");
        status = !status;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
 
  //define I/O
  pinMode(LED, OUTPUT);
  pinMode(SW1, INPUT_PULLUP);

  //WIFI Connecting
  Serial.print("Connecting to ");
  WiFi.mode(WIFI_STA);
  WiFi.begin("Wokwi-GUEST", "");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_ONBOARD, !digitalRead(LED_ONBOARD));
  }
  digitalWrite(LED_ONBOARD, HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);

  //setup EEPROM
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("Failed init EEPROM");
    delay(1000);
    ESP.restart();
  }
  Serial.println("Success init EEPROM");
 
  //setup Display OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
}

void loop() {
  //Checkconnect MQTT
  if (mqtt.connected() == false) {
      Serial.print("MQTT connection... ");
      if (mqtt.connect(MQTT_NAME, MQTT_USERNAME,
                        MQTT_PASSWORD)) {
        Serial.println("connected");
        mqtt.subscribe("/ESP32_1/status1");
      } else {
        Serial.println("failed");
        delay(5000);
      }
    } else {
      mqtt.loop();
    } 

  //Check status On button
  if(digitalRead(SW1) == LOW){
      status = !status;
  }

  //Poten set update
  pot = analogRead(POT);
  conv = pot * 255 /4095;
  if(millis() - lasttime >= 1000 ){
    lasttime = millis();
    if(status == true){
      //POTEN TO LED
        EEPROM.writeUChar(address, conv);
        EEPROM.commit();
        String topic = "/ESP32_1/conv";
        String topic1 = "/ESP32_1/ip";
        mqtt.publish(topic.c_str(), String(conv).c_str());
        mqtt.publish(topic1.c_str(), String(MQTT_SERVER).c_str());
      }
    }
   
    //readEEPROM from address
    var1 = EEPROM.readUChar(address);
    //show Display
    display.display();
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    
    if(status == true){    
      display.print(F("ON    "));
      String topicstatus = "/ESP32_1/status";
      String on = "ON";
        mqtt.publish(topicstatus.c_str(), on.c_str());
    }else{
      String topicstatus = "/ESP32_1/status";
      String off = "OFF";
        mqtt.publish(topicstatus.c_str(), off.c_str());
      display.print(F("OFF   "));
    }

    display.setTextSize(1);
    display.print(F("ADC:"));
    display.print(pot);
    
    display.setCursor(0, 16);
    display.print(F("Address:0"));
    display.print(address);
    display.print(F(" (0x"));
    display.print(address, HEX);
    display.print(F(")"));

    display.setCursor(0, 26);
    display.print(F("Data:"));
    display.print(var1);
    display.print(F("(0x"));
    display.print(var1, HEX);
    display.print(F(")"));

    display.display();
    analogWrite(LED,var1);
}
//Peerapat Saengphoem 6530300376 T12