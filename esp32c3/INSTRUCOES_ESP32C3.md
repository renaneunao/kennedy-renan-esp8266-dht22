# ESP32-C3 DHT22 + OLED Monitor com Configuração Web

## Hardware Necessário

- **ESP32-C3** (placa principal)
- **Sensor DHT22** (temperatura e umidade)
- **Display OLED SSD1306** (128x64 pixels)
- **Cabo USB-C** (para programação e alimentação)
- **Jumpers** para conexões

## 🆕 Funcionalidades Completas

- **Configuração Total via Web**: Sem necessidade de hardcoding
- **Modo AP**: Cria rede WiFi própria para configuração
- **Armazenamento Persistente**: Configurações salvas na memória flash
- **Fallback Automático**: Volta ao modo AP se WiFi falhar
- **Interface Completa**: Página web com TODOS os parâmetros
- **GPIOs Configuráveis**: Todos os pinos configuráveis via web
- **Intervalos Configuráveis**: Todos os tempos ajustáveis
- **OLED 0.96" Otimizado**: Configuração específica para display 0.96"
- **Sistema Robusto**: Debug, reconexão automática, reset

## Conexões

### Sensor DHT22
- **VCC** → 3.3V da ESP32-C3
- **GND** → GND da ESP32-C3
- **DATA** → GPIO4 da ESP32-C3

### Display OLED SSD1306
- **VCC** → 3.3V da ESP32-C3
- **GND** → GND da ESP32-C3
- **SDA** → GPIO6 da ESP32-C3
- **SCL** → GPIO7 da ESP32-C3

### LED de Status (opcional)
- **Anodo** → GPIO2 da ESP32-C3
- **Catodo** → GND da ESP32-C3 (com resistor 220Ω)

## Bibliotecas Necessárias

Instale as seguintes bibliotecas no Arduino IDE:

1. **DHT sensor library** (por Adafruit)
2. **Adafruit GFX Library**
3. **Adafruit SSD1306**
4. **ArduinoJson** (por Benoit Blanchon)
5. **WebServer** (já incluída no ESP32)
6. **SPIFFS** (já incluída no ESP32)
7. **Preferences** (já incluída no ESP32)

### Como instalar as bibliotecas:

1. Abra o Arduino IDE
2. Vá em **Ferramentas** → **Gerenciar Bibliotecas**
3. Procure por cada biblioteca e clique em **Instalar**

## Configuração do Arduino IDE

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

## 🚀 Como Usar o Sistema de Configuração

### 1. Primeira Configuração (Modo AP)

1. **Faça o upload do código** `esp32c3_dht22_oled_complete.ino`
2. **Ligue o dispositivo** - ele criará uma rede WiFi própria
3. **Conecte-se à rede**:
   - SSID: `ESP32-C3-Config`
   - Senha: `config123`
4. **Abra o navegador** e acesse: `http://192.168.4.1`
5. **Configure TODOS os parâmetros**:
   - **WiFi**: SSID e senha
   - **Dispositivo**: Nome, localização, descrição
   - **Servidor**: URL completa e chave API
   - **Hardware**: Todos os GPIOs (DHT22, OLED, LEDs)
   - **Intervalos**: Tempos de envio, leitura, display
   - **Sistema**: Debug, reconexão, tentativas
6. **Clique em "Salvar"** - o dispositivo reiniciará automaticamente

### 2. Modo Normal de Operação

Após a configuração inicial:
- O dispositivo conecta automaticamente ao WiFi configurado
- Envia dados para o servidor a cada 60 segundos
- Display OLED mostra informações em tempo real

### 3. Fallback para Modo AP

Se o WiFi falhar 3 vezes consecutivas:
- O dispositivo volta automaticamente ao modo AP
- Você pode reconfigurar sem reiniciar
- Timeout de 5 minutos no modo AP

## 📋 Parâmetros Configuráveis

