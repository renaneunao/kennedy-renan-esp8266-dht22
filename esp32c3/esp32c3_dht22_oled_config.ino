/*
 * ESP32-C3 DHT22 + OLED Monitor com Configuração Web
 * Sistema de monitoramento com configuração via web
 * 
 * Hardware:
 * - ESP32-C3
 * - Sensor DHT22 (GPIO4)
 * - Display OLED SSD1306 (SDA: GPIO6, SCL: GPIO7)
 * 
 * Funcionalidades:
 * - Modo AP para configuração inicial
 * - Servidor web para configurar WiFi e dispositivo
 * - Armazenamento de configurações no SPIFFS
 * - Fallback para modo AP em caso de falha WiFi
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <Preferences.h>

// Configurações do sensor DHT22
#define DHT_PIN 4
#define DHT_TYPE DHT22

// Configurações do OLED
#define OLED_SDA 6
#define OLED_SCL 7
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Configurações do AP
const char* apSSID = "ESP32-C3-Config";
const char* apPassword = "config123";
const char* apIP = "192.168.4.1";

// Configurações padrão do servidor
const char* defaultServerUrl = "http://194.163.142.108:5006/api/sensor-data";
const char* defaultApiKey = "esp32c3-dht22-key";

// Configurações de timing
const unsigned long SEND_INTERVAL = 60000; // 1 minuto
const unsigned long WIFI_TIMEOUT = 30000;  // 30 segundos
const unsigned long SENSOR_TIMEOUT = 5000; // 5 segundos
const unsigned long DISPLAY_UPDATE_INTERVAL = 2000; // 2 segundos
const unsigned long AP_TIMEOUT = 300000; // 5 minutos em modo AP

// Pinos
#define LED_PIN 2
#define STATUS_LED_PIN LED_BUILTIN

// Objetos
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WebServer server(80);
Preferences preferences;

// Variáveis de configuração
String wifiSSID = "";
String wifiPassword = "";
String deviceName = "ESP32C3_001";
String serverUrl = defaultServerUrl;
String apiKey = defaultApiKey;

// Variáveis de estado
unsigned long lastSendTime = 0;
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
float lastTemperature = 0;
float lastHumidity = 0;
bool wifiConnected = false;
bool apMode = false;
bool configMode = false;
int connectionAttempts = 0;
const int maxConnectionAttempts = 3;
unsigned long apStartTime = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32-C3 DHT22 + OLED Monitor ===");
  Serial.println("Iniciando sistema...");
  
  // Inicializar pinos
  pinMode(LED_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, HIGH);
  
  // Inicializar I2C para OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  
  // Inicializar display OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erro ao inicializar OLED!");
    while(1);
  }
  Serial.println("Display OLED inicializado");
  
  // Inicializar SPIFFS
  if(!SPIFFS.begin(true)) {
    Serial.println("Erro ao inicializar SPIFFS!");
  }
  
  // Inicializar sensor DHT22
  dht.begin();
  Serial.println("Sensor DHT22 inicializado");
  
  // Carregar configurações salvas
  loadConfig();
  
  // Exibir informações iniciais
  displayInitialInfo();
  
  // Tentar conectar ao WiFi ou entrar em modo AP
  if (wifiSSID.length() > 0) {
    connectToWiFi();
  } else {
    startAPMode();
  }
  
  Serial.println("Sistema inicializado com sucesso!");
  Serial.println("===============================\n");
}

void loop() {
  unsigned long currentTime = millis();
  
  if (apMode) {
    // Modo AP - gerenciar servidor web
    server.handleClient();
    
    // Timeout do modo AP (5 minutos)
    if (currentTime - apStartTime > AP_TIMEOUT) {
      Serial.println("Timeout do modo AP. Reiniciando...");
      ESP.restart();
    }
    
    // Atualizar display do AP
    if (currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
      updateAPDisplay();
      lastDisplayUpdate = currentTime;
    }
  } else {
    // Modo normal - monitoramento
    if (WiFi.status() != WL_CONNECTED) {
      if (wifiConnected) {
        Serial.println("WiFi desconectado! Tentando reconectar...");
        wifiConnected = false;
        digitalWrite(STATUS_LED_PIN, HIGH);
      }
      
      connectionAttempts++;
      if (connectionAttempts >= maxConnectionAttempts) {
        Serial.println("Muitas tentativas de conexão. Entrando em modo AP...");
        startAPMode();
        return;
      }
      
      connectToWiFi();
      return;
    }
    
    // Ler sensor a cada 10 segundos
    if (currentTime - lastSensorRead >= 10000) {
      readSensor();
      lastSensorRead = currentTime;
    }
    
    // Atualizar display a cada 2 segundos
    if (currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
      updateDisplay();
      lastDisplayUpdate = currentTime;
    }
    
    // Enviar dados no intervalo configurado
    if (currentTime - lastSendTime >= SEND_INTERVAL) {
      sendDataToServer();
      lastSendTime = currentTime;
    }
  }
  
  // Indicador de status
  blinkStatusLED();
  delay(1000);
}

void startAPMode() {
  Serial.println("Iniciando modo AP para configuração...");
  apMode = true;
  apStartTime = millis();
  
  // Configurar AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID, apPassword);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  
  // Configurar servidor web
  setupWebServer();
  server.begin();
  
  Serial.println("AP iniciado!");
  Serial.println("SSID: " + String(apSSID));
  Serial.println("IP: " + WiFi.softAPIP().toString());
  Serial.println("Conecte-se e acesse: http://192.168.4.1");
}

void setupWebServer() {
  // Página principal de configuração
  server.on("/", HTTP_GET, []() {
    String html = getConfigPage();
    server.send(200, "text/html", html);
  });
  
  // Salvar configurações
  server.on("/save", HTTP_POST, []() {
    if (server.hasArg("wifi_ssid")) {
      wifiSSID = server.arg("wifi_ssid");
    }
    if (server.hasArg("wifi_password")) {
      wifiPassword = server.arg("wifi_password");
    }
    if (server.hasArg("device_name")) {
      deviceName = server.arg("device_name");
    }
    if (server.hasArg("server_url")) {
      serverUrl = server.arg("server_url");
    }
    if (server.hasArg("api_key")) {
      apiKey = server.arg("api_key");
    }
    
    // Salvar configurações
    saveConfig();
    
    // Resposta
    String html = "<!DOCTYPE html><html><head><title>Configuração Salva</title></head><body>";
    html += "<h1>Configuração Salva!</h1>";
    html += "<p>Reiniciando o dispositivo...</p>";
    html += "<p>O dispositivo tentará conectar ao WiFi configurado.</p>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
    
    // Reiniciar após 3 segundos
    delay(3000);
    ESP.restart();
  });
  
  // Página de status
  server.on("/status", HTTP_GET, []() {
    String json = "{";
    json += "\"wifi_connected\":" + String(wifiConnected ? "true" : "false") + ",";
    json += "\"temperature\":" + String(lastTemperature, 1) + ",";
    json += "\"humidity\":" + String(lastHumidity, 1) + ",";
    json += "\"device_name\":\"" + deviceName + "\"";
    json += "}";
    
    server.send(200, "application/json", json);
  });
}

String getConfigPage() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>Configuração ESP32-C3</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }";
  html += ".container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += "h1 { color: #333; text-align: center; }";
  html += "label { display: block; margin-top: 15px; font-weight: bold; }";
  html += "input, select { width: 100%; padding: 10px; margin-top: 5px; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }";
  html += "button { background-color: #4CAF50; color: white; padding: 12px 20px; border: none; border-radius: 5px; cursor: pointer; width: 100%; margin-top: 20px; font-size: 16px; }";
  html += "button:hover { background-color: #45a049; }";
  html += ".info { background-color: #e7f3ff; padding: 10px; border-radius: 5px; margin-bottom: 20px; }";
  html += "</style></head><body>";
  
  html += "<div class='container'>";
  html += "<h1>🔧 Configuração ESP32-C3</h1>";
  
  html += "<div class='info'>";
  html += "<strong>📡 Conecte-se à rede:</strong><br>";
  html += "SSID: <strong>" + String(apSSID) + "</strong><br>";
  html += "Senha: <strong>" + String(apPassword) + "</strong><br>";
  html += "Acesse: <strong>http://192.168.4.1</strong>";
  html += "</div>";
  
  html += "<form action='/save' method='POST'>";
  
  html += "<label for='wifi_ssid'>📶 Nome da Rede WiFi:</label>";
  html += "<input type='text' id='wifi_ssid' name='wifi_ssid' value='" + wifiSSID + "' required>";
  
  html += "<label for='wifi_password'>🔐 Senha do WiFi:</label>";
  html += "<input type='password' id='wifi_password' name='wifi_password' value='" + wifiPassword + "' required>";
  
  html += "<label for='device_name'>🏷️ Nome do Dispositivo:</label>";
  html += "<input type='text' id='device_name' name='device_name' value='" + deviceName + "' required>";
  
  html += "<label for='server_url'>🌐 URL do Servidor:</label>";
  html += "<input type='url' id='server_url' name='server_url' value='" + serverUrl + "' required>";
  
  html += "<label for='api_key'>🔑 Chave da API:</label>";
  html += "<input type='text' id='api_key' name='api_key' value='" + apiKey + "' required>";
  
  html += "<button type='submit'>💾 Salvar Configuração</button>";
  html += "</form>";
  
  html += "<div style='text-align: center; margin-top: 20px; color: #666;'>";
  html += "<small>Após salvar, o dispositivo reiniciará automaticamente.</small>";
  html += "</div>";
  
  html += "</div></body></html>";
  
  return html;
}

void loadConfig() {
  preferences.begin("esp32config", false);
  
  wifiSSID = preferences.getString("wifi_ssid", "");
  wifiPassword = preferences.getString("wifi_password", "");
  deviceName = preferences.getString("device_name", "ESP32C3_001");
  serverUrl = preferences.getString("server_url", defaultServerUrl);
  apiKey = preferences.getString("api_key", defaultApiKey);
  
  preferences.end();
  
  Serial.println("Configurações carregadas:");
  Serial.println("  WiFi SSID: " + wifiSSID);
  Serial.println("  Device Name: " + deviceName);
  Serial.println("  Server URL: " + serverUrl);
}

void saveConfig() {
  preferences.begin("esp32config", false);
  
  preferences.putString("wifi_ssid", wifiSSID);
  preferences.putString("wifi_password", wifiPassword);
  preferences.putString("device_name", deviceName);
  preferences.putString("server_url", serverUrl);
  preferences.putString("api_key", apiKey);
  
  preferences.end();
  
  Serial.println("Configurações salvas!");
}

void connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    return;
  }
  
  if (wifiSSID.length() == 0) {
    Serial.println("Nenhuma configuração WiFi encontrada. Entrando em modo AP...");
    startAPMode();
    return;
  }
  
  Serial.print("Conectando ao WiFi: ");
  Serial.println(wifiSSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT) {
    delay(500);
    Serial.print(".");
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    apMode = false;
    connectionAttempts = 0;
    digitalWrite(STATUS_LED_PIN, LOW);
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\nFalha na conexão WiFi!");
    Serial.println("Tentativa: " + String(connectionAttempts) + "/" + String(maxConnectionAttempts));
  }
}

void readSensor() {
  if (millis() - lastSensorRead < 2000) {
    return;
  }
  
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Erro na leitura do sensor DHT22!");
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    return;
  }
  
  lastTemperature = temperature;
  lastHumidity = humidity;
  
  Serial.println("Dados do sensor:");
  Serial.println("  Temperatura: " + String(temperature, 1) + "°C");
  Serial.println("  Umidade: " + String(humidity, 1) + "%");
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Cabeçalho
  display.setCursor(0, 0);
  display.println("ESP32-C3 Monitor");
  display.println("----------------");
  
  // Status WiFi
  display.setCursor(0, 16);
  if (wifiConnected) {
    display.println("WiFi: Conectado");
    display.print("IP: ");
    display.println(WiFi.localIP().toString());
  } else {
    display.println("WiFi: Desconectado");
  }
  
  // Dados do sensor
  display.setCursor(0, 32);
  display.println("Temp: " + String(lastTemperature, 1) + "°C");
  display.println("Umid: " + String(lastHumidity, 1) + "%");
  
  // Status de envio
  display.setCursor(0, 48);
  unsigned long timeToNextSend = (SEND_INTERVAL - (millis() - lastSendTime)) / 1000;
  display.println("Prox envio: " + String(timeToNextSend) + "s");
  
  display.display();
}

void updateAPDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Cabeçalho
  display.setCursor(0, 0);
  display.println("Modo Configuracao");
  display.println("----------------");
  
  // Informações do AP
  display.setCursor(0, 16);
  display.println("SSID: " + String(apSSID));
  display.println("IP: 192.168.4.1");
  display.println("Conecte-se e");
  display.println("configure o WiFi");
  
  // Contador
  display.setCursor(0, 48);
  unsigned long timeLeft = (AP_TIMEOUT - (millis() - apStartTime)) / 1000;
  display.println("Timeout: " + String(timeLeft) + "s");
  
  display.display();
}

void displayInitialInfo() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ESP32-C3 Monitor");
  display.println("----------------");
  display.println("Iniciando...");
  display.display();
}

void sendDataToServer() {
  if (!wifiConnected) {
    Serial.println("WiFi não conectado. Pulando envio de dados.");
    return;
  }
  
  if (lastTemperature == 0 && lastHumidity == 0) {
    Serial.println("Nenhum dado válido para enviar.");
    return;
  }
  
  Serial.println("Enviando dados para o servidor...");
  
  DynamicJsonDocument doc(1024);
  doc["temperature"] = lastTemperature;
  doc["humidity"] = lastHumidity;
  doc["device_id"] = deviceName;
  doc["timestamp"] = millis();
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  Serial.println("JSON: " + jsonString);
  
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", apiKey);
  http.addHeader("User-Agent", "ESP32-C3-DHT22/1.0");
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Resposta do servidor:");
    Serial.println("  Código: " + String(httpResponseCode));
    Serial.println("  Resposta: " + response);
    
    if (httpResponseCode == 200) {
      Serial.println("✅ Dados enviados com sucesso!");
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
    } else {
      Serial.println("❌ Erro no servidor: " + String(httpResponseCode));
    }
  } else {
    Serial.println("❌ Erro na conexão: " + String(httpResponseCode));
  }
  
  http.end();
}

void blinkStatusLED() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  if (millis() - lastBlink >= 1000) {
    ledState = !ledState;
    digitalWrite(STATUS_LED_PIN, ledState ? HIGH : LOW);
    lastBlink = millis();
  }
}
