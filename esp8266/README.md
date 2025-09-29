# ESP8266 DHT22 Monitor - Firmware

Firmware para ESP8266 que monitora temperatura e umidade usando sensor DHT22 e envia dados para servidor Flask.

## 📋 Hardware Necessário

- **NodeMCU ESP8266** ou similar
- **Sensor DHT22** (AM2302)
- **Resistor pull-up 10kΩ** (opcional, alguns DHT22 já incluem)
- **LED** (opcional, para indicador de status)
- **Breadboard e jumpers**

## 🔌 Conexões

| Componente | ESP8266 | Observações |
|------------|---------|-------------|
| DHT22 VCC  | 3.3V    | Alimentação |
| DHT22 GND  | GND      | Terra       |
| DHT22 DATA | D4 (GPIO2) | Dados do sensor |
| LED Anodo  | D2 (GPIO4) | Opcional |
| LED Catodo | GND      | Opcional |

## ⚙️ Configuração

### 1. Instalar Bibliotecas

No Arduino IDE, instale as seguintes bibliotecas:

```
- ESP8266WiFi (já incluída)
- DHT sensor library (Adafruit)
- ArduinoJson (Benoit Blanchon)
- ESP8266HTTPClient (já incluída)
```

### 2. Configurar Credenciais

Edite o arquivo `config.h` com suas configurações:

```cpp
// WiFi
#define WIFI_SSID "SEU_WIFI_SSID"
#define WIFI_PASSWORD "SUA_SENHA_WIFI"

// Servidor
#define SERVER_URL "http://194.163.142.108:5005/api/sensor-data"
#define API_KEY "sua_api_key_secreta"

// Dispositivo
#define DEVICE_ID "ESP8266_001"
```

### 3. Configurar Arduino IDE

1. **Placa**: NodeMCU 1.0 (ESP-12E Module)
2. **CPU Frequency**: 80MHz
3. **Flash Size**: 4MB (FS:2MB OTA:~1019KB)
4. **Upload Speed**: 115200
5. **Port**: Selecione a porta COM do seu ESP8266

### 4. Upload do Código

1. Conecte o ESP8266 ao computador via USB
2. Selecione a porta correta no Arduino IDE
3. Clique em "Upload"
4. Aguarde a compilação e upload

## 🔧 Configurações Avançadas

### Intervalos de Envio

```cpp
#define SEND_INTERVAL 60000  // 1 minuto (60000ms)
#define SENSOR_READ_INTERVAL 10000  // 10 segundos
```

### Debug

```cpp
#define DEBUG_SERIAL true   // Debug no Serial Monitor
#define DEBUG_HTTP false    // Debug HTTP
#define DEBUG_SENSOR true    // Debug do sensor
```

### Economia de Energia

```cpp
#define ENABLE_DEEP_SLEEP false  // Deep sleep (desabilita WiFi)
#define DEEP_SLEEP_DURATION 60000  // 1 minuto
```

## 📊 Monitoramento

### Serial Monitor

Configure o Serial Monitor para 115200 baud para ver os logs:

```
=== ESP8266 DHT22 Monitor ===
Iniciando sistema...
Sensor DHT22 inicializado
Conectando ao WiFi: SEU_WIFI_SSID
WiFi conectado!
IP: 192.168.1.100
RSSI: -45 dBm
Sistema inicializado com sucesso!
Enviando dados a cada 60 segundos
Servidor: http://194.163.142.108:5005/api/sensor-data
===============================

Dados do sensor:
  Temperatura: 25.6°C
  Umidade: 60.2%
  Timestamp: 12345678
Enviando dados para o servidor...
JSON: {"temperature":25.6,"humidity":60.2,"device_id":"ESP8266_001","timestamp":12345678}
Resposta do servidor:
  Código: 200
  Resposta: {"status":"success","message":"Dados salvos com sucesso","id":123}
✅ Dados enviados com sucesso!
```

### LEDs de Status

- **LED Interno (azul)**: Pisca quando WiFi desconectado
- **LED Externo (opcional)**: Pisca quando dados enviados com sucesso

## 🔧 Troubleshooting

### Problemas Comuns

1. **"Erro na leitura do sensor DHT22!"**
   - Verifique as conexões do sensor
   - Teste com resistor pull-up 10kΩ
   - Verifique se o sensor não está danificado

2. **"Falha na conexão WiFi!"**
   - Verifique SSID e senha
   - Verifique força do sinal WiFi
   - Teste com outro roteador

3. **"Erro na conexão: -1"**
   - Verifique URL do servidor
   - Teste conectividade de rede
   - Verifique se o servidor está rodando

4. **"Erro no servidor: 400/500"**
   - Verifique formato do JSON
   - Verifique API key
   - Verifique logs do servidor

### Diagnóstico

Para diagnóstico completo, adicione esta função ao loop:

```cpp
void loop() {
  // ... código existente ...
  
  // Diagnóstico a cada 5 minutos
  static unsigned long lastDiag = 0;
  if (millis() - lastDiag >= 300000) {
    printDiagnostics();
    lastDiag = millis();
  }
}
```

## 🔄 Atualizações OTA

Para atualizações Over-The-Air, adicione:

```cpp
#include <ESP8266httpUpdate.h>

void checkForUpdates() {
  String updateUrl = "http://seu-servidor.com/firmware.bin";
  
  t_httpUpdate_return ret = httpUpdate.update(client, updateUrl);
  
  switch(ret) {
    case HTTP_UPDATE_FAILED:
      Serial.println("Falha na atualização");
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("Nenhuma atualização disponível");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("Atualização concluída");
      break;
  }
}
```

## 🔐 Segurança

### API Key

Configure uma API key forte no servidor e no ESP8266:

```cpp
#define API_KEY "sua_api_key_muito_segura_aqui"
```

### HTTPS (Futuro)

Para HTTPS, configure:

```cpp
#define ENABLE_SSL true
#define SSL_FINGERPRINT "AA:BB:CC:DD:EE:FF:00:11:22:33:44:55:66:77:88:99"
```

## 📈 Otimizações

### Memória

- Use `String` ao invés de `char*` para economizar RAM
- Limite o tamanho do JSON
- Use `PROGMEM` para strings constantes

### Energia

- Use deep sleep para economia de bateria
- Reduza frequência de envio
- Desabilite debug em produção

### Rede

- Implemente retry com backoff exponencial
- Use timeouts apropriados
- Monitore qualidade do sinal

## 🚀 Deploy em Produção

1. **Configure credenciais** no `config.h`
2. **Desabilite debug** em produção
3. **Teste conectividade** antes do deploy
4. **Configure monitoramento** do dispositivo
5. **Implemente backup** de configuração

## 📞 Suporte

Para problemas específicos:

1. Verifique logs no Serial Monitor
2. Teste conectividade de rede
3. Verifique configurações do servidor
4. Consulte documentação das bibliotecas

---

**⚠️ Importante**: Sempre teste o código em ambiente de desenvolvimento antes de fazer deploy em produção!

