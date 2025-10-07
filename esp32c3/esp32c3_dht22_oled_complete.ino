/*
 * ESP32-C3 DHT22 + OLED 0.96" Monitor Completo
 * Sistema de monitoramento com configuração web completa
 * 
 * Hardware:
 * - ESP32-C3
 * - Sensor DHT22 (configurável)
 * - Display OLED SSD1306 0.96" (configurável)
 * - LED de status (configurável)
 * 
 * Funcionalidades:
 * - Configuração completa via web
 * - GPIOs configuráveis
 * - Intervalos configuráveis
 * - Servidor configurável
 * - OLED 0.96" otimizado
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

// Configurações padrão do hardware
#define DEFAULT_DHT_PIN 4
#define DEFAULT_DHT_TYPE DHT22
#define DEFAULT_OLED_SDA 6
#define DEFAULT_OLED_SCL 7
#define DEFAULT_LED_PIN 2
#define DEFAULT_STATUS_LED_PIN LED_BUILTIN

// Configurações do OLED 0.96"
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Configurações do AP
const char* apSSID = "ESP32-C3-Config";
const char* apPassword = "config123";
const char* apIP = "192.168.4.1";

// Configurações padrão
const char* defaultServerUrl = "http://194.163.142.108:5006/api/sensor-data";
const char* defaultApiKey = "esp32c3-dht22-key";
const unsigned long defaultSendInterval = 60000;    // 1 minuto
const unsigned long defaultSensorInterval = 10000;  // 10 segundos
const unsigned long defaultDisplayInterval = 2000; // 2 segundos
const unsigned long defaultWifiTimeout = 30000;     // 30 segundos
const unsigned long defaultApTimeout = 300000;      // 5 minutos

// Objetos
DHT* dht = nullptr;
Adafruit_SSD1306* display = nullptr;
WebServer server(80);
Preferences preferences;

// Variáveis de configuração
struct DeviceConfig {
  // WiFi
  String wifiSSID = "";
  String wifiPassword = "";
  
  // Dispositivo
  String deviceName = "ESP32C3_001";
  String deviceLocation = "Não especificado";
  String deviceDescription = "ESP32-C3 DHT22 Monitor";
  
  // Servidor
  String serverUrl = defaultServerUrl;
  String apiKey = defaultApiKey;
  
  // Hardware - GPIOs
  int dhtPin = DEFAULT_DHT_PIN;
  int oledSDA = DEFAULT_OLED_SDA;
  int oledSCL = DEFAULT_OLED_SCL;
  int ledPin = DEFAULT_LED_PIN;
  int statusLedPin = DEFAULT_STATUS_LED_PIN;
  
  // Intervalos
  unsigned long sendInterval = defaultSendInterval;
  unsigned long sensorInterval = defaultSensorInterval;
  unsigned long displayInterval = defaultDisplayInterval;
  unsigned long wifiTimeout = defaultWifiTimeout;
  unsigned long apTimeout = defaultApTimeout;
  
  // OLED
  int oledAddress = 0x3C;
  bool oledEnabled = true;
  
  // Sistema
  bool debugMode = false;
  bool autoReconnect = true;
  int maxConnectionAttempts = 3;
} config;

// Variáveis de estado
unsigned long lastSendTime = 0;
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
float lastTemperature = 0;
float lastHumidity = 0;
bool wifiConnected = false;
bool apMode = false;
int connectionAttempts = 0;
unsigned long apStartTime = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32-C3 DHT22 + OLED 0.96\" Monitor Completo ===");
  Serial.println("Iniciando sistema...");
  
  // Inicializar SPIFFS
  if(!SPIFFS.begin(true)) {
    Serial.println("Erro ao inicializar SPIFFS!");
  }
  
  // Carregar configurações
  loadConfig();
  
  // Inicializar hardware com configurações carregadas
  initializeHardware();
  
  // Exibir informações iniciais
  displayInitialInfo();
  
  // Tentar conectar ao WiFi ou entrar em modo AP
  if (config.wifiSSID.length() > 0) {
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
    
    // Timeout do modo AP
    if (currentTime - apStartTime > config.apTimeout) {
      Serial.println("Timeout do modo AP. Reiniciando...");
      ESP.restart();
    }
    
    // Atualizar display do AP
    if (currentTime - lastDisplayUpdate >= config.displayInterval) {
      updateAPDisplay();
      lastDisplayUpdate = currentTime;
    }
  } else {
    // Modo normal - monitoramento
    if (WiFi.status() != WL_CONNECTED) {
      if (wifiConnected) {
        Serial.println("WiFi desconectado! Tentando reconectar...");
        wifiConnected = false;
        if (config.statusLedPin >= 0) digitalWrite(config.statusLedPin, HIGH);
      }
      
      if (config.autoReconnect) {
        connectionAttempts++;
        if (connectionAttempts >= config.maxConnectionAttempts) {
          Serial.println("Muitas tentativas de conexão. Entrando em modo AP...");
          startAPMode();
          return;
        }
        connectToWiFi();
      }
      return;
    }
    
    // Ler sensor no intervalo configurado
    if (currentTime - lastSensorRead >= config.sensorInterval) {
      readSensor();
      lastSensorRead = currentTime;
    }
    
    // Atualizar display no intervalo configurado
    if (currentTime - lastDisplayUpdate >= config.displayInterval && config.oledEnabled) {
      updateDisplay();
      lastDisplayUpdate = currentTime;
    }
    
    // Enviar dados no intervalo configurado
    if (currentTime - lastSendTime >= config.sendInterval) {
      sendDataToServer();
      lastSendTime = currentTime;
    }
  }
  
  // Indicador de status
  blinkStatusLED();
  delay(1000);
}

void initializeHardware() {
  // Inicializar pinos
  if (config.ledPin >= 0) {
    pinMode(config.ledPin, OUTPUT);
    digitalWrite(config.ledPin, LOW);
  }
  if (config.statusLedPin >= 0) {
    pinMode(config.statusLedPin, OUTPUT);
    digitalWrite(config.statusLedPin, HIGH);
  }
  
  // Inicializar I2C para OLED
  if (config.oledEnabled) {
    Wire.begin(config.oledSDA, config.oledSCL);
    
    // Inicializar display OLED
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    if(!display->begin(SSD1306_SWITCHCAPVCC, config.oledAddress)) {
      Serial.println("Erro ao inicializar OLED!");
      config.oledEnabled = false;
    } else {
      Serial.println("Display OLED 0.96\" inicializado");
    }
  }
  
  // Inicializar sensor DHT22
  dht = new DHT(config.dhtPin, DEFAULT_DHT_TYPE);
  dht->begin();
  Serial.println("Sensor DHT22 inicializado no pino " + String(config.dhtPin));
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
    String html = getCompleteConfigPage();
    server.send(200, "text/html", html);
  });
  
  // Salvar configurações
  server.on("/save", HTTP_POST, []() {
    saveConfigFromWeb();
    
    String html = "<!DOCTYPE html><html><head><title>Configuração Salva</title></head><body>";
    html += "<h1>Configuração Salva!</h1>";
    html += "<p>Reiniciando o dispositivo...</p>";
    html += "<p>O dispositivo tentará conectar ao WiFi configurado.</p>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
    
    delay(3000);
    ESP.restart();
  });
  
  // Página de status
  server.on("/status", HTTP_GET, []() {
    String json = "{";
    json += "\"wifi_connected\":" + String(wifiConnected ? "true" : "false") + ",";
    json += "\"temperature\":" + String(lastTemperature, 1) + ",";
    json += "\"humidity\":" + String(lastHumidity, 1) + ",";
    json += "\"device_name\":\"" + config.deviceName + "\",";
    json += "\"ap_mode\":" + String(apMode ? "true" : "false");
    json += "}";
    
    server.send(200, "application/json", json);
  });
  
  // Resetar configurações
  server.on("/reset", HTTP_POST, []() {
    resetConfig();
    server.send(200, "text/plain", "Configurações resetadas. Reiniciando...");
    delay(2000);
    ESP.restart();
  });
}

String getCompleteConfigPage() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>Configuração Completa ESP32-C3</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }";
  html += ".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += "h1 { color: #333; text-align: center; }";
  html += "h2 { color: #4CAF50; border-bottom: 2px solid #4CAF50; padding-bottom: 5px; }";
  html += "label { display: block; margin-top: 15px; font-weight: bold; }";
  html += "input, select, textarea { width: 100%; padding: 10px; margin-top: 5px; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }";
  html += "button { background-color: #4CAF50; color: white; padding: 12px 20px; border: none; border-radius: 5px; cursor: pointer; width: 100%; margin-top: 20px; font-size: 16px; }";
  html += "button:hover { background-color: #45a049; }";
  html += ".info { background-color: #e7f3ff; padding: 10px; border-radius: 5px; margin-bottom: 20px; }";
  html += ".section { margin-bottom: 30px; padding: 20px; border: 1px solid #ddd; border-radius: 5px; }";
  html += ".grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; }";
  html += ".reset-btn { background-color: #f44336; margin-top: 10px; }";
  html += ".reset-btn:hover { background-color: #da190b; }";
  html += "</style></head><body>";
  
  html += "<div class='container'>";
  html += "<h1>🔧 Configuração Completa ESP32-C3</h1>";
  
  html += "<div class='info'>";
  html += "<strong>📡 Conecte-se à rede:</strong><br>";
  html += "SSID: <strong>" + String(apSSID) + "</strong><br>";
  html += "Senha: <strong>" + String(apPassword) + "</strong><br>";
  html += "Acesse: <strong>http://192.168.4.1</strong>";
  html += "</div>";
  
  html += "<form action='/save' method='POST'>";
  
  // Seção WiFi
  html += "<div class='section'>";
  html += "<h2>📶 Configuração WiFi</h2>";
  html += "<label for='wifi_ssid'>Nome da Rede WiFi:</label>";
  html += "<input type='text' id='wifi_ssid' name='wifi_ssid' value='" + config.wifiSSID + "' required>";
  html += "<label for='wifi_password'>Senha do WiFi:</label>";
  html += "<input type='password' id='wifi_password' name='wifi_password' value='" + config.wifiPassword + "' required>";
  html += "</div>";
  
  // Seção Dispositivo
  html += "<div class='section'>";
  html += "<h2>🏷️ Informações do Dispositivo</h2>";
  html += "<label for='device_name'>Nome do Dispositivo:</label>";
  html += "<input type='text' id='device_name' name='device_name' value='" + config.deviceName + "' required>";
  html += "<label for='device_location'>Localização:</label>";
  html += "<input type='text' id='device_location' name='device_location' value='" + config.deviceLocation + "'>";
  html += "<label for='device_description'>Descrição:</label>";
  html += "<textarea id='device_description' name='device_description' rows='3'>" + config.deviceDescription + "</textarea>";
  html += "</div>";
  
  // Seção Servidor
  html += "<div class='section'>";
  html += "<h2>🌐 Configuração do Servidor</h2>";
  html += "<label for='server_url'>URL do Servidor:</label>";
  html += "<input type='url' id='server_url' name='server_url' value='" + config.serverUrl + "' required>";
  html += "<label for='api_key'>Chave da API:</label>";
  html += "<input type='text' id='api_key' name='api_key' value='" + config.apiKey + "' required>";
  html += "</div>";
  
  // Seção Hardware - GPIOs
  html += "<div class='section'>";
  html += "<h2>🔌 Configuração de Hardware (GPIOs)</h2>";
  html += "<div class='grid'>";
  html += "<div>";
  html += "<label for='dht_pin'>Pino do DHT22:</label>";
  html += "<input type='number' id='dht_pin' name='dht_pin' value='" + String(config.dhtPin) + "' min='0' max='21'>";
  html += "<label for='oled_sda'>OLED SDA:</label>";
  html += "<input type='number' id='oled_sda' name='oled_sda' value='" + String(config.oledSDA) + "' min='0' max='21'>";
  html += "<label for='oled_scl'>OLED SCL:</label>";
  html += "<input type='number' id='oled_scl' name='oled_scl' value='" + String(config.oledSCL) + "' min='0' max='21'>";
  html += "</div>";
  html += "<div>";
  html += "<label for='led_pin'>Pino do LED:</label>";
  html += "<input type='number' id='led_pin' name='led_pin' value='" + String(config.ledPin) + "' min='-1' max='21'>";
  html += "<label for='status_led_pin'>Pino LED Status:</label>";
  html += "<input type='number' id='status_led_pin' name='status_led_pin' value='" + String(config.statusLedPin) + "' min='-1' max='21'>";
  html += "<label for='oled_address'>Endereço OLED (hex):</label>";
  html += "<input type='text' id='oled_address' name='oled_address' value='0x" + String(config.oledAddress, HEX) + "'>";
  html += "</div>";
  html += "</div>";
  html += "</div>";
  
  // Seção Intervalos
  html += "<div class='section'>";
  html += "<h2>⏱️ Configuração de Intervalos (ms)</h2>";
  html += "<div class='grid'>";
  html += "<div>";
  html += "<label for='send_interval'>Intervalo de Envio:</label>";
  html += "<input type='number' id='send_interval' name='send_interval' value='" + String(config.sendInterval) + "' min='1000' max='3600000'>";
  html += "<label for='sensor_interval'>Intervalo do Sensor:</label>";
  html += "<input type='number' id='sensor_interval' name='sensor_interval' value='" + String(config.sensorInterval) + "' min='1000' max='60000'>";
  html += "</div>";
  html += "<div>";
  html += "<label for='display_interval'>Intervalo do Display:</label>";
  html += "<input type='number' id='display_interval' name='display_interval' value='" + String(config.displayInterval) + "' min='500' max='10000'>";
  html += "<label for='wifi_timeout'>Timeout WiFi:</label>";
  html += "<input type='number' id='wifi_timeout' name='wifi_timeout' value='" + String(config.wifiTimeout) + "' min='5000' max='120000'>";
  html += "</div>";
  html += "</div>";
  html += "</div>";
  
  // Seção Sistema
  html += "<div class='section'>";
  html += "<h2>⚙️ Configurações do Sistema</h2>";
  html += "<label for='max_attempts'>Máximo de Tentativas WiFi:</label>";
  html += "<input type='number' id='max_attempts' name='max_attempts' value='" + String(config.maxConnectionAttempts) + "' min='1' max='10'>";
  html += "<label for='auto_reconnect'>Reconexão Automática:</label>";
  html += "<select id='auto_reconnect' name='auto_reconnect'>";
  html += "<option value='1'" + (config.autoReconnect ? " selected" : "") + ">Ativada</option>";
  html += "<option value='0'" + (!config.autoReconnect ? " selected" : "") + ">Desativada</option>";
  html += "</select>";
  html += "<label for='debug_mode'>Modo Debug:</label>";
  html += "<select id='debug_mode' name='debug_mode'>";
  html += "<option value='1'" + (config.debugMode ? " selected" : "") + ">Ativado</option>";
  html += "<option value='0'" + (!config.debugMode ? " selected" : "") + ">Desativado</option>";
  html += "</select>";
  html += "</div>";
  
  html += "<button type='submit'>💾 Salvar Configuração Completa</button>";
  html += "</form>";
  
  html += "<form action='/reset' method='POST' style='margin-top: 20px;'>";
  html += "<button type='submit' class='reset-btn'>🔄 Resetar Configurações</button>";
  html += "</form>";
  
  html += "<div style='text-align: center; margin-top: 20px; color: #666;'>";
  html += "<small>Após salvar, o dispositivo reiniciará automaticamente.</small>";
  html += "</div>";
  
  html += "</div></body></html>";
  
  return html;
}

void saveConfigFromWeb() {
  // WiFi
  if (server.hasArg("wifi_ssid")) config.wifiSSID = server.arg("wifi_ssid");
  if (server.hasArg("wifi_password")) config.wifiPassword = server.arg("wifi_password");
  
  // Dispositivo
  if (server.hasArg("device_name")) config.deviceName = server.arg("device_name");
  if (server.hasArg("device_location")) config.deviceLocation = server.arg("device_location");
  if (server.hasArg("device_description")) config.deviceDescription = server.arg("device_description");
  
  // Servidor
  if (server.hasArg("server_url")) config.serverUrl = server.arg("server_url");
  if (server.hasArg("api_key")) config.apiKey = server.arg("api_key");
  
  // Hardware
  if (server.hasArg("dht_pin")) config.dhtPin = server.arg("dht_pin").toInt();
  if (server.hasArg("oled_sda")) config.oledSDA = server.arg("oled_sda").toInt();
  if (server.hasArg("oled_scl")) config.oledSCL = server.arg("oled_scl").toInt();
  if (server.hasArg("led_pin")) config.ledPin = server.arg("led_pin").toInt();
  if (server.hasArg("status_led_pin")) config.statusLedPin = server.arg("status_led_pin").toInt();
  if (server.hasArg("oled_address")) {
    String addr = server.arg("oled_address");
    if (addr.startsWith("0x")) addr = addr.substring(2);
    config.oledAddress = strtol(addr.c_str(), NULL, 16);
  }
  
  // Intervalos
  if (server.hasArg("send_interval")) config.sendInterval = server.arg("send_interval").toInt();
  if (server.hasArg("sensor_interval")) config.sensorInterval = server.arg("sensor_interval").toInt();
  if (server.hasArg("display_interval")) config.displayInterval = server.arg("display_interval").toInt();
  if (server.hasArg("wifi_timeout")) config.wifiTimeout = server.arg("wifi_timeout").toInt();
  
  // Sistema
  if (server.hasArg("max_attempts")) config.maxConnectionAttempts = server.arg("max_attempts").toInt();
  if (server.hasArg("auto_reconnect")) config.autoReconnect = (server.arg("auto_reconnect") == "1");
  if (server.hasArg("debug_mode")) config.debugMode = (server.arg("debug_mode") == "1");
  
  // Salvar configurações
  saveConfig();
}

void loadConfig() {
  preferences.begin("esp32config", false);
  
  // WiFi
  config.wifiSSID = preferences.getString("wifi_ssid", "");
  config.wifiPassword = preferences.getString("wifi_password", "");
  
  // Dispositivo
  config.deviceName = preferences.getString("device_name", "ESP32C3_001");
  config.deviceLocation = preferences.getString("device_location", "Não especificado");
  config.deviceDescription = preferences.getString("device_description", "ESP32-C3 DHT22 Monitor");
  
  // Servidor
  config.serverUrl = preferences.getString("server_url", defaultServerUrl);
  config.apiKey = preferences.getString("api_key", defaultApiKey);
  
  // Hardware
  config.dhtPin = preferences.getInt("dht_pin", DEFAULT_DHT_PIN);
  config.oledSDA = preferences.getInt("oled_sda", DEFAULT_OLED_SDA);
  config.oledSCL = preferences.getInt("oled_scl", DEFAULT_OLED_SCL);
  config.ledPin = preferences.getInt("led_pin", DEFAULT_LED_PIN);
  config.statusLedPin = preferences.getInt("status_led_pin", DEFAULT_STATUS_LED_PIN);
  config.oledAddress = preferences.getInt("oled_address", 0x3C);
  config.oledEnabled = preferences.getBool("oled_enabled", true);
  
  // Intervalos
  config.sendInterval = preferences.getULong("send_interval", defaultSendInterval);
  config.sensorInterval = preferences.getULong("sensor_interval", defaultSensorInterval);
  config.displayInterval = preferences.getULong("display_interval", defaultDisplayInterval);
  config.wifiTimeout = preferences.getULong("wifi_timeout", defaultWifiTimeout);
  config.apTimeout = preferences.getULong("ap_timeout", defaultApTimeout);
  
  // Sistema
  config.maxConnectionAttempts = preferences.getInt("max_attempts", 3);
  config.autoReconnect = preferences.getBool("auto_reconnect", true);
  config.debugMode = preferences.getBool("debug_mode", false);
  
  preferences.end();
  
  if (config.debugMode) {
    Serial.println("Configurações carregadas:");
    Serial.println("  WiFi SSID: " + config.wifiSSID);
    Serial.println("  Device Name: " + config.deviceName);
    Serial.println("  Server URL: " + config.serverUrl);
    Serial.println("  DHT Pin: " + String(config.dhtPin));
    Serial.println("  OLED SDA: " + String(config.oledSDA) + ", SCL: " + String(config.oledSCL));
  }
}

void saveConfig() {
  preferences.begin("esp32config", false);
  
  // WiFi
  preferences.putString("wifi_ssid", config.wifiSSID);
  preferences.putString("wifi_password", config.wifiPassword);
  
  // Dispositivo
  preferences.putString("device_name", config.deviceName);
  preferences.putString("device_location", config.deviceLocation);
  preferences.putString("device_description", config.deviceDescription);
  
  // Servidor
  preferences.putString("server_url", config.serverUrl);
  preferences.putString("api_key", config.apiKey);
  
  // Hardware
  preferences.putInt("dht_pin", config.dhtPin);
  preferences.putInt("oled_sda", config.oledSDA);
  preferences.putInt("oled_scl", config.oledSCL);
  preferences.putInt("led_pin", config.ledPin);
  preferences.putInt("status_led_pin", config.statusLedPin);
  preferences.putInt("oled_address", config.oledAddress);
  preferences.putBool("oled_enabled", config.oledEnabled);
  
  // Intervalos
  preferences.putULong("send_interval", config.sendInterval);
  preferences.putULong("sensor_interval", config.sensorInterval);
  preferences.putULong("display_interval", config.displayInterval);
  preferences.putULong("wifi_timeout", config.wifiTimeout);
  preferences.putULong("ap_timeout", config.apTimeout);
  
  // Sistema
  preferences.putInt("max_attempts", config.maxConnectionAttempts);
  preferences.putBool("auto_reconnect", config.autoReconnect);
  preferences.putBool("debug_mode", config.debugMode);
  
  preferences.end();
  
  Serial.println("Configurações salvas!");
}

void resetConfig() {
  preferences.begin("esp32config", false);
  preferences.clear();
  preferences.end();
  
  // Resetar para valores padrão
  config = DeviceConfig();
  
  Serial.println("Configurações resetadas para padrão!");
}

void connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    return;
  }
  
  if (config.wifiSSID.length() == 0) {
    Serial.println("Nenhuma configuração WiFi encontrada. Entrando em modo AP...");
    startAPMode();
    return;
  }
  
  Serial.print("Conectando ao WiFi: ");
  Serial.println(config.wifiSSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.wifiSSID.c_str(), config.wifiPassword.c_str());
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < config.wifiTimeout) {
    delay(500);
    Serial.print(".");
    if (config.statusLedPin >= 0) digitalWrite(config.statusLedPin, !digitalRead(config.statusLedPin));
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    apMode = false;
    connectionAttempts = 0;
    if (config.statusLedPin >= 0) digitalWrite(config.statusLedPin, LOW);
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\nFalha na conexão WiFi!");
    Serial.println("Tentativa: " + String(connectionAttempts) + "/" + String(config.maxConnectionAttempts));
  }
}

void readSensor() {
  if (millis() - lastSensorRead < 2000) {
    return;
  }
  
  float temperature = dht->readTemperature();
  float humidity = dht->readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Erro na leitura do sensor DHT22!");
    if (config.ledPin >= 0) {
      digitalWrite(config.ledPin, HIGH);
      delay(100);
      digitalWrite(config.ledPin, LOW);
    }
    return;
  }
  
  lastTemperature = temperature;
  lastHumidity = humidity;
  
  if (config.debugMode) {
    Serial.println("Dados do sensor:");
    Serial.println("  Temperatura: " + String(temperature, 1) + "°C");
    Serial.println("  Umidade: " + String(humidity, 1) + "%");
  }
}

void updateDisplay() {
  if (!config.oledEnabled || !display) return;
  
  display->clearDisplay();
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  
  // Cabeçalho
  display->setCursor(0, 0);
  display->println("ESP32-C3 Monitor");
  display->println("----------------");
  
  // Status WiFi
  display->setCursor(0, 16);
  if (wifiConnected) {
    display->println("WiFi: Conectado");
    display->print("IP: ");
    display->println(WiFi.localIP().toString());
  } else {
    display->println("WiFi: Desconectado");
  }
  
  // Dados do sensor
  display->setCursor(0, 32);
  display->println("Temp: " + String(lastTemperature, 1) + "°C");
  display->println("Umid: " + String(lastHumidity, 1) + "%");
  
  // Status de envio
  display->setCursor(0, 48);
  unsigned long timeToNextSend = (config.sendInterval - (millis() - lastSendTime)) / 1000;
  display->println("Prox envio: " + String(timeToNextSend) + "s");
  
  display->display();
}

void updateAPDisplay() {
  if (!config.oledEnabled || !display) return;
  
  display->clearDisplay();
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  
  // Cabeçalho
  display->setCursor(0, 0);
  display->println("Modo Configuracao");
  display->println("----------------");
  
  // Informações do AP
  display->setCursor(0, 16);
  display->println("SSID: " + String(apSSID));
  display->println("IP: 192.168.4.1");
  display->println("Conecte-se e");
  display->println("configure o WiFi");
  
  // Contador
  display->setCursor(0, 48);
  unsigned long timeLeft = (config.apTimeout - (millis() - apStartTime)) / 1000;
  display->println("Timeout: " + String(timeLeft) + "s");
  
  display->display();
}

void displayInitialInfo() {
  if (!config.oledEnabled || !display) return;
  
  display->clearDisplay();
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println("ESP32-C3 Monitor");
  display->println("----------------");
  display->println("Iniciando...");
  display->display();
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
  doc["device_id"] = config.deviceName;
  doc["timestamp"] = millis();
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  if (config.debugMode) {
    Serial.println("JSON: " + jsonString);
  }
  
  HTTPClient http;
  http.begin(config.serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", config.apiKey);
  http.addHeader("User-Agent", "ESP32-C3-DHT22/1.0");
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    if (config.debugMode) {
      Serial.println("Resposta do servidor:");
      Serial.println("  Código: " + String(httpResponseCode));
      Serial.println("  Resposta: " + response);
    }
    
    if (httpResponseCode == 200) {
      Serial.println("✅ Dados enviados com sucesso!");
      if (config.ledPin >= 0) {
        digitalWrite(config.ledPin, HIGH);
        delay(200);
        digitalWrite(config.ledPin, LOW);
      }
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
    if (config.statusLedPin >= 0) digitalWrite(config.statusLedPin, ledState ? HIGH : LOW);
    lastBlink = millis();
  }
}
