# ESP8266/ESP32-C3 DHT22 Monitor - Sistema de Monitoramento IoT

Sistema completo de monitoramento de temperatura e umidade usando ESP8266 ou ESP32-C3 com sensor DHT22, conectado a uma aplicação web Flask que armazena e visualiza os dados em tempo real.

## 📋 Visão Geral

Este projeto consiste em três partes principais:

1. **Aplicação Web Flask** - Interface web para visualização dos dados
2. **Código ESP8266** - Firmware para ESP8266 com sensor DHT22
3. **Código ESP32-C3** - Firmware avançado para ESP32-C3 com OLED e configuração web

## 🏗️ Arquitetura

```
ESP8266/ESP32-C3 + DHT22 → WiFi → Flask API → SQLite → Interface Web
```

- **ESP8266**: Coleta dados do sensor DHT22 (configuração hardcoded)
- **ESP32-C3**: Coleta dados com OLED 0.96" e configuração web completa
- **Flask API**: Recebe dados via HTTP POST e armazena no SQLite
- **Interface Web**: Dashboard para visualização dos dados em tempo real
- **Docker**: Containerização para deploy fácil
- **GitHub Actions**: CI/CD automático

## 🚀 Tecnologias Utilizadas

### Backend
- **Flask** - Framework web Python
- **SQLite** - Banco de dados local
- **SQLAlchemy** - ORM para Python
- **Gunicorn** - Servidor WSGI para produção

### Frontend
- **HTML5/CSS3** - Interface responsiva
- **JavaScript** - Interatividade e gráficos
- **Chart.js** - Visualização de dados em tempo real
- **Bootstrap** - Framework CSS responsivo

### IoT
- **ESP8266** - Microcontrolador WiFi (versão básica)
- **ESP32-C3** - Microcontrolador WiFi avançado com OLED
- **DHT22** - Sensor de temperatura e umidade
- **OLED SSD1306 0.96"** - Display para ESP32-C3
- **Arduino IDE** - Desenvolvimento do firmware

### DevOps
- **Docker** - Containerização
- **Docker Compose** - Orquestração de containers
- **GitHub Actions** - CI/CD automático
- **Docker Hub** - Registry de imagens

## 📁 Estrutura do Projeto

```
kennedy-renan-esp8266-sht22/
├── README.md
├── docker-compose.yml
├── .github/
│   └── workflows/
│       └── docker-build.yml
├── app/                          # Aplicação Flask
│   ├── app.py
│   ├── models.py
│   ├── requirements.txt
│   ├── static/
│   ├── templates/
│   └── database/
├── esp8266/                      # Firmware ESP8266
│   ├── esp8266_dht22.ino
│   ├── config.h
│   ├── INSTRUCOES.md
│   └── README.md
└── esp32c3/                      # Firmware ESP32-C3
    ├── esp32c3_dht22_oled_complete.ino
    ├── esp32c3_dht22_oled_config.ino
    ├── esp32c3_dht22_oled.ino
    ├── INSTRUCOES_ESP32C3.md
    └── README.md
```

## 🔧 Configuração e Instalação

### Pré-requisitos

- Python 3.8+
- Docker e Docker Compose
- Arduino IDE (para ESP8266/ESP32-C3)
- **ESP8266**: NodeMCU ESP8266 + Sensor DHT22
- **ESP32-C3**: ESP32-C3 + Sensor DHT22 + OLED SSD1306 0.96"

### 1. Clone o Repositório

```bash
git clone https://github.com/renaneunao/esp8266-dht22.git
cd esp8266-dht22
```

### 2. Configuração da Aplicação Web

```bash
cd app
pip install -r requirements.txt
python app.py
```

A aplicação estará disponível em: `http://localhost:5005`

### 3. Configuração do Hardware

#### Para ESP8266 (Básico):
1. Abra o Arduino IDE
2. Instale as bibliotecas necessárias:
   - ESP8266WiFi
   - DHT sensor library
   - ArduinoJson
3. Configure as credenciais WiFi no arquivo `esp8266/config.h`
4. Configure o endpoint da API no código
5. Faça upload do código para o ESP8266

#### Para ESP32-C3 (Avançado):
1. Abra o Arduino IDE
2. Instale as bibliotecas necessárias:
   - DHT sensor library
   - Adafruit GFX Library
   - Adafruit SSD1306
   - ArduinoJson
3. **Use o código `esp32c3_dht22_oled_complete.ino`** (recomendado)
4. Faça upload do código para o ESP32-C3
5. **Configure via web**: Conecte-se à rede `ESP32-C3-Config` e acesse `http://192.168.4.1`

### 4. Deploy com Docker

```bash
# Build e execução com Docker Compose
docker-compose up -d

# Verificar logs
docker-compose logs -f
```

## 📊 API Endpoints

### POST /api/sensor-data
Recebe dados do sensor ESP8266

**Payload:**
```json
{
  "temperature": 25.6,
  "humidity": 60.2,
  "timestamp": "2024-01-15T10:30:00Z",
  "device_id": "ESP8266_001"
}
```

**Resposta:**
```json
{
  "status": "success",
  "message": "Data saved successfully"
}
```

### GET /api/sensor-data
Retorna dados históricos do sensor

**Parâmetros:**
- `limit`: Número de registros (padrão: 100)
- `device_id`: ID do dispositivo (opcional)

### GET /api/sensor-data/latest
Retorna o último registro de cada sensor

