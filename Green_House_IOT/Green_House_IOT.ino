#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <ArduinoJson.h>

// Cấu hình Wi-Fi
const char* ssid = "Hoag_bui";      //"BMDT_Teacher"; "Hoag_bui"; "My Home 3 sau";
const char* password = "77777777";  //"PTN@203B3!@#"; "77777777";  "phongtromyhome.vn";

// Cấu hình MQTT
const char* mqtt_server = "54.251.103.14";  // Địa chỉ IP MQTT Broker
const int mqtt_port = 1883;                 // Cổng MQTT
const char* mqtt_user = "thinh";            // Tên đăng nhập MQTT
const char* mqtt_password = "thinh";        // Mật khẩu MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Cấu hình DHT
#define DHTPIN 25
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Cấu hình ADC
#define LIGHT_SENSOR_PIN 34
#define SOIL_SENSOR_PIN 35
#define POTENTIOMETER_PIN 32
// Các nút nhấn
#define BUTTON_SELECT_PIN 2
#define BUTTON_INC_PIN 15
#define BUTTON_DEC_PIN 4
// Cấu hình OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// I2C configuration for ESP32
#define I2C_SDA 21
#define I2C_SCL 22

// GPIO điều khiển các thiết bị
#define TRIAC_PIN 27  // Chân điều khiển TRIAC (thay cho LIGHT_PIN)
#define BL_FAN_PIN 5
#define EXH_FAN_PIN 18
#define IRR_PUMP_PIN 19
#define MIST_PUMP_PIN 17
#define ZERO_CROSS_PIN 14  // Chân nhận tín hiệu Zero Cross

// Các biến chứa dữ liệu cảm biến
float lightLevel, soilMoisture, temperature, humidity, potentiometerValue;
float thresholdLight, thresholdSoilMoisture, thresholdTemperature, thresholdHumidity;  // Ngưỡng điều khiển thiết bị
// Biến cho dimmer
volatile bool zeroCrossDetected = false;  // Biến cờ để đánh dấu Zero Cross
volatile int dimmingValue = 128;          // Giá trị điều chỉnh dimming từ 0-255

// ISR cho Zero Cross
void IRAM_ATTR zeroCrossISR() {
  zeroCrossDetected = true;
}