### 🌐 Configuração WiFi
- **SSID**: Nome da rede WiFi
- **Senha**: Senha da rede WiFi
- **Timeout**: Tempo limite para conexão (5-120s)
- **Tentativas**: Máximo de tentativas de conexão (1-10)

### 🏷️ Informações do Dispositivo
- **Nome**: Identificação única do dispositivo
- **Localização**: Onde está instalado
- **Descrição**: Detalhes adicionais

### 🌐 Configuração do Servidor
- **URL**: Endpoint completo do servidor Flask
- **API Key**: Chave de autenticação
- **Intervalo de Envio**: Frequência de envio (1s-1h)

### 🔌 Hardware (GPIOs)
- **DHT22**: Pino do sensor (0-21)
- **OLED SDA**: Pino de dados I2C (0-21)
- **OLED SCL**: Pino de clock I2C (0-21)
- **LED**: Pino do LED de status (-1 a 21)
- **LED Status**: Pino do LED interno (-1 a 21)
- **OLED Address**: Endereço I2C (hex)

### ⏱️ Intervalos
- **Sensor**: Frequência de leitura (1-60s)
- **Display**: Atualização da tela (0.5-10s)
- **Envio**: Intervalo de envio (1s-1h)
- **WiFi Timeout**: Timeout de conexão (5-120s)

### ⚙️ Sistema
- **Debug**: Modo de depuração (on/off)
- **Reconexão**: Reconexão automática (on/off)
- **OLED**: Habilitar/desabilitar display
- **Reset**: Botão para resetar configurações

## Funcionalidades

### Display OLED 0.96"
O display mostra em tempo real:
- Status da conexão WiFi
- Endereço IP atual
- Temperatura e umidade
- Tempo para próximo envio
- Informações do modo AP

### Envio de Dados
- Envia dados a cada 60 segundos
- Reconexão automática em caso de falha WiFi
- Indicadores visuais de status

### Monitoramento Serial
- Logs detalhados no Monitor Serial (115200 baud)
- Diagnóstico de problemas
- Status de conexão

## Solução de Problemas

### Display não funciona
1. Verifique as conexões SDA (GPIO6) e SCL (GPIO7)
2. Confirme se o OLED está alimentado com 3.3V
3. Verifique se as bibliotecas estão instaladas

### Sensor DHT22 não lê
1. Verifique a conexão DATA (GPIO4)
2. Confirme alimentação 3.3V
3. Aguarde 2 segundos entre leituras (limitação do DHT22)

### WiFi não conecta
1. Verifique SSID e senha
2. Confirme se a rede está no alcance
3. Verifique se não há caracteres especiais na senha

### Upload falha
1. Pressione o botão BOOT durante o upload
2. Verifique se a placa está selecionada corretamente
3. Tente reduzir a velocidade de upload

## Diferenças da ESP8266

### Vantagens da ESP32-C3:
- **WiFi mais estável**
- **Mais memória RAM**
- **Processador mais rápido**
- **Suporte nativo a I2C**
- **Melhor para displays OLED**

### Mudanças no código:
- Bibliotecas WiFi diferentes (`WiFi.h` em vez de `ESP8266WiFi.h`)
- Pinos I2C configuráveis
- Melhor gerenciamento de energia
- Suporte aprimorado a displays

## Monitoramento

### Serial Monitor
Configure para 115200 baud para ver:
- Status de inicialização
- Leituras do sensor
- Status de conexão WiFi
- Logs de envio de dados

### Display OLED
Mostra informações em tempo real:
- Status do sistema
- Dados atuais
- Tempo para próximo envio

## Próximos Passos

1. **Teste básico:** Verifique se o sensor e display funcionam
2. **Configuração WiFi:** Configure suas credenciais
3. **Teste de envio:** Verifique se os dados chegam ao servidor
4. **Monitoramento:** Acompanhe os dados no dashboard web

## Suporte

Em caso de problemas:
1. Verifique as conexões
2. Confirme as bibliotecas instaladas
3. Teste com o Monitor Serial
4. Verifique a alimentação (3.3V)
