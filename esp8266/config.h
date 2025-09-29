/*
 * Configurações do ESP8266 DHT22 Monitor
 * 
 * ATENÇÃO: Configure suas credenciais antes de fazer upload!
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===========================================
// CONFIGURAÇÕES DE REDE WIFI
// ===========================================
#define WIFI_SSID "SEU_WIFI_SSID_AQUI"
#define WIFI_PASSWORD "SUA_SENHA_WIFI_AQUI"

// ===========================================
// CONFIGURAÇÕES DO SERVIDOR
// ===========================================
#define SERVER_URL "http://194.163.142.108:5005/api/sensor-data"
#define API_KEY "sua_api_key_secreta_aqui"

// ===========================================
// CONFIGURAÇÕES DO SENSOR
// ===========================================
#define DHT_PIN D4              // Pino do sensor DHT22
#define DHT_TYPE DHT22          // Tipo do sensor
#define SENSOR_READ_INTERVAL 10000  // Intervalo entre leituras (ms)
#define SENSOR_TIMEOUT 5000     // Timeout para leitura (ms)

// ===========================================
// CONFIGURAÇÕES DE ENVIO
// ===========================================
#define SEND_INTERVAL 60000     // Intervalo entre envios (ms) - 1 minuto
#define MAX_RETRIES 3           // Máximo de tentativas de envio
#define RETRY_DELAY 5000        // Delay entre tentativas (ms)

// ===========================================
// CONFIGURAÇÕES DE HARDWARE
// ===========================================
#define LED_PIN D2              // LED de status (opcional)
#define STATUS_LED_PIN LED_BUILTIN  // LED interno do ESP8266
#define WIFI_TIMEOUT 30000       // Timeout para conexão WiFi (ms)

// ===========================================
// CONFIGURAÇÕES DE DEBUG
// ===========================================
#define DEBUG_SERIAL true       // Habilitar debug no Serial
#define DEBUG_HTTP false        // Habilitar debug HTTP
#define DEBUG_SENSOR true       // Habilitar debug do sensor

// ===========================================
// CONFIGURAÇÕES AVANÇADAS
// ===========================================
#define DEVICE_ID "ESP8266_001" // ID único do dispositivo
#define FIRMWARE_VERSION "1.0.0" // Versão do firmware
#define USER_AGENT "ESP8266-DHT22/1.0"

// ===========================================
// CONFIGURAÇÕES DE ECONOMIA DE ENERGIA
// ===========================================
#define ENABLE_DEEP_SLEEP false // Habilitar deep sleep (desabilita WiFi)
#define DEEP_SLEEP_DURATION 60000 // Duração do deep sleep (ms)

// ===========================================
// CONFIGURAÇÕES DE SEGURANÇA
// ===========================================
#define ENABLE_SSL false        // Habilitar HTTPS (requer certificado)
#define SSL_FINGERPRINT ""      // Fingerprint do certificado SSL

// ===========================================
// CONFIGURAÇÕES DE ALERTAS
// ===========================================
#define TEMP_MIN 0.0            // Temperatura mínima (°C)
#define TEMP_MAX 50.0           // Temperatura máxima (°C)
#define HUMIDITY_MIN 0.0        // Umidade mínima (%)
#define HUMIDITY_MAX 100.0      // Umidade máxima (%)

// ===========================================
// CONFIGURAÇÕES DE BACKUP
// ===========================================
#define ENABLE_LOCAL_STORAGE false // Armazenar dados localmente
#define MAX_LOCAL_RECORDS 100   // Máximo de registros locais

#endif // CONFIG_H

