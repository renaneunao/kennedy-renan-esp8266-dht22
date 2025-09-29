#!/usr/bin/env python3
"""
Script para simular dados do ESP8266 DHT22
Testa o endpoint /api/sensor-data enviando dados de temperatura e umidade
"""

import requests
import json
import time
import random
from datetime import datetime

# Configurações
API_URL = "http://localhost:5005/api/sensor-data"

def generate_sensor_data(device_id):
    """Gera dados simulados do sensor DHT22"""
    # Simular variação de temperatura (20-30°C)
    base_temp = 25.0
    temp_variation = random.uniform(-3, 3)
    temperature = round(base_temp + temp_variation, 1)
    
    # Simular variação de umidade (40-80%)
    base_humidity = 60.0
    humidity_variation = random.uniform(-10, 10)
    humidity = round(base_humidity + humidity_variation, 1)
    
    return {
        "temperature": temperature,
        "humidity": humidity,
        "device_id": device_id
    }

def send_sensor_data(data):
    """Envia dados para a API"""
    try:
        response = requests.post(
            API_URL,
            json=data,
            headers={"Content-Type": "application/json"},
            timeout=5
        )
        
        if response.status_code == 200:
            result = response.json()
            print(f"SUCESSO: Dados enviados!")
            print(f"  Dispositivo: {data['device_id']}")
            print(f"  Temperatura: {data['temperature']}°C")
            print(f"  Umidade: {data['humidity']}%")
            print(f"  Resposta: {result.get('message', 'OK')}")
            return True
        else:
            print(f"ERRO HTTP {response.status_code}: {response.text}")
            return False
            
    except requests.exceptions.RequestException as e:
        print(f"ERRO de conexao: {e}")
        return False

def main():
    """Função principal"""
    print("Simulador de Dados ESP8266 DHT22")
    print("=" * 50)
    
    # Teste único
    print("\nEnviando dados de teste...")
    data = generate_sensor_data("ESP8266_001")
    send_sensor_data(data)
    
    # Teste com múltiplos dispositivos
    print("\n" + "=" * 50)
    print("Testando multiplos dispositivos...")
    
    devices = ["ESP8266_001", "ESP8266_002", "ESP8266_003"]
    
    for device in devices:
        print(f"\nEnviando dados do {device}...")
        data = generate_sensor_data(device)
        send_sensor_data(data)
        time.sleep(1)
    
    print("\nTeste concluido!")
    print("Acesse http://localhost:5005 para ver os dados no dashboard")

if __name__ == "__main__":
    main()
