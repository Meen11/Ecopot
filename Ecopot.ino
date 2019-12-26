#include <FirebaseESP32.h>
#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <ErriezBH1750.h>

const char* ssid = "Pimmmmm";
const char* password = "pimcutemak";

FirebaseData firebaseData;

// Initialize DHT sensor.
#define DHTTYPE DHT11   // DHT 11
const int DHTPin = 23;
DHT dht(DHTPin, DHTTYPE);


// Initialize Lux sensor.
BH1750 sensor(LOW);
float lumi = 0;

// Initialize Soil Moisture sensor.
const int moisturePin = A0;
int mois = 0;

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
  

  if (Firebase.getFloat(firebaseData, "/temperature")) {
    if (firebaseData.dataType() == "float") {
      int val = firebaseData.floatData();
      Serial.print("temp: ");
      Serial.println(val);
    }
  }
  if (Firebase.getFloat(firebaseData, "/humidity")) {
    if (firebaseData.dataType() == "float") {
      int val = firebaseData.floatData();
      Serial.print("humi: ");
      Serial.println(val);
    }
  }
  if (Firebase.getFloat(firebaseData, "/luminance")) {
    if (firebaseData.dataType() == "float") {
      int val = firebaseData.floatData();
      Serial.print("lumi: ");
      Serial.println(val);
    }
  }
  if (Firebase.getFloat(firebaseData, "/moisture")) {
    if (firebaseData.dataType() == "float") {
      int val = firebaseData.floatData();
      Serial.print("mois: ");
      Serial.println(val);
    }
  }
  
  delay(5000);
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
  Firebase.setFloat(firebaseData, "/humidity", humi);
  Firebase.setFloat(firebaseData, "/temperature", temp);
  Firebase.setFloat(firebaseData, "/luminance", lumi);
  Firebase.setFloat(firebaseData, "/moisture", mois);
}
