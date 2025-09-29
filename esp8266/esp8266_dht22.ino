/*
 * ESP8266 DHT22 Monitor
 * Sistema de monitoramento de temperatura e umidade
 * Envia dados via HTTP POST para servidor Flask
 * 
 * Hardware:
 * - NodeMCU ESP8266
 * - Sensor DHT22
 * - LED de status (opcional)
 * 
 * Conexões:
 * - DHT22 VCC -> 3.3V
 * - DHT22 GND -> GND
 * - DHT22 DATA -> D4 (GPIO2)
 * - LED -> D2 (GPIO4) - opcional
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

// Configurações do sensor DHT22
#define DHT_PIN D4
#define DHT_TYPE DHT22

// Configurações de rede
const char* ssid = "SEU_WIFI_SSID";        // Configure seu WiFi
const char* password = "SUA_SENHA_WIFI";   // Configure sua senha WiFi

// Configurações do servidor
const char* serverUrl = "http://194.163.142.108:5005/api/sensor-data";
const char* apiKey = "esp8266-dht22-key"; // API key para identificação

// Configurações de timing
const unsigned long SEND_INTERVAL = 60000; // 1 minuto (60000ms)
const unsigned long WIFI_TIMEOUT = 30000;  // 30 segundos
const unsigned long SENSOR_TIMEOUT = 5000; // 5 segundos

// Pinos
#define LED_PIN D2
#define STATUS_LED_PIN LED_BUILTIN

// Objetos
DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient client;
HTTPClient http;

// Variáveis
unsigned long lastSendTime = 0;
unsigned long lastSensorRead = 0;
float lastTemperature = 0;
float lastHumidity = 0;
bool wifiConnected = false;
int connectionAttempts = 0;
const int maxConnectionAttempts = 10;

// Estrutura para dados do sensor
struct SensorData {
  float temperature;
  float humidity;
  unsigned long timestamp;
  String deviceId;
};

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP8266 DHT22 Monitor ===");
  Serial.println("Iniciando sistema...");
  
  // Inicializar pinos
  pinMode(LED_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, HIGH); // LED interno ligado = WiFi desconectado
  
  // Inicializar sensor DHT22
  dht.begin();
  Serial.println("Sensor DHT22 inicializado");
  
  // Conectar ao WiFi
  connectToWiFi();
  
  // Teste inicial do sensor
  testSensor();
  
  Serial.println("Sistema inicializado com sucesso!");
  Serial.println("Enviando dados a cada " + String(SEND_INTERVAL/1000) + " segundos");
  Serial.println("Servidor: " + String(serverUrl));
  Serial.println("===============================\n");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Verificar conexão WiFi
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      Serial.println("WiFi desconectado! Tentando reconectar...");
      wifiConnected = false;
      digitalWrite(STATUS_LED_PIN, HIGH);
    }
    connectToWiFi();
    return;
  }
  
  // Ler sensor a cada 10 segundos
  if (currentTime - lastSensorRead >= 10000) {
    readSensor();
    lastSensorRead = currentTime;
  }
  
  // Enviar dados no intervalo configurado
  if (currentTime - lastSendTime >= SEND_INTERVAL) {
    sendDataToServer();
    lastSendTime = currentTime;
  }
  
  // Indicador de status
  blinkStatusLED();
  
  delay(1000); // Pequena pausa para estabilidade
}

void connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    return;
  }
  
  Serial.print("Conectando ao WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT) {
    delay(500);
    Serial.print(".");
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN)); // Piscar LED
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    connectionAttempts = 0;
    digitalWrite(STATUS_LED_PIN, LOW); // LED interno desligado = WiFi conectado
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    connectionAttempts++;
    Serial.println("\nFalha na conexão WiFi!");
    Serial.println("Tentativa: " + String(connectionAttempts) + "/" + String(maxConnectionAttempts));
    
    if (connectionAttempts >= maxConnectionAttempts) {
      Serial.println("Muitas tentativas de conexão. Reiniciando...");
      ESP.restart();
    }
  }
}

void readSensor() {
  // Verificar se o sensor está pronto
  if (millis() - lastSensorRead < 2000) {
    return; // DHT22 precisa de 2 segundos entre leituras
  }
  
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Verificar se as leituras são válidas
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Erro na leitura do sensor DHT22!");
    digitalWrite(LED_PIN, HIGH); // LED de erro
    delay(100);
    digitalWrite(LED_PIN, LOW);
    return;
  }
  
  // Atualizar valores
  lastTemperature = temperature;
  lastHumidity = humidity;
  
  Serial.println("Dados do sensor:");
  Serial.println("  Temperatura: " + String(temperature, 1) + "°C");
  Serial.println("  Umidade: " + String(humidity, 1) + "%");
  Serial.println("  Timestamp: " + String(millis()));
}

void sendDataToServer() {
  if (!wifiConnected) {
    Serial.println("WiFi não conectado. Pulando envio de dados.");
    return;
  }
  
  // Verificar se temos dados válidos
  if (lastTemperature == 0 && lastHumidity == 0) {
    Serial.println("Nenhum dado válido para enviar.");
    return;
  }
  
  Serial.println("Enviando dados para o servidor...");
  
  // Criar JSON
  DynamicJsonDocument doc(1024);
  doc["temperature"] = lastTemperature;
  doc["humidity"] = lastHumidity;
  doc["device_id"] = "ESP8266_001";  // Mude para ESP8266_002, ESP8266_003, etc. para múltiplos sensores
  doc["timestamp"] = millis();
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  Serial.println("JSON: " + jsonString);
  
  // Configurar HTTP
  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", apiKey);
  http.addHeader("User-Agent", "ESP8266-DHT22/1.0");
  
  // Enviar requisição
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Resposta do servidor:");
    Serial.println("  Código: " + String(httpResponseCode));
    Serial.println("  Resposta: " + response);
    
    if (httpResponseCode == 200) {
      Serial.println("✅ Dados enviados com sucesso!");
      digitalWrite(LED_PIN, HIGH); // LED de sucesso
      delay(200);
      digitalWrite(LED_PIN, LOW);
    } else {
      Serial.println("❌ Erro no servidor: " + String(httpResponseCode));
    }
  } else {
    Serial.println("❌ Erro na conexão: " + String(httpResponseCode));
    Serial.println("Erro: " + http.errorToString(httpResponseCode));
  }
  
  http.end();
}

void testSensor() {
  Serial.println("Testando sensor DHT22...");
  
  for (int i = 0; i < 3; i++) {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    
    if (!isnan(temp) && !isnan(hum)) {
      Serial.println("✅ Sensor funcionando!");
      Serial.println("  Temperatura: " + String(temp, 1) + "°C");
      Serial.println("  Umidade: " + String(hum, 1) + "%");
      return;
    }
    
    delay(2000);
  }
  
  Serial.println("❌ Erro no sensor DHT22!");
  Serial.println("Verifique as conexões:");
  Serial.println("  VCC -> 3.3V");
  Serial.println("  GND -> GND");
  Serial.println("  DATA -> D4");
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

// Função para configurar via web (futuro)
void startWebConfig() {
  // Implementar configuração via web interface
  // Para configuração de WiFi e servidor
}

// Função para deep sleep (economia de bateria)
void enterDeepSleep() {
  Serial.println("Entrando em deep sleep por " + String(SEND_INTERVAL/1000) + " segundos...");
  ESP.deepSleep(SEND_INTERVAL * 1000);
}

// Função de diagnóstico
void printDiagnostics() {
  Serial.println("\n=== DIAGNÓSTICO ===");
  Serial.println("Uptime: " + String(millis()/1000) + " segundos");
  Serial.println("WiFi Status: " + String(WiFi.status()));
  Serial.println("IP: " + WiFi.localIP().toString());
  Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
  Serial.println("Última temperatura: " + String(lastTemperature, 1) + "°C");
  Serial.println("Última umidade: " + String(lastHumidity, 1) + "%");
  Serial.println("Próximo envio em: " + String((SEND_INTERVAL - (millis() - lastSendTime))/1000) + " segundos");
  Serial.println("==================\n");
}

