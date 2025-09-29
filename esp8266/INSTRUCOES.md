# 📡 Instruções para ESP8266 DHT22

## 🔧 Configuração Inicial

### 1. **Configurar WiFi**
```cpp
const char* ssid = "SEU_WIFI_SSID";        // Configure seu WiFi
const char* password = "SUA_SENHA_WIFI";   // Configure sua senha WiFi
```

### 2. **Configurar Device ID (para múltiplos sensores)**
```cpp
doc["device_id"] = "ESP8266_001";  // Mude para ESP8266_002, ESP8266_003, etc.
```

### 3. **URL do Servidor**
```cpp
const char* serverUrl = "http://194.163.142.108:5005/api/sensor-data";
```

## 🔌 Conexões do Hardware

```
DHT22 -> ESP8266
VCC   -> 3.3V
GND   -> GND
DATA  -> D4 (GPIO2)

LED (opcional) -> D2 (GPIO4)
```

## 📊 Dados Enviados

O ESP8266 envia dados a cada 1 minuto:

```json
{
  "temperature": 25.3,
  "humidity": 60.2,
  "device_id": "ESP8266_001",
  "timestamp": 1234567890
}
```

## 🚀 Como Usar

1. **Configure o WiFi** no código
2. **Faça upload** para o ESP8266
3. **Abra o Serial Monitor** (115200 baud)
4. **Acesse** `http://194.163.142.108:5005` para ver os dados

## 🔧 Para Múltiplos Sensores

1. **Copie o código** para cada ESP8266
2. **Mude o device_id**:
   - Sensor 1: `"ESP8266_001"`
   - Sensor 2: `"ESP8266_002"`
   - Sensor 3: `"ESP8266_003"`
   - etc.

## 📱 Monitoramento

- **Dashboard**: `http://194.163.142.108:5005`
- **Dispositivos**: `http://194.163.142.108:5005/devices`
- **Login**: `renaneunao` / `!Senhas123` ou `kennedy` / `kenn$132`

## 🐛 Troubleshooting

### WiFi não conecta
- Verifique SSID e senha
- Verifique se o WiFi suporta 2.4GHz
- Reinicie o ESP8266

### Dados não aparecem
- Verifique se o servidor está rodando
- Verifique a URL no código
- Abra o Serial Monitor para debug

### Sensor não lê
- Verifique as conexões
- DHT22 precisa de 2 segundos entre leituras
- Verifique se está alimentado com 3.3V
