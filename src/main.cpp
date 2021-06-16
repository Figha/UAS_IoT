#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#define relay1 D6
#define relay2 D7

const char *ssid = "Babadan";    //silakan disesuaikan sendiri
const char *password = "gwencantik"; //silakan disesuaikan sendiri

const char *mqtt_server = "ec2-3-81-235-104.compute-1.amazonaws.com";

WiFiClient espClient;
PubSubClient client(espClient);

SimpleDHT11 dht11(D1);
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  lcd.print("Connecting...");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messageTemp;
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();
  if (String(topic) == "control/lcd")
  {
    if (messageTemp == "Nyala")
    {
      Serial.println("Nyala");
      lcd.backlight();
    }
    else
    {
      Serial.println("Mati");
      lcd.noBacklight();
    }
  }
  else if (String(topic) == "control/door")
  {
    if (messageTemp == "Terkunci")
    {
      Serial.println("Terkunci");
      digitalWrite(relay1, HIGH);
    }
    else
    {
      Serial.println("Terbuka");
      digitalWrite(relay1, LOW);
    }
  }
  else if (String(topic) == "control/lamp")
  {
    if (messageTemp == "Nyala")
    {
      Serial.println("Nyala");
      digitalWrite(relay2, LOW);
    }
    else
    {
      Serial.println("Mati");
      digitalWrite(relay2, HIGH);
    }
  }
}
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    // String clientId = "ESP8266Client-";
    // clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect("ESP8266Client","endg","figha210770"))
    {
      Serial.println("connected");
      client.subscribe("control/lamp");
      client.subscribe("control/door");
      client.subscribe("control/lcd");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup()
{
  Wire.begin(2,00);
  lcd.init();
  lcd.backlight();
  lcd.home();

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  
}
void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;
    ++value;
    int err = SimpleDHTErrSuccess;
    byte temperature = 0;
    byte humidity = 0;
    if ((err = dht11.read(&temperature, &humidity, NULL)) !=
        SimpleDHTErrSuccess)
    {
      Serial.print("Pembacaan DHT11 gagal, err=");
      Serial.println(err);
      delay(500);
      return;
    }
    static char temperatureTemp[7];
    static char humTemp[8];
    dtostrf(temperature, 4, 2, temperatureTemp);
    dtostrf(humidity, 4, 2, humTemp);
    Serial.println(temperatureTemp);
    Serial.println(humTemp);
    client.publish("monitoring/suhu", temperatureTemp);
    client.publish("monitoring/humadity", humTemp);
  }
  lcd.setCursor(0,0);
  lcd.print("Connected");
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());
  delay(500);
  lcd.clear();
}
