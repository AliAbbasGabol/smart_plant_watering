#include "DHT.h"
#include "PubSubClient.h"
#include "WiFi.h"
#include <ArduinoJson.h>
#include "Adafruit_Sensor.h"
#include "DHT_U.h"

#define DHTPIN 4 
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define RAIN_PIN 39
#define SENSOR 27


const int soilMoistureThreshold = 25; // Soil moisture threshold in percentage
const unsigned long soilMoistureDuration = 180000; // 3 minutes in milliseconds

// Variables



unsigned long soilMoistureLowStartTime;
bool soilMoistureLowFlag = false;



const int pumpPin = 2;
const int pumpOnDuration = 3000;
const int pumpOffDelay = 6000;
const int interval = 1000;
const float calibrationFactor = 7.5;
volatile unsigned long pulseCount = 0;
unsigned long pulse1Sec = 0;
float flowRate = 0;
unsigned long flowMilliLitres = 0;
unsigned long totalMilliLitres = 0;
long currentMillis = 0;
long previousMillis = 0;
long lastResetMillis = 0;
long lastPumpOffTime = 0;
long pumpOnTime = 0;

float temperature_sum = 24;
float humidity_sum = 50;
int reading_count = 1;
long last_average_calculation_time = 0;
long last_weather_update_time = 0; // Added variable to track the last weather update time

float Kp = 0.7;
float Eto = 4.5;
float Area_covered_by_plant = 0.5;
float Etc = Eto * Kp;
float Etc_monthly = Etc * 30;
float daily_water = (Etc_monthly - 55) / 30;
float water_req = Etc * Area_covered_by_plant * 1000;
float average_temperature = 0;
float average_humidity = 0;
float regional_avg_temp = 24.6;
float regional_avg_hum = 45;

const char* ssid = "Fake";                
const char* wifi_password = "Smart1234";
const char* mqtt_server = "192.168.43.21";
const char* humidity_topic = "humidity";
const char* temperature_topic = "temperature";
const char* moisture_topic = "moisture";
const char* raindrop_topic = "raindrop";
const char* flow_rate_topic = "flow_rate";
const char* plant_common_name_topic = "plant/common_name";
const char* plant_water_needs_topic = "plant/water_needs";
const char* plant_plant_constant_topic = "plant/plant_constant";
const char* mqtt_username = "neo";
const char* mqtt_password = "eglabs";
const char* clientID = "Weather_Reporter";
WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient);
const char* weather_rain_check_topic = "weather/rain_check";
const int LED_PIN = 2; // Define the LED pin explicitly

void connect_MQTT() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print("Connecting to ");
        Serial.println(ssid);
        WiFi.begin(ssid, wifi_password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }

    if (!client.connected()) {
        if (client.connect(clientID, mqtt_username, mqtt_password)) {
            Serial.println("Connected to MQTT Broker!");
            client.subscribe(plant_common_name_topic);
            client.subscribe(plant_water_needs_topic);
            client.subscribe(plant_plant_constant_topic);
            client.subscribe(weather_rain_check_topic);
        } else {
            Serial.println("Connection to MQTT Broker failed…");
        }
    }
}

void IRAM_ATTR pulseCounter() {
    pulseCount++;
}

void setup() {
    Serial.begin(115200);
    dht.begin();
    pinMode(LED_PIN, OUTPUT); // Use the defined LED pin
    pinMode(SENSOR, INPUT_PULLUP);
    pinMode(pumpPin, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
    connect_MQTT();
    client.setCallback(callback);
}

bool isCommonNameReceived = false;
bool isWaterNeedsReceived = false;
bool isPlantConstantReceived = false;

void callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0'; // Null-terminate the string

    if (strcmp(topic, plant_common_name_topic) == 0) {
        Serial.print("Received plant common name: ");
        Serial.println((char*)payload);
        isCommonNameReceived = true;
    } else if (strcmp(topic, plant_water_needs_topic) == 0) {
        water_req = atof((char*)payload);
        Serial.print("Received water needs: ");
        Serial.println(water_req);
        isWaterNeedsReceived = true;
    } else if (strcmp(topic, plant_plant_constant_topic) == 0) {
        Kp = atof((char*)payload);
        Serial.print("Received plant constant: ");
        Serial.println(Kp);
        isPlantConstantReceived = true;
        // Recalculate the water requirements based on the new Kp
        Etc = Eto * Kp;
        Etc_monthly = Etc * 30;
        daily_water = (Etc_monthly - 55) / 30;
        water_req = Etc * Area_covered_by_plant * 1000;
    } else if (strcmp(topic, weather_rain_check_topic) == 0) {
        float weather_data = atof((char*)payload);
        // Print the received weather data
        Serial.print("Received weather data - Rain Check: ");
        Serial.println(weather_data);
    }
}