## 🎯 Funcionalidades

### Dashboard Web
- ✅ Visualização em tempo real dos dados
- ✅ Gráficos históricos de temperatura e umidade
- ✅ Múltiplos sensores suportados
- ✅ Interface responsiva
- ✅ Exportação de dados (CSV/JSON)

### Sistema IoT - ESP8266 (Básico)
- ✅ Coleta automática de dados
- ✅ Reconexão automática WiFi
- ✅ Envio de dados via HTTP POST
- ✅ Indicadores LED de status
- ❌ Configuração hardcoded

### Sistema IoT - ESP32-C3 (Avançado)
- ✅ Coleta automática de dados (configurável)
- ✅ Reconexão automática WiFi
- ✅ Envio de dados via HTTP POST
- ✅ Indicadores LED de status
- ✅ **Configuração web completa**
- ✅ **Display OLED 0.96" em tempo real**
- ✅ **GPIOs configuráveis**
- ✅ **Intervalos configuráveis**
- ✅ **Servidor configurável**
- ✅ **Sistema robusto com fallback**

### Monitoramento
- ✅ Alertas por email/SMS (futuro)
- ✅ Limites de temperatura/umidade
- ✅ Logs de sistema
- ✅ Backup automático de dados

## 🔐 Configuração de Segurança

### Variáveis de Ambiente

Crie um arquivo `.env` na raiz do projeto:

```env
FLASK_ENV=production
SECRET_KEY=sua_chave_secreta_aqui
DATABASE_URL=sqlite:///database/sensor_data.db
API_KEY=sua_api_key_para_esp8266
```

### Autenticação API

O ESP8266 deve incluir uma API key nos headers:

```cpp
http.addHeader("X-API-Key", "sua_api_key_aqui");
```

## 🚀 Deploy em Produção

### 1. Configuração do Servidor

```bash
# Clone o repositório no servidor
git clone https://github.com/seu-usuario/kennedy-renan-esp8266-sht22.git
cd kennedy-renan-esp8266-sht22

# Configure as variáveis de ambiente
cp .env.example .env
nano .env

# Execute com Docker Compose
docker-compose up -d
```

### 2. Configuração do Nginx (Opcional)

```nginx
server {
    listen 80;
    server_name seu-dominio.com;
    
    location / {
        proxy_pass http://localhost:5005;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

### 3. SSL/HTTPS (Recomendado)

```bash
# Instalar Certbot
sudo apt install certbot python3-certbot-nginx

# Obter certificado SSL
sudo certbot --nginx -d seu-dominio.com
```

## 📈 Monitoramento e Logs

### Logs da Aplicação

```bash
# Ver logs em tempo real
docker-compose logs -f app

# Logs específicos
docker-compose logs app | grep ERROR
```

### Métricas do Sistema

- CPU e memória dos containers
- Uso de disco do banco de dados
- Status de conectividade dos sensores
- Taxa de sucesso das requisições

## 🔧 Manutenção

### Backup do Banco de Dados

```bash
# Backup manual
docker-compose exec app python -c "
import sqlite3
import shutil
shutil.copy('database/sensor_data.db', 'backup/sensor_data_$(date +%Y%m%d).db')
"

# Backup automático (cron)
0 2 * * * cd /path/to/project && docker-compose exec app python backup.py
```

### Limpeza de Dados Antigos

```bash
# Script de limpeza (manter últimos 30 dias)
docker-compose exec app python cleanup.py --days 30
```

## 🐛 Troubleshooting

### Problemas Comuns

1. **ESP8266 não conecta ao WiFi**
   - Verificar credenciais no `config.h`
   - Verificar força do sinal WiFi
   - Resetar o dispositivo

2. **Dados não aparecem no dashboard**
   - Verificar logs da aplicação
   - Testar endpoint da API
   - Verificar conectividade do ESP8266

3. **Aplicação não inicia**
   - Verificar logs do Docker
   - Verificar portas disponíveis
   - Verificar permissões de arquivo

### Logs de Debug

```bash
# Ativar logs detalhados
export FLASK_DEBUG=1
docker-compose up
```

## 🤝 Contribuição

1. Fork o projeto
2. Crie uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request no [repositório](https://github.com/renaneunao/esp8266-dht22)

## 📄 Licença

Este projeto está sob a licença MIT. Veja o arquivo `LICENSE` para mais detalhes.

## 👥 Autores

- **Renan Kennedy** - Desenvolvimento inicial
- **Contribuidores** - Lista de contribuidores

## 📞 Suporte

- **Email**: renan@exemplo.com
- **Issues**: [GitHub Issues](https://github.com/renaneunao/esp8266-dht22/issues)
- **Documentação**: [Wiki do Projeto](https://github.com/renaneunao/esp8266-dht22/wiki)

## 🗺️ Roadmap

### Versão 1.1
- [ ] Sistema de alertas por email
- [ ] Múltiplos sensores por dispositivo
- [ ] API REST completa

### Versão 1.2
- [ ] Interface mobile (PWA)
- [ ] Notificações push
- [ ] Integração com Home Assistant

### Versão 2.0
- [ ] Machine Learning para previsões
- [ ] Dashboard avançado com widgets
- [ ] Sistema de usuários e permissões

---

**⚠️ Importante**: Este projeto está em desenvolvimento ativo. Para uso em produção, configure adequadamente as variáveis de ambiente e implemente as medidas de segurança necessárias.
