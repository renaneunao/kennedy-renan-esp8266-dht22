# ESP32-C3 DHT22 + OLED Monitor

Sistema de monitoramento de temperatura e umidade com display OLED 0.96" e configuração web completa.

## 📁 Estrutura de Arquivos

```
esp32c3/
├── esp32c3_dht22_oled_complete.ino    # Código completo com configuração web
├── esp32c3_dht22_oled_config.ino     # Versão com configuração básica
├── esp32c3_dht22_oled.ino            # Versão simples sem configuração web
├── INSTRUCOES_ESP32C3.md             # Instruções detalhadas
└── README.md                         # Este arquivo
```

## 🚀 Versões Disponíveis

### 1. **esp32c3_dht22_oled_complete.ino** (RECOMENDADO)
- ✅ Configuração web completa
- ✅ Todos os GPIOs configuráveis
- ✅ Intervalos configuráveis
- ✅ Servidor configurável
- ✅ OLED 0.96" otimizado
- ✅ Sistema robusto com fallback

### 2. **esp32c3_dht22_oled_config.ino**
- ✅ Configuração web básica
- ✅ WiFi e servidor configuráveis
- ✅ OLED 0.96" básico

### 3. **esp32c3_dht22_oled.ino**
- ✅ Código simples
- ❌ Configuração hardcoded
- ✅ OLED 0.96" básico

## 🔧 Hardware Necessário

- **ESP32-C3** (placa principal)
- **Sensor DHT22** (temperatura e umidade)
- **Display OLED SSD1306 0.96"** (128x64 pixels)
- **Cabo USB-C** (para programação e alimentação)
- **Jumpers** para conexões

## 🔌 Conexões Padrão

### Sensor DHT22
- **VCC** → 3.3V da ESP32-C3
- **GND** → GND da ESP32-C3
- **DATA** → GPIO4 da ESP32-C3

### Display OLED 0.96"
- **VCC** → 3.3V da ESP32-C3
- **GND** → GND da ESP32-C3
- **SDA** → GPIO6 da ESP32-C3
- **SCL** → GPIO7 da ESP32-C3

### LED de Status (opcional)
- **Anodo** → GPIO2 da ESP32-C3
- **Catodo** → GND da ESP32-C3 (com resistor 220Ω)

## 📚 Bibliotecas Necessárias

Instale as seguintes bibliotecas no Arduino IDE:

1. **DHT sensor library** (por Adafruit)
2. **Adafruit GFX Library**
3. **Adafruit SSD1306**
4. **ArduinoJson** (por Benoit Blanchon)
5. **WebServer** (já incluída no ESP32)
6. **SPIFFS** (já incluída no ESP32)
7. **Preferences** (já incluída no ESP32)

## ⚙️ Configuração do Arduino IDE

1. **Instalar ESP32 Board Package:**
   - Vá em **Arquivo** → **Preferências**
   - Em "URLs Adicionais para Gerenciadores de Placas", adicione:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Vá em **Ferramentas** → **Placa** → **Gerenciador de Placas**
   - Procure por "ESP32" e instale o pacote

2. **Configurar a Placa:**
   - Vá em **Ferramentas** → **Placa** → **ESP32 Arduino** → **ESP32C3 Dev Module**
   - Configure as seguintes opções:
     - **Upload Speed:** 921600
     - **CPU Frequency:** 160MHz
     - **Flash Mode:** QIO
     - **Flash Size:** 4MB
     - **Partition Scheme:** Default 4MB with spiffs
     - **Core Debug Level:** None

## 🚀 Como Usar

### Para Configuração Web (Recomendado):

1. **Faça o upload** do arquivo `esp32c3_dht22_oled_complete.ino`
2. **Ligue o dispositivo** - ele criará uma rede WiFi própria
3. **Conecte-se à rede**:
   - SSID: `ESP32-C3-Config`
   - Senha: `config123`
