#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library
#include <Servo.h>
 

//////////////////////////
// MicroOLED Definition //
//////////////////////////
#define PIN_RESET 255  // Connect RST to pin 9 (SPI & I2C)
#define DC_JUMPER 0  // DC jumper setting(I2C only)
MicroOLED oled(PIN_RESET, DC_JUMPER);  // I2C Example

// WiFi Credentials
const char* ssid = "";
const char* password = "";
// MQTT Credentials
const char* mqtt_server = ".cloudmqtt.com";
const char* mqtt_username = "";
const char* mqtt_password = "";
const int   mqtt_port=1;

Servo myservo;  // create servo object to control a servo 

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient espClient;
PubSubClient client(mqtt_server,mqtt_port,callback,espClient);

long lastMsg = 0;

int value = 0;
String strTemperature,strPressure,strServo, strLed;    //MQTT values will be stored in this variables
byte ledSignal=1;

int servoPos=90;

void setup() {
  
  Serial.begin(115200);
  myservo.attach(D8);  // attaches the servo on D8 to the servo object 
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, ledSignal);   // Turn the LED off (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is acive low on the ESP-01)
  oled.begin();    // Initialize the OLED
  oled.clear(ALL); // Clear the display's internal memory
  oled.display();  // Display what's in the buffer (splashscreen)
  delay(500);
  oled.setFontType(3);  // Set font to type 3 (Large)
  oled.clear(PAGE);     // Clear the page
  oled.setCursor(0, 0); // Set cursor to top-left
  oled.print("MQTT");
  oled.display();
  delay(500);
  setup_wifi();
  reconnect();
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  oled.clear(PAGE);
  oled.setCursor(0, 0);
  oled.setFontType(0);
  oled.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
    oled.print(".");
    oled.display();
  }
 Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  oled.setCursor(0, 32);
  oled.setFontType(0);
  oled.println("WiFi");
  oled.println("connected");
  oled.display();
  delay(1000);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] -> ");
  payload[length] = '\0';
  // check topic to identify type of content
  // values will be stored and displayed in the main loop
  // servo position will be set in the main loop
  if(String(topic) == "display/temperature") {
    // convert payload to String
    strTemperature = String((char*)payload);
    Serial.println(strTemperature);
    }
  if(String(topic) == "display/pressure") {
    // convert payload to String
    strPressure = String((char*)payload);
    Serial.println(strPressure);
  }
    if(String(topic) == "display/servo") {
    // convert payload to int
    strServo = String((char*)payload);
    Serial.println(strServo);
    servoPos=strServo.toInt();
  }
    if(String(topic) == "display/led") {
    // convert payload to int
    strLed = String((char*)payload);
    Serial.println(strLed);
    ledSignal=1-strLed.toInt();  // onboard LED is active LOW
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    uint32_t chipid=ESP.getChipId();
    char clientid[25];
    snprintf(clientid,25,"WIFI-Display-%08X",chipid); //this adds the mac address to the client for a unique id
    Serial.print("Client ID: ");
    Serial.println(clientid);
    if (client.connect(clientid,mqtt_username,mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("Say", "-t 'hello world'");
      // ... and resubscribe
      client.subscribe("display/+");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    oled.clear(PAGE);
    oled.setFontType(2);  // Set font to type 2
    oled.setCursor(0, 0); // Set cursor to top-left
    oled.print(strTemperature);
    oled.setCursor(0,20);
    oled.setFontType(0);
    oled.print("Pressure:");
    oled.setFontType(1);  //set font to type 1
    oled.setCursor(0,32);
    oled.print(strPressure);
    oled.display();
    myservo.write(servoPos);
    digitalWrite(LED_BUILTIN, ledSignal);   // Turn the LED on/off
  }
}  
