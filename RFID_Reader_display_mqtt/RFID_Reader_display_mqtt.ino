// SIMPLIFIED FIRMWARE FOR ESP8266 WITH MFRC522 WITHOUT LEDS  

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "font.h"
/* wiring the MFRC522 to ESP8266 (ESP-12)
RST     = GPIO4
SDA(SS) = GPIO2 
MOSI    = GPIO13
MISO    = GPIO12
SCK     = GPIO14
GND     = GND
3.3V    = 3.3V
*/

/* wiring the MFRC522 to D1 Mini ESP8266 (ESP - 12)
RST     = D3
SDA(SS) = D8
MOSI    = D7
MISO    = D6
SCK     = D5
GND     = GND
3.3V    = 3.3V

Utiliser carte Wemos D1 mini pro
speed 115200
 */
 
#define RST_PIN  D3 // RST-PIN GPIO4 
#define SS_PIN  D8  // SDA-PIN GPIO2 
SSD1306  display(0x3C, D2, D1);

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

// Wifi Connection details
const char* ssid = "";
const char* password = "";

const char* mqtt_server = "";  // mqtt IP
const char* mqtt_user = "";  // mqtt user
const char* mqtt_password = "";  // mqtt password
const char* clientID = "";  // rfid name
const char* rfid_topic = "";  // rfid path

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
void setup() {
  Serial.begin(115200);
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  delay(2);
  setup_wifi();  
  display.init();
  display.flipScreenVertically();
  display.setFont((uint8_t *) Dialog_plain_22);
   display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}
// Connect to Wifi
void setup_wifi() {
  //Turn off Access Point
  WiFi.mode(WIFI_STA);
  delay(10);
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
// Check for incoming messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("");
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  display.clear();
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  char s[20];
  
  sprintf(s, "%s", payload);

    if ( (strcmp(topic,"home/switch/332/selector/set")==0))
  {
    payload[length] = '\0';
    String sp = String((char*)payload);

  // Alarm is off
    if (sp == "0")
    {
    Serial.println();
    Serial.print("Alarm is set to Disarmed");
    Serial.println();
    display.drawString(display.getWidth()/2, display.getHeight()/2, "Alarme\n Désactivée");
    //display.drawString(0, 0, "Désactivé");
  display.display();
  }   
  // Alarm is Armed Home    
    else if (sp == "10")
    {
    Serial.println();
    Serial.print("Alarm is set to Armed Home");
    Serial.println(); 
    display.drawString(display.getWidth()/2, display.getHeight()/2, "Activé\n -- Garage --");
  display.display();
  } 
  // Alarm is arming
      else if (sp == "1")
    {
    Serial.println();
    Serial.print("Alarm set to Pending");
    Serial.println();  
    display.drawString(display.getWidth()/2, display.getHeight()/2, "...Activation...");
  display.display();
  }   
  // Alarm is Armed Away
    else if (sp == "20")
    {
    Serial.println();
    Serial.print("Alarm set to Armed Away");
    Serial.println();  
    display.drawString(display.getWidth()/2, display.getHeight()/2, "Activé\n -- Total --");
  display.display();
  } 
  // Alarm is Triggered
    else if (sp == "triggered")
    {
    Serial.println();
    Serial.print("Alarm is triggered!!");
    Serial.println(); 
  }
}}
/* interpret the ascii digits in[0] and in[1] as hex
* notation and convert to an integer 0..255.
*/
int hex8(byte *in)
{
   char c, h;

   c = (char)in[0];

   if (c <= '9' && c >= '0') {  c -= '0'; }
   else if (c <= 'f' && c >= 'a') { c -= ('a' - 0x0a); }
   else if (c <= 'F' && c >= 'A') { c -= ('A' - 0x0a); }
   else return(-1);

   h = c;

   c = (char)in[1];

   if (c <= '9' && c >= '0') {  c -= '0'; }
   else if (c <= 'f' && c >= 'a') { c -= ('a' - 0x0a); }
   else if (c <= 'F' && c >= 'A') { c -= ('A' - 0x0a); }
   else return(-1);

   return ( h<<4 | c);
}
// Reconnect to wifi if connection lost
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientID, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("home/rfid", "connected");
      // ... and resubscribe
      client.subscribe("home/switch/332/selector/set");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
// Main functions
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  // Show some details of the PICC (that is: the tag/card)
  Serial.println("");
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  display.clear();
  display.drawString(display.getWidth()/2, display.getHeight()/2, "Traitement\n en cours");
  display.display();
  
  //digitalWrite(idPin, HIGH);
  delay(500);
  //digitalWrite(idPin, LOW);
  delay(500);
  // Send data to MQTT
  String rfidUid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidUid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    rfidUid += String(mfrc522.uid.uidByte[i], HEX);
  }
  const char* id = rfidUid.c_str();
  client.publish("home/rfid", id);
  delay(3000);
}
// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