4. **Abra o navegador** e acesse: `http://192.168.4.1`
5. **Configure todos os parâmetros**:
   - WiFi, dispositivo, servidor, hardware, intervalos
6. **Clique em "Salvar"** - o dispositivo reiniciará automaticamente

### Para Uso Simples:

1. **Edite o código** `esp32c3_dht22_oled.ino` com suas configurações
2. **Faça o upload** para a ESP32-C3
3. **Ligue o dispositivo** - ele funcionará automaticamente

## 📋 Parâmetros Configuráveis (Versão Complete)

### 🌐 WiFi & Rede
- SSID e senha WiFi
- Timeout de conexão (5-120s)
- Máximo de tentativas (1-10)

### 🏷️ Dispositivo
- Nome único do dispositivo
- Localização física
- Descrição detalhada

### 🌐 Servidor
- URL completa do servidor Flask
- Chave de autenticação API
- Intervalo de envio (1s-1h)

### 🔌 Hardware (GPIOs)
- Pino do DHT22 (0-21)
- Pinos OLED SDA/SCL (0-21)
- Pinos dos LEDs (-1 a 21)
- Endereço I2C do OLED (hex)

### ⏱️ Intervalos
- Frequência de leitura do sensor (1-60s)
- Atualização do display (0.5-10s)
- Intervalo de envio (1s-1h)
- Timeout WiFi (5-120s)

### ⚙️ Sistema
- Modo debug (on/off)
- Reconexão automática (on/off)
- OLED habilitado (on/off)
- Reset de configurações

## 🔄 Fluxo de Funcionamento

```
Ligar → Modo AP → Configurar → Salvar → Reiniciar → Conectar WiFi → Monitorar
```

Se WiFi falhar → Volta ao Modo AP → Reconfigurar

## 📱 Display OLED 0.96"

### Modo Normal:
- Status da conexão WiFi
- Endereço IP atual
- Temperatura e umidade
- Tempo para próximo envio

### Modo AP:
- Nome da rede: `ESP32-C3-Config`
- IP: `192.168.4.1`
- Instruções de conexão
- Contador de timeout

## 🛠️ Solução de Problemas

### Display não funciona
1. Verifique as conexões SDA (GPIO6) e SCL (GPIO7)
2. Confirme se o OLED está alimentado com 3.3V
3. Verifique se as bibliotecas estão instaladas

### Sensor DHT22 não lê
1. Verifique a conexão DATA (GPIO4)
2. Confirme alimentação 3.3V
3. Aguarde 2 segundos entre leituras

### WiFi não conecta
1. Verifique SSID e senha
2. Confirme se a rede está no alcance
3. Use o modo AP para reconfigurar

### Upload falha
1. Pressione o botão BOOT durante o upload
2. Verifique se a placa está selecionada corretamente
3. Tente reduzir a velocidade de upload

## 📊 Monitoramento

### Serial Monitor (115200 baud)
- Status de inicialização
- Leituras do sensor
- Status de conexão WiFi
- Logs de envio de dados

### Display OLED
- Informações em tempo real
- Status do sistema
- Dados atuais

## 🔗 Integração com Servidor Flask

O sistema envia dados para o endpoint:
```
POST /api/sensor-data
```

Com formato JSON:
```json
{
  "temperature": 25.5,
  "humidity": 60.2,
  "device_id": "ESP32C3_001",
  "timestamp": 1234567890
}
```

## 📈 Próximos Passos

1. **Teste básico**: Verifique se o sensor e display funcionam
2. **Configuração WiFi**: Configure suas credenciais
3. **Teste de envio**: Verifique se os dados chegam ao servidor
4. **Monitoramento**: Acompanhe os dados no dashboard web

## 🆘 Suporte

Em caso de problemas:
1. Verifique as conexões
2. Confirme as bibliotecas instaladas
3. Teste com o Monitor Serial
4. Verifique a alimentação (3.3V)
5. Use o modo AP para reconfigurar