// Task: Đọc cảm biến DHT11
// void readDHTTask(void* parameter) {
//   for (;;) {
//     if (!isnan(temperature) && !isnan(humidity)) {
//       // Process and send data
//       temperature = dht.readTemperature();
//       humidity = dht.readHumidity();
//     } else {
//       Serial.println("Failed to read from DHT sensor!");
//     }
//     vTaskDelay(2000 / portTICK_PERIOD_MS);  // Đọc mỗi 2 giây
//   }
// }
void readDHTTask(void* parameter) {
  for (;;) {
    // Đọc giá trị nhiệt độ và độ ẩm từ cảm biến DHT
    float newTemperature = dht.readTemperature();
    float newHumidity = dht.readHumidity();
    Serial.print(" readTemperature Value: ");
    Serial.println(newTemperature);
    Serial.print(" readHumidity Value: ");
    Serial.println(newHumidity);
    // Kiểm tra nếu không phải NaN, cập nhật các giá trị
    if (!isnan(newTemperature) && !isnan(newHumidity)) {
      temperature = newTemperature;
      humidity = newHumidity;
      // Xử lý và gửi dữ liệu
    } else {
      Serial.println("Failed to read from DHT sensor!");
    }

    // Đọc mỗi 2 giây
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}
// Task: Đọc cảm biến ánh sáng và độ ẩm đất
void readSensorsTask(void* parameter) {
  for (;;) {
    lightLevel = 255 * (analogRead(LIGHT_SENSOR_PIN) * 3.3 / 4095.0);  // Chuyển đổi giá trị ADC thành %
    // int adcValue = analogRead(LIGHT_SENSOR_PIN);  // Read ADC value (0-4095)
    // float voltage = adcValue * (3.3 / 4095.0);    // Convert ADC value to voltage
    // float current = voltage / 51000.0;            // Calculate current (I = V / R) with R = 51kΩ

    // // Assuming 1µA corresponds to 1 lux (example approximation, actual value from the datasheet or calibration may differ)
    // float lightLevel = current * 13.33 * pow(10, 6);  // Correct exponentiation

    Serial.print("Lux: ");
    Serial.println(lightLevel);
    //  soilMoisture = (-74)*log(((4095-analogRead(SOIL_SENSOR_PIN))*3.3 / 4095)+0.0001)+42 ;
    soilMoisture = ((4095 - analogRead(SOIL_SENSOR_PIN)) / 4095.0) * 100;
    vTaskDelay(5000 / portTICK_PERIOD_MS);  // Đọc mỗi 1 giây
  }
}
// void readAllSensorsTask(void* parameter) {
//   for (;;) {
//     // Đọc cảm biến DHT11
//     float temperature = dht.readTemperature();
//     float humidity = dht.readHumidity();

//     if (isnan(temperature) || isnan(humidity)) {
//       Serial.println("Failed to read from DHT sensor!");
//     } else {
//       Serial.print("Temperature: ");
//       Serial.print(temperature);
//       Serial.print(" °C, Humidity: ");
//       Serial.print(humidity);
//       Serial.println(" %");
//     }

//     // Đọc cảm biến ánh sáng
//     int lightLevel = 255 * (analogRead(LIGHT_SENSOR_PIN) * 3.3 / 4095.0);  // Chuyển đổi giá trị ADC thành %
//     Serial.print("Light Level: ");
//     Serial.println(lightLevel);

//     // Đọc cảm biến độ ẩm đất
//     float soilMoisture = ((4095 - analogRead(SOIL_SENSOR_PIN)) / 4095.0) * 100;
//     Serial.print("Soil Moisture: ");
//     Serial.print(soilMoisture);
//     Serial.println(" %");

//     // Tạm dừng 5 giây trước khi lặp lại chu kỳ đọc dữ liệu
//     vTaskDelay(1000 / portTICK_PERIOD_MS);
//   }
// }
// Task điều khiển đèn bằng TRIAC
void controlLightTask(void* parameter) {
  for (;;) {
    if (zeroCrossDetected) {
      zeroCrossDetected = false;

      // Đọc giá trị trung bình từ potentiometer
      potentiometerValue = analogRead(POTENTIOMETER_PIN);

      // Tính toán thời gian trễ dựa trên giá trị potentiometer
      int delayTime = map(potentiometerValue, 0, 4095, 1000, 7500);  // Điều chỉnh dải trễ

      Serial.print("Potentiometer Value: ");
      Serial.println(potentiometerValue);

      // Trễ trước khi kích hoạt TRIAC
      delayMicroseconds(delayTime);

      // Kích hoạt TRIAC
      digitalWrite(TRIAC_PIN, HIGH);
      delayMicroseconds(10);  // Đảm bảo TRIAC bật đủ lâu
      digitalWrite(TRIAC_PIN, LOW);
    }

    // Đảm bảo task không chiếm CPU quá nhiều
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

int getFilteredPotValue() {
  int total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(POTENTIOMETER_PIN);
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Delay nhỏ giữa các lần đọc
  }
  return total / 10;  // Giá trị trung bình
}


// Task: Hiển thị dữ liệu lên OLED
void displayTask(void* parameter) {
  for (;;) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);

    // Display sensor data
    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");

    display.print("Humidity: ");
    display.print(humidity);
    display.println(" %");

    display.print("Light: ");
    display.print(lightLevel);
    display.println(" lux");

    display.print("Soil Moisture: ");
    display.print(soilMoisture);
    display.println(" %");

    // Display threshold values
    display.print("ThresTemp: ");
    display.print(thresholdTemperature);
    display.println(" C");

    display.print("ThresHumid: ");
    display.print(thresholdHumidity);
    display.println(" %");

    display.print("ThresSoil: ");
    display.print(thresholdSoilMoisture);
    display.println(" %");

    // Display updated data
    display.display();

    // Delay for 1 second
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void deviceControlTask(void* pvParameters) {
  while (true) {
    // Control irrigation pump based on soil moisture
    if (soilMoisture < thresholdSoilMoisture) {
      digitalWrite(IRR_PUMP_PIN, LOW);
      Serial.println("Bơm nước bật!");
      client.publish("home/green_house/Pump_IRR/status", "ON", true);
    } else if (soilMoisture >= (thresholdSoilMoisture + 5)) {
      digitalWrite(IRR_PUMP_PIN, HIGH);
      Serial.println("Bơm nước tắt!");
      client.publish("home/green_house/Pump_IRR/status", "OFF", true);
    }

    // Control fan and pump based on temperature threshold
    if (temperature > thresholdTemperature) {
      digitalWrite(BL_FAN_PIN, LOW);
      // digitalWrite(MIST_PUMP_PIN, LOW);
      // Serial.println("Bơm phun sương bật!");
      // client.publish("home/green_house/Pump_mist/status", "ON", true);
      Serial.println("Quạt bật!");
      client.publish("home/green_house/Fan_blow/status", "ON", true);
    } else if (temperature <= (thresholdTemperature - 2)) {
      digitalWrite(BL_FAN_PIN, HIGH);
      // digitalWrite(MIST_PUMP_PIN, HIGH);
      Serial.println("Quạt tắt!");
      client.publish("home/green_house/Fan_blow/status", "OFF", true);
      // Serial.println("Bơm phun sương tắt!");
      // client.publish("home/green_house/Pump_mist/status", "OFF", true);
    }

    // Control mist pump and exh fan based on humidity threshold
    if (humidity < thresholdHumidity) {
      digitalWrite(MIST_PUMP_PIN, LOW);
      Serial.println("Máy tạo độ ẩm bật!");
      client.publish("home/green_house/Pump_mist/status", "ON", true);
    } else if (humidity >= (thresholdHumidity + 5)) {
      digitalWrite(MIST_PUMP_PIN, HIGH);
      Serial.println("Máy tạo độ ẩm tắt!");
      client.publish("home/green_house/Pump_mist/status", "OFF", true);
    }

    if (humidity > thresholdHumidity + 6) {
      digitalWrite(EXH_FAN_PIN, LOW);
      Serial.println("Quạt hút bật!");
      client.publish("home/green_house/FAN_EXH/status", "ON", true);
    } else if (humidity <= thresholdHumidity) {
      digitalWrite(EXH_FAN_PIN, HIGH);
      Serial.println("quạt hút tắt!");
      client.publish("home/green_house/FAN_EXH/status", "OFF", true);
    }

    // Delay for 1 second to reduce CPU load
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

void publishMQTTTask(void* parameter) {
  for (;;) {
    if (!client.connected()) {
      reconnectMQTT();
    }

    // Publish temperature
    String temperaturePayload = "{\"temperature\": " + String(temperature) + "}";
    if (!client.publish("home/green_house/temperature", temperaturePayload.c_str())) {
      Serial.println("Failed to publish temperature");
    }

    // Publish humidity
    String humidityPayload = "{\"humidity\": " + String(humidity) + "}";
    if (!client.publish("home/green_house/humidity", humidityPayload.c_str())) {
      Serial.println("Failed to publish humidity");
    }

    // Publish soil moisture
    String soilMoisturePayload = "{\"moisture_soil\": " + String(soilMoisture) + "}";
    if (!client.publish("home/green_house/moisture_soil", soilMoisturePayload.c_str())) {
      Serial.println("Failed to publish soil moisture");
    }

    // Publish light level
    String lightLevelPayload = "{\"light_level\": " + String(lightLevel) + "}";
    if (!client.publish("home/green_house/light_level", lightLevelPayload.c_str())) {
      Serial.println("Failed to publish light level");
    }

    // Publish threshold temperature
    String thresholdTemperaturePayload = "{\"threshold_temperature\": " + String(thresholdTemperature) + "}";
    if (!client.publish("home/green_house/threshold_temperature", thresholdTemperaturePayload.c_str())) {
      Serial.println("Failed to publish threshold temperature");
    }

    // Publish threshold humidity
    String thresholdHumidityPayload = "{\"threshold_humidity\": " + String(thresholdHumidity) + "}";
    if (!client.publish("home/green_house/threshold_humidity", thresholdHumidityPayload.c_str())) {
      Serial.println("Failed to publish threshold humidity");
    }

    // Publish threshold soil moisture
    String thresholdSoilMoisturePayload = "{\"threshold_soil_moisture\": " + String(thresholdSoilMoisture) + "}";
    if (!client.publish("home/green_house/threshold_soil_moisture", thresholdSoilMoisturePayload.c_str())) {
      Serial.println("Failed to publish threshold soil moisture");
    }

    vTaskDelay(5000 / portTICK_PERIOD_MS);  // Publish every 5 seconds
  }
}



// Task: Subscribe để nhận ngưỡng điều khiển từ MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Chuyển đổi message thành JSON object
  DynamicJsonDocument doc(1024);  // Kích thước của JSON Document, điều chỉnh nếu cần
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("Lỗi phân tích JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Xử lý ngưỡng nhiệt độ từ topic "home/green_house/threshold_temperature/set"
  if (String(topic) == "home/green_house/threshold_temperature/set") {
    if (doc.containsKey("threshold_temperature")) {
      thresholdTemperature = doc["threshold_temperature"].as<float>();
      Serial.print("Ngưỡng nhiệt độ cập nhật: ");
      Serial.println(thresholdTemperature);
    }
  }

  // Xử lý ngưỡng độ ẩm từ topic "home/green_house/threshold_humidity/set"
  else if (String(topic) == "home/green_house/threshold_humidity/set") {
    if (doc.containsKey("threshold_humidity")) {
      thresholdHumidity = doc["threshold_humidity"].as<float>();
      Serial.print("Ngưỡng độ ẩm cập nhật: ");
      Serial.println(thresholdHumidity);
    }
  }

  // Xử lý ngưỡng độ ẩm đất từ topic "home/green_house/threshold_soil_moisture/set"
  else if (String(topic) == "home/green_house/threshold_soil_moisture/set") {
    if (doc.containsKey("threshold_soil_moisture")) {
      thresholdSoilMoisture = doc["threshold_soil_moisture"].as<float>();
      Serial.print("Ngưỡng độ ẩm đất cập nhật: ");
      Serial.println(thresholdSoilMoisture);
    }
  }
}




// Kết nối lại MQTT nếu bị ngắt kết nối
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Đang kết nối tới MQTT...");

    // Thử kết nối
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("Kết nối thành công!");

      // Subscribe các topic ngưỡng điều khiển
      // Subscribe to the topics
      client.subscribe("home/green_house/threshold_temperature/set");
      client.subscribe("home/green_house/threshold_humidity/set");
      client.subscribe("home/green_house/threshold_soil_moisture/set");
      client.subscribe("home/green_house/threshold_light/set");
    } else {
      Serial.print("Kết nối thất bại, rc=");
      Serial.print(client.state());
      Serial.println(" Thử lại sau 5 giây...");

      // Chờ 5 giây trước khi thử lại
      delay(5000);
    }
  }
}
void reconnectWiFi() {
  // Hàm để kết nối lại Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);  // Đặt SSID và mật khẩu Wi-Fi ở đây
    unsigned long startAttemptTime = millis();
    const unsigned long wifiReconnectTimeout = 1000;  // Thời gian chờ tối đa để kết nối lại Wi-Fi (15 giây)

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiReconnectTimeout) {
      Serial.print(".");
      delay(500);  // Chờ 500ms trước khi thử lại
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to WiFi");
    } else {
      Serial.println("\nFailed to connect to WiFi.");
    }
  }
}
// NGẮT NÚT NHẤN
volatile int currentSelection = 0;  // Để lưu trữ lựa chọn hiện tại khi thay đổi
// volatile bool valueChanged = false; // Đánh dấu thay đổi ngưỡng

