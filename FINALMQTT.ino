#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>

#include <DHT_U.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

#define DHTTYPE DHT22
#define DHTPIN 0 
DHT dht(DHTPIN, DHTTYPE);
#define LUM A0
unsigned long readTime;

//Wifi 
const char* ssid = "yassinewifi";
const char* password = "codewifi";
const char* mqtt_server = "192.168.43.74";
int keyIndex = 0;            // your network key Index number (needed only for WEP)


int status = WL_IDLE_STATUS;
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, password);

    // wait 5 seconds for connection:
    delay(5000);
  }
  
  Serial.println("Connected to wifi");
  Serial.print("Adress IP :");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
  
  client.publish("test", "Connexion établie");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
    Serial.println();
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("test", "Connexion établi");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


// GET luminosity
void callback_lum(){
  
  char reponse[50];
  int l = analogRead(LUM);

  if (isnan(l)) {
    Serial.println("Failed to read from light sensor!");
    sprintf(reponse, "erreur lecture capteur");
    client.publish("luminosite", reponse);
    return;
  }
   
  Serial.println(l);
  if(l>500){  
    sprintf(reponse, "Jour");
  }
  else if(l>200){
    sprintf(reponse, "Soir");
  }
  else {
    sprintf(reponse, "Nuit");
  }

  //client.publish("luminosite", reponse);
  
   sprintf(reponse, "%d", l);
   client.publish("luminosite", reponse);
  
}


// GET humidity
void callback_hum(){
  
  char reponse[50];
  char char_h;
  
  float h = dht.readHumidity();

  if (isnan(h)) {
    Serial.println("Failed to read humidity from DHT sensor!");
    sprintf(reponse, "erreur lecture capteur");
    client.publish("humidite", reponse);
    return;
  }
  
  Serial.println(h);
  //dtostrf(h, 2, 0, char_h);
  
  sprintf(reponse, "%d", (int) h);
  client.publish("humidite", reponse);
  
}

// GET temperature
void callback_temp(){
  char reponse[50];
  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println("Failed to read temperature from DHT sensor!");
    client.publish("temperature", reponse);
    return;
  }
  
  Serial.println(t);
  //dtostrf(t, 2, 2, char_t);
  //dtostrf(h, 2, 0, char_h);  
  sprintf(reponse, "%d" , (int) t);

  client.publish("temperature", reponse);

}

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, password);

    // wait 5 seconds for connection:
    delay(5000);
  }
  
  Serial.println("Connected to wifi");
  Serial.print("Adress IP :");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
   dht.begin();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    callback_lum();
    callback_hum();
    callback_temp();
  }
}