void loop() {
    client.loop(); // Process MQTT messages

    if (!isCommonNameReceived || !isWaterNeedsReceived || !isPlantConstantReceived) {
        // Wait until all necessary data is received
        delay(1000); // Wait for 1 second before checking again
        return;
    }

    currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        pulse1Sec = pulseCount;
        pulseCount = 0;
        flowRate = ((1000.0 / interval) * pulse1Sec) / calibrationFactor;
        flowMilliLitres = (flowRate / 60) * 1000;
        totalMilliLitres += flowMilliLitres;

        if (client.publish(flow_rate_topic, String(flowRate).c_str())) {
            Serial.println("Flow rate sent!");
        } else {
            Serial.println("Flow rate failed to send. Reconnecting to MQTT Broker and trying again.");
            client.connect(clientID, mqtt_username, mqtt_password);
            delay(10);
            client.publish(flow_rate_topic, String(flowRate).c_str());
        }

        if (currentMillis - lastResetMillis >= 86400000) {
            totalMilliLitres = 0;
            lastResetMillis = currentMillis;
        }
    }

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int m = analogRead(36);
    int raindropValue = analogRead(RAIN_PIN);
    int rain_calibrated = map(raindropValue, 4096, 0, 0, 100);
    float soil_humidity_calibrated = map(m, 2900, 0, 0, 100);

    temperature_sum += t;
    humidity_sum += h;
    reading_count++;
    average_temperature = temperature_sum / reading_count;
    average_humidity = humidity_sum / reading_count;

    float temperature_change_percentage = (average_temperature - regional_avg_temp) / regional_avg_temp;
    float humidity_change_percentage = (average_humidity - regional_avg_hum) / regional_avg_hum;

    float temperature_adjustment = 1 + temperature_change_percentage;
    float humidity_adjustment = 1 - humidity_change_percentage;
    float overall_adjustment_factor = humidity_adjustment * temperature_adjustment;
    float new_water_amount = water_req * overall_adjustment_factor;

if (soil_humidity_calibrated < soilMoistureThreshold) {
    if (!soilMoistureLowFlag) {
        soilMoistureLowFlag = true;
        soilMoistureLowStartTime = currentMillis;
    } else if (currentMillis - soilMoistureLowStartTime >= soilMoistureDuration) {
        // Soil moisture has been below the threshold for more than 3 minutes
        Serial.println("Soil moisture has been low for more than 3 minutes");
    }
} else {
    soilMoistureLowFlag = false; // Reset the flag if soil moisture is not low
}

if ((soilMoistureLowFlag && currentMillis - soilMoistureLowStartTime >= soilMoistureDuration) || totalMilliLitres < new_water_amount && (rain_calibrated < 80 ) && (weather_data <1 )) {
    // Pump logic
    if (currentMillis - lastPumpOffTime >= pumpOffDelay) {
        digitalWrite(pumpPin, HIGH);
        Serial.println("Pump is ON");
        pumpOnTime = currentMillis;

        while (millis() - pumpOnTime < pumpOnDuration) {
            // Keep the pump on
        }

        digitalWrite(pumpPin, LOW);
        Serial.println("Pump is OFF");
        lastPumpOffTime = currentMillis;
    }
}


      Serial.println(totalMilliLitres);
      Serial.println(new_water_amount);
    if (currentMillis - last_average_calculation_time >= 60000) {
        temperature_sum = dht.readTemperature();
        humidity_sum = dht.readHumidity();
        reading_count = 1;
        last_average_calculation_time = currentMillis;
    }

    if (currentMillis - last_weather_update_time >= 3600000) { // Check if an hour has passed
        last_weather_update_time = currentMillis;
        // Code to update weather data
        float weather_data = 0; // Placeholder for actual weather data retrieval
        Serial.print("Updated weather data - Rain Check: ");
        Serial.println(weather_data);
    }

    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println(" %");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
    Serial.print("Soil Moisture: ");
    Serial.println(soil_humidity_calibrated);
    Serial.print("Raindrop Sensor: ");
    Serial.println(rain_calibrated);

    if (client.publish(temperature_topic, String(t).c_str())) {
        Serial.println("Temperature sent!");
    } else {
        Serial.println("Temperature failed to send. Reconnecting to MQTT Broker and trying again.");
        client.connect(clientID, mqtt_username, mqtt_password);
        delay(10);
        client.publish(temperature_topic, String(t).c_str());
    }

    if (client.publish(humidity_topic, String(h).c_str())) {
        Serial.println("Humidity sent!");
    } else {
        Serial.println("Humidity failed to send. Reconnecting to MQTT Broker and trying again.");
        client.connect(clientID, mqtt_username, mqtt_password);
        delay(10);
        client.publish(humidity_topic, String(h).c_str());
    }

    if (client.publish(moisture_topic, String(soil_humidity_calibrated).c_str())) {
        Serial.println("Soil Moisture sent!");
    } else {
        Serial.println("Soil Moisture failed to send. Reconnecting to MQTT Broker and trying again.");
        client.connect(clientID, mqtt_username, mqtt_password);
        delay(10);
        client.publish(moisture_topic, String(soil_humidity_calibrated).c_str());
    }

    if (client.publish(raindrop_topic, String(rain_calibrated).c_str())) {
        Serial.println("Raindrop Sensor sent!");
    } else {
        Serial.println("Raindrop Sensor failed to send. Reconnecting to MQTT Broker and trying again.");
        client.connect(clientID, mqtt_username, mqtt_password);
    }
    
    delay(1000); // Add a delay to prevent flooding the MQTT broker
}