// void IRAM_ATTR handleSelectButton() {
//   currentSelection = (currentSelection + 1) % 3; // Chuyển qua 3 giá trị (0 đến 2)
//   valueChanged = true;  // Đánh dấu cần cập nhật màn hình
// }

// void IRAM_ATTR handleIncButton() {
//   valueChanged = true;
//   switch (currentSelection) {
//     case 0:
//       thresholdSoilMoisture = min(thresholdSoilMoisture + 2.0f, 100.0f);  // Tăng đến tối đa 100
//       break;
//     case 1:
//       thresholdTemperature = min(thresholdTemperature + 1.0f, 50.0f);  // Giới hạn nhiệt độ từ 0-50
//       break;
//     case 2:
//       thresholdHumidity = min(thresholdHumidity + 2.0f, 100.0f);
//       break;
//   }
// }

// void IRAM_ATTR handleDecButton() {
//   valueChanged = true;
//   switch (currentSelection) {
//     case 0:
//       thresholdSoilMoisture = max(thresholdSoilMoisture - 2.0f, 0.0f);  // Giảm đến tối thiểu 0
//       break;
//     case 1:
//       thresholdTemperature = max(thresholdTemperature - 1.0f, 0.0f);
//       break;
//     case 2:
//       thresholdHumidity = max(thresholdHumidity - 2.0f, 0.0f);
//       break;
//   }
// }
void setup() {
  // Cài đặt pin đầu vào và đầu ra
  pinMode(TRIAC_PIN, OUTPUT);
  pinMode(ZERO_CROSS_PIN, INPUT);
  pinMode(EXH_FAN_PIN, OUTPUT);
  pinMode(MIST_PUMP_PIN, OUTPUT);
  pinMode(IRR_PUMP_PIN, OUTPUT);
  pinMode(BL_FAN_PIN, OUTPUT);
  // Cấu hình các nút nhấn
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_INC_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DEC_PIN, INPUT_PULLUP);

  // attachInterrupt(digitalPinToInterrupt(BUTTON_SELECT_PIN), handleSelectButton, FALLING);
  // attachInterrupt(digitalPinToInterrupt(BUTTON_INC_PIN), handleIncButton, FALLING);
  // attachInterrupt(digitalPinToInterrupt(BUTTON_DEC_PIN), handleDecButton, FALLING);

  // Light triac
  pinMode(TRIAC_PIN, OUTPUT);
  pinMode(ZERO_CROSS_PIN, INPUT);
  // Gắn ISR cho Zero Cross
  attachInterrupt(digitalPinToInterrupt(ZERO_CROSS_PIN), zeroCrossISR, RISING);
  // Khởi tạo Serial, DHT, và các task FreeRTOS
  Serial.begin(115200);
  dht.begin();

  // Khởi tạo OLED
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;)
      ;
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED initialized");

  // Khởi tạo Wi-Fi
  display.println("WiFi Connecting...!");
  display.display();
  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();  // Lưu thời gian bắt đầu kết nối
  const unsigned long wifiTimeout = 10000;    // Thời gian chờ kết nối Wi-Fi (10 giây)

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
    delay(500);
    Serial.print(".");
    // display.println("WiFi Connecting...!");
  }  

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected!");
    display.println("WiFi Connected!");
  } else {
    Serial.println("WiFi không kết nối được, tiếp tục chạy các task.");
    display.println("Wifi Failed!");
  }

  // Khởi tạo MQTT chỉ khi có Wi-Fi
  if (WiFi.status() == WL_CONNECTED) {
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(mqttCallback);
  }
  // Subscribe to the topics
  client.subscribe("home/green_house/threshold_temperature/set");
  client.subscribe("home/green_house/threshold_humidity/set");
  client.subscribe("home/green_house/threshold_soil_moisture/set");
  client.subscribe("home/green_house/threshold_light/set");
  // Tạo các task FreeRTOS
  xTaskCreate(readDHTTask, "Read DHT", 4096, NULL, 1, NULL);
  xTaskCreate(readSensorsTask, "Read Sensors", 4096, NULL, 1, NULL);
  xTaskCreate(controlLightTask, "Control Light", 4096, NULL, 4, NULL);  // Task cho dimmer bằng TRIAC
  xTaskCreate(displayTask, "Display", 4096, NULL, 4, NULL);

  // Chỉ tạo task MQTT nếu Wi-Fi đã kết nối
  if (WiFi.status() == WL_CONNECTED) {
    xTaskCreate(publishMQTTTask, "Publish MQTT", 4096, NULL, 2, NULL);
  }

  xTaskCreate(deviceControlTask, "Device Control Task", 4096, NULL, 3, NULL);
}

