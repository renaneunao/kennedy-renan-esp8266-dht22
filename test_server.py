#!/usr/bin/env python3
"""
Script para testar POSTs no servidor de produção
"""

import requests
import json
import time
import random
from datetime import datetime

# Configurações do servidor
SERVER_URL = "http://194.163.142.108:5006/api/sensor-data"

def send_sensor_data(device_id, temperature, humidity):
    """Envia dados do sensor para o servidor"""
    data = {
        "temperature": temperature,
        "humidity": humidity,
        "device_id": device_id,
        "timestamp": int(time.time())
    }
    
    headers = {
        "Content-Type": "application/json",
        "X-API-Key": "esp8266-dht22-key",
        "User-Agent": "ESP8266-DHT22/1.0"
    }
    
    try:
        response = requests.post(SERVER_URL, json=data, headers=headers)
        if response.status_code == 200:
            result = response.json()
            print(f"✅ {device_id}: {temperature}°C, {humidity}% - {result.get('message', 'OK')}")
            return True
        else:
            print(f"❌ {device_id}: Erro {response.status_code} - {response.text}")
            return False
    except Exception as e:
        print(f"❌ {device_id}: Erro de conexão - {e}")
        return False

def generate_realistic_data():
    """Gera dados realistas de temperatura e umidade"""
    # Simular variação de temperatura (20-30°C)
    base_temp = 25.0
    variation = random.uniform(-3, 3)
    temperature = round(base_temp + variation, 1)
    
    # Simular variação de umidade (40-80%)
    base_humidity = 60.0
    variation = random.uniform(-10, 10)
    humidity = round(base_humidity + variation, 1)
    
    return temperature, humidity

def main():
    print("Testando envio de dados para o servidor de produção")
    print("=" * 60)
    
    devices = ["ESP8266_001", "ESP8266_002", "ESP8266_003"]
    
    for i in range(5):  # Enviar 5 rodadas de dados
        print(f"\n--- Rodada {i+1} ---")
        
        for device_id in devices:
            temp, humidity = generate_realistic_data()
            send_sensor_data(device_id, temp, humidity)
            time.sleep(1)  # Pausa de 1 segundo entre dispositivos
        
        if i < 4:  # Não pausar na última rodada
            print("Aguardando 3 segundos...")
            time.sleep(3)
    
    print("\n✅ Teste concluído!")
    print(f"🌐 Acesse: http://194.163.142.108:5006")

if __name__ == "__main__":
    main()

