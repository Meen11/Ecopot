// SP2019-37
#include <FirebaseESP32.h>
#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <ErriezBH1750.h>

const char* ssid = "Meen";
const char* password = "donotuseme";

FirebaseData firebaseData;

// Declaration of DHT sensor.
#define DHTTYPE DHT11   // DHT 11
const int DHTPin = 23;
DHT dht(DHTPin, DHTTYPE);

// Declaration of Lux sensor.
BH1750 sensor(LOW);
float lumi = 0;

// Declaration of Soil Moisture sensor.
const int moisturePin = A0;
int mois = 0;

// Declaration for Pot Controller
bool led_controller = false;
bool water_controller = false;
bool psb_controller = false;
const int led_pin = 14;
const int water_pin = 12;
const int psb_pin = 13;

void setup() {
  dht.begin();
  Serial.begin(9600);
  connectWifi();
  Firebase.begin("https://ecopot-c09bb.firebaseio.com/", "6d5e3esmi2ihZi7SaKdRCP22pU2MEZbEZZwQiuae");

  // Initialize I2C bus
  Wire.begin();

  // Initialize sensor in continues mode, high 0.5 lx resolution
  sensor.begin(ModeContinuous, ResolutionHigh);

  // Start conversion
  sensor.startConversion();

  pinMode(led_pin, OUTPUT);
  pinMode(psb_pin, OUTPUT);
  pinMode(water_pin, OUTPUT);
}

void loop() {
 
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humi = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temp = dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(humi) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }   

  uint16_t lux;
  // Wait for completion (blocking busy-wait delay)
  if (sensor.isConversionCompleted()) {
    // Read light
    lux = sensor.read();
    lumi = lux;
  }

  mois = analogRead(moisturePin);
  mois = map(mois,4095,1000,0,100);
  
  writeToFirebase(humi, temp, lumi, mois);

  potControllerOpt();

  delay(0);
}


void connectWifi() {
  // Let us connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(".......");
  Serial.println("WiFi Connected....IP Address:");
  Serial.println(WiFi.localIP());
}

void writeToFirebase(float humi, float temp, float lumi, float mois) {
  Firebase.setFloat(firebaseData, "/ECOPOT/HUMIDITY", humi);
  Firebase.setFloat(firebaseData, "/ECOPOT/TEMPERATURE", temp);
  Firebase.setFloat(firebaseData, "/ECOPOT/LUMINANCE", lumi);
  Firebase.setFloat(firebaseData, "/ECOPOT/MOISTURE", mois);
}

void potControllerOpt(){
  if (Firebase.getBool(firebaseData, "/ECOPOT/POT_CONTROL/LED")) {
    if (firebaseData.dataType() == "boolean") {
      bool val = firebaseData.boolData();
      Serial.print("LED: ");
      Serial.println(val);
      led_controller = val;
      
      if(led_controller) digitalWrite(led_pin, HIGH);
      else digitalWrite(led_pin, LOW);
    }
  } 
  if (Firebase.getBool(firebaseData, "/ECOPOT/POT_CONTROL/PSB")) {
    if (firebaseData.dataType() == "boolean") {
      bool val = firebaseData.boolData();
      Serial.print("PSB: ");
      Serial.println(val);
      psb_controller = val;

      if(psb_controller) digitalWrite(psb_pin, HIGH);
      else digitalWrite(psb_pin, LOW);
    }
  } 
  if (Firebase.getBool(firebaseData, "/ECOPOT/POT_CONTROL/WATER")) {
    if (firebaseData.dataType() == "boolean") {
      bool val = firebaseData.boolData();
      Serial.print("WATER: ");
      Serial.println(val);
      water_controller = val;

      if(water_controller) digitalWrite(water_pin, HIGH);
      else digitalWrite(water_pin, LOW);
    }
  } 
}