// void loop() {
//  // Kiểm tra trạng thái kết nối Wi-Fi
//   if (WiFi.status() != WL_CONNECTED) {
//     Serial.println("WiFi not connected, attempting to reconnect...");
//     reconnectWiFi();  // Hàm kết nối lại Wi-Fi
//   } else {
//     // Nếu đã kết nối Wi-Fi, kiểm tra và kết nối MQTT nếu cần
//     if (!client.connected()) {
//       Serial.println("MQTT not connected, attempting to reconnect...");
//       reconnectMQTT();  // Hàm kết nối lại MQTT
//     }
//   }
//   client.loop();  // Lắng nghe MQTT

//   // Hiển thị các ngưỡng hiện tại
//   Serial.print("Ngưỡng nhiệt độ hiện tại: ");
//   Serial.println(thresholdTemperature);
//   Serial.print("Ngưỡng độ ẩm đất hiện tại: ");
//   Serial.println(thresholdSoilMoisture);
//   Serial.print("Ngưỡng độ ẩm kk hiện tại: ");
//   Serial.println(thresholdHumidity);

//   // Cập nhật màn hình OLED nếu có sự thay đổi
//   if (valueChanged) {
//     display.clearDisplay();
//     display.setCursor(0, 0);
//     display.print("Selected: ");
//     switch (currentSelection) {
//       case 0:
//         display.println("Soil Moisture");
//         display.print("Threshold Soil Moisture: ");
//         display.println(thresholdSoilMoisture);
//         break;
//       case 1:
//         display.println("Temperature");
//         display.print("Threshold Temperature: ");
//         display.println(thresholdTemperature);
//         break;
//       case 2:
//         display.println("Humidity");
//         display.print("Threshold Humidity: ");
//         display.println(thresholdHumidity);
//         break;
//     }
//     display.display();  // Hiển thị lên màn hình
//     valueChanged = false;  // Đặt lại cờ
//   }

