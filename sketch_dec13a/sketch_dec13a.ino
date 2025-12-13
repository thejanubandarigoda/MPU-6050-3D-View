#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Arduino_JSON.h> // Arduino_JSON library එක install කරන්න

// WiFi විස්තර මෙතැනට දාන්න
const char* ssid = "LTE";
const char* password = "123456789";

// MPU6050 Setup
Adafruit_MPU6050 mpu;

// Web Server සහ WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

JSONVar readings;

void initMPU() {
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) { delay(10); }
  }
  Serial.println("MPU6050 Found!");
  
  // Sensor settings
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

String getSensorReadings() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // JSON object එකක් හදනවා
  readings["x"] = String(a.acceleration.x);
  readings["y"] = String(a.acceleration.y);
  readings["z"] = String(a.acceleration.z);
  
  // Gyro data (optional - අවශ්‍ය නම් මේවාත් යවන්න පුළුවන්)
  readings["gx"] = String(g.gyro.x);
  readings["gy"] = String(g.gyro.y);
  readings["gz"] = String(g.gyro.z);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

void setup() {
  Serial.begin(115200);
  initMPU();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println(WiFi.localIP()); // මේ IP එක copy කරගන්න

  // Start WebSocket
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.begin();
}

void loop() {
  // දිගටම data යවනවා WebSocket හරහා
  ws.cleanupClients();
  String sensorData = getSensorReadings();
  ws.textAll(sensorData);
  delay(50); // Data යවන වේගය (50ms)
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("Web Client Connected");
  }
}
