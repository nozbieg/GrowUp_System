#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "Adafruit_LiquidCrystal.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <PubSubClient.h>

//time

long now = millis();
long lastMeasure = 0;
long lastMeasure1 = 0;
//.

// lcd  setup
const int rs=D8, en=D9, db4=D4, db5=D5, db6=D6, db7=D7;
const int LCD_COLS = 16;
const int LCD_ROWS = 2;
Adafruit_LiquidCrystal  lcd(rs, en, db4, db5, db6, db7);
//--
//local wifi setup
const char* ssid = "TP-LINK_B6CD";
const char* password = "78703896";
BearSSL::CertStore certStore;
WiFiClient espClient;
//--

//mqtt server
const char* mqtt_server = "broker.hivemq.com";
PubSubClient mqtt_client(espClient);
//--

//Multiplekset

//.

//DHT temp, humid
#define DHTPIN D13
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
float temperature = 0.0;
float humidity = 0.0;
//.
//sensor wilgotnosci gleby
char mois[50];
//--
//sensor wilgotnosci gleby
char wlvl[50];
//--
char temperature_msg[50];
char humidity_msg[50];


void changeMux(int c, int b, int a) {
  digitalWrite(D12, a);
  digitalWrite(D11, b);
  digitalWrite(D10, c);
}
void printWiFiInfo(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi connected");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP address: ");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP()); 
}

void printWiFi_Connection(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to: ");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
}

void printReadyMessage(){
  lcd.clear();
  lcd.print("GrowUp System");
  lcd.setCursor(0,1);
  lcd.print("Ready");
}

void setup_wifi() {
  delay(100);
  // We start by connecting to a WiFi network
  printWiFiInfo();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
   {
    delay(500);
   }
  randomSeed(micros());
  printWiFiInfo();
}

void turnonWaterPump(int time){
    digitalWrite(D15, HIGH);
    delay(time*1000);
    digitalWrite(D15, LOW);
}

void mqtt_reconnect() {
  // Loop until we’re reconnected
  delay(1000);
  while (!mqtt_client.connected()) {
    lcd.clear();
    lcd.print("MQTT connection…");
    delay(500);
    String clientId = "GrowUpSystemExecutor";
    // Attempt to connect
    // Insert your password
    if (mqtt_client.connect(clientId.c_str())) {
      lcd.setCursor(0,1);
      lcd.print("connected");
      delay(500);
      // … and resubscribe
      mqtt_client.subscribe("GrowUp_System_BaseTopic");
      mqtt_client.subscribe("GrowUp_System_WaterPumpOn");
    } else {
      lcd.clear();
      delay(500);
      lcd.setCursor(0,0);
      lcd.print("failed, rc = ");
      lcd.setCursor(0,1);
      lcd.print("try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
lcd.clear();
lcd.print("MQTT Connected");
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  lcd.clear();
  lcd.print("Message arrived");
  lcd.setCursor(0,1);
  lcd.print(topic);
  String message ="";
  if (strcmp(topic,"GrowUp_System_WaterPumpOn")==0){
    turnonWaterPump(5);
  }

  for (int i = 0; i < length; i++) {
   message += (char)payload[i];
  }
  delay(1000);
  lcd.clear();
  lcd.print(message);
}


int readWilgotnoscA0(){
changeMux(LOW, HIGH, LOW); //chanel 2 MUX
 int value = analogRead(A0);
 value= value/10;

 //MQTT SEND
 String mois_str = String(value);
 mois_str.toCharArray(mois, mois_str.length() +1);
 mqtt_client.publish("GrowUp_System_Moisture_1", mois);
 delay(500);
 return value;
}
int readPoziomWodyA0(){
changeMux(LOW, LOW, HIGH); //chanel 1 MUX
int value = analogRead(A0);
value= value/10;

//MQTT SEND
String wlvl_str = String(value);
wlvl_str.toCharArray(wlvl, wlvl_str.length() +1);
mqtt_client.publish("GrowUp_System_WaterLevel_1", wlvl);
delay(500);
return value;
}

void updateLcd(String name1, int moisValue, String name2, int waterValue){
  lcd.clear();
  lcd.setCursor(0,0);
  Serial.print(name1);
  Serial.println(moisValue);
  Serial.print(name2);
  Serial.println(waterValue);
  lcd.print(name1);
  lcd.print(moisValue);
  lcd.setCursor(0,1);
  lcd.print(name2);
  lcd.print(waterValue);
  delay(2000);
}

void readTemperature(){
  float newT = dht.readTemperature(false);
  if(isnan(newT)){
  Serial.println("Nan... TEMP");
  }else {
    Serial.print("Temp:  ");
    Serial.println(newT);
 temperature = newT;
String temp_str = String(temperature);
temp_str.toCharArray(temperature_msg, temp_str.length() +1);
 mqtt_client.publish("GrowUp_System_Temperature", temperature_msg);
 }
  delay(500);
}

void readHumidity(){
   float newH = dht.readHumidity();
   if(isnan(newH)){
  Serial.println("Nan... HUM");
  }else {
    Serial.print("Humm: ");
    Serial.println(newH);
    humidity = newH;

    String hum_str = String(humidity);
hum_str.toCharArray(humidity_msg, hum_str.length() +1);
 mqtt_client.publish("GrowUp_System_Humidity", humidity_msg);
 }
}
void setup() {
  Serial.begin(9600);
  Serial.println("Hello world");
  //Piny multiplekser
  pinMode(D12, OUTPUT);
  pinMode(D11, OUTPUT);     
  pinMode(D10, OUTPUT);  
  pinMode(D15, OUTPUT);
  //..
  dht.begin();

  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.print("Witaj GrovUp System");
  delay(1000);
  setup_wifi();
  delay(1000);

  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(mqtt_callback);
  mqtt_reconnect();
  delay(1000);
  printReadyMessage();
}

void loop() 
{
  
  now = millis();
  mqtt_client.loop(); 
  if (now - lastMeasure1 > 10000) {
    lastMeasure1 = now;
    readTemperature();
    readHumidity();  
    updateLcd("temp: ",temperature, "wilg: ", humidity);
  }
  if (now - lastMeasure > 15000) 
  {
    lastMeasure = now;
    int moisValue = readWilgotnoscA0();
    delay(10);
    int waterValue = readPoziomWodyA0();
    updateLcd("wilg Gleb: ",moisValue, "woda lvl: ", waterValue);
 
  }
  
  delay(500);
}