//   vTaskDelay(1000 / portTICK_PERIOD_MS);  // Trì hoãn 1 giây để giảm tải CPU
// }

void loop() {

  // Kiểm tra trạng thái kết nối Wi-Fi
  // if (WiFi.status() != WL_CONNECTED) {
  //   Serial.println("WiFi not connected, attempting to reconnect...");
  //   reconnectWiFi();  // Hàm kết nối lại Wi-Fi
  // } else {
  // Nếu đã kết nối Wi-Fi, kiểm tra và kết nối MQTT nếu cần
  // if (!client.connected()) {
  //   Serial.println("MQTT not connected, attempting to reconnect...");
  //   reconnectMQTT();  // Hàm kết nối lại MQTT
  // }
  // }


  client.loop();  // Lắng nghe MQTT và nhận các thông điệp chỉ khi có kết nối Wi-Fi
  // } else {
  //   Serial.println("Wi-Fi không kết nối, tiếp tục các task khác...");
  // }

  // Hiển thị các ngưỡng hiện tại
  Serial.print("Ngưỡng nhiệt độ hiện tại: ");
  Serial.println(thresholdTemperature);
  Serial.print("Ngưỡng độ ẩm đất hiện tại: ");
  Serial.println(thresholdSoilMoisture);
  Serial.print("Ngưỡng độ ẩm kk hiện tại: ");
  Serial.println(thresholdHumidity);
  Serial.print("Ngưỡng ánh sáng hiện tại: ");
  Serial.println(thresholdLight);

  // Thực hiện các task khác như điều khiển thiết bị, đọc cảm biến, v.v.
  // Các task khác sẽ không phụ thuộc vào kết nối Wi-Fi
  // Nút nhấn
  static int prevSelection = -1;  // Lưu trữ lựa chọn trước đó để kiểm tra sự thay đổi

  // Chọn giá trị cần điều chỉnh
  if (digitalRead(BUTTON_SELECT_PIN) == LOW) {
    currentSelection = (currentSelection + 1) % 3;  // Chuyển qua 3 giá trị (0 đến 2)
    delay(500);                                     // Debounce
  }

  // Tăng giá trị của ngưỡng hiện tại
  if (digitalRead(BUTTON_INC_PIN) == LOW) {
    switch (currentSelection) {
      case 0:
        thresholdSoilMoisture = min(thresholdSoilMoisture + 2.0f, 100.0f);  // Tăng đến tối đa 100
        break;
      case 1:
        thresholdTemperature = min(thresholdTemperature + 1.0f, 50.0f);  // Giới hạn nhiệt độ từ 0-50
        break;
      case 2:
        thresholdHumidity = min(thresholdHumidity + 2.0f, 100.0f);
        break;
    }            // Cập nhật màn hình OLED với giá trị mới
    delay(500);  // Debounce
  }

  // Giảm giá trị của ngưỡng hiện tại
  if (digitalRead(BUTTON_DEC_PIN) == LOW) {
    switch (currentSelection) {
      case 0:
        thresholdSoilMoisture = max(thresholdSoilMoisture - 2.0f, 0.0f);  // Giảm đến tối thiểu 0
        break;
      case 1:
        thresholdTemperature = max(thresholdTemperature - 1.0f, 0.0f);
        break;
      case 2:
        thresholdHumidity = max(thresholdHumidity - 2.0f, 0.0f);
        break;
    }
    // Cập nhật màn hình OLED với giá trị mới
    delay(500);  // Debounce
  }

  // Cập nhật màn hình OLED nếu lựa chọn thay đổi
  if (prevSelection != currentSelection) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Selected: ");
    switch (currentSelection) {
      case 0:
        display.println("Soil Moisture");
        display.print("Threshold Soil Moisture: ");
        display.println(thresholdSoilMoisture);
        break;
      case 1:
        display.println("Temperature");
        display.print("Threshold Temperature: ");
        display.println(thresholdTemperature);
        break;
      case 2:
        display.println("Humidity");
        display.print("Threshold Humidity: ");
        display.println(thresholdHumidity);
        break;
    }
    display.display();  // Hiển thị lên màn hình
    prevSelection = currentSelection;
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS);  // Trì hoãn 1 giây để giảm tải CPU
}
