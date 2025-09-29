#!/usr/bin/env python3
"""
Simulador contínuo de dados ESP8266 DHT22
Envia dados a cada minuto simulando o comportamento real do ESP8266
"""

import requests
import json
import time
import random
from datetime import datetime

# Configurações
API_URL = "http://localhost:5005/api/sensor-data"
DEVICES = [
    {"id": "ESP8266_001", "name": "Sensor Sala", "location": "Sala de Estar"},
    {"id": "ESP8266_002", "name": "Sensor Cozinha", "location": "Cozinha"},
    {"id": "ESP8266_003", "name": "Sensor Quarto", "location": "Quarto"}
]

def generate_realistic_data(device):
    """Gera dados realistas baseados no dispositivo"""
    # Diferentes perfis de temperatura por ambiente
    if "Sala" in device["location"]:
        base_temp = 24.0
        base_humidity = 55.0
    elif "Cozinha" in device["location"]:
        base_temp = 26.0  # Cozinha é mais quente
        base_humidity = 65.0  # Mais úmida
    else:  # Quarto
        base_temp = 22.0  # Quarto é mais fresco
        base_humidity = 50.0  # Menos úmido
    
    # Variação realista
    temp_variation = random.uniform(-2, 2)
    humidity_variation = random.uniform(-8, 8)
    
    temperature = round(base_temp + temp_variation, 1)
    humidity = round(base_humidity + humidity_variation, 1)
    
    return {
        "temperature": temperature,
        "humidity": humidity,
        "device_id": device["id"]
    }

def send_data(data, device):
    """Envia dados para a API"""
    try:
        response = requests.post(
            API_URL,
            json=data,
            headers={"Content-Type": "application/json"},
            timeout=5
        )
        
        if response.status_code == 200:
            timestamp = datetime.now().strftime("%H:%M:%S")
            print(f"[{timestamp}] {device['name']}: {data['temperature']}°C, {data['humidity']}%")
            return True
        else:
            print(f"ERRO {device['name']}: HTTP {response.status_code}")
            return False
            
    except requests.exceptions.RequestException as e:
        print(f"ERRO {device['name']}: {e}")
        return False

def main():
    """Simulação contínua"""
    print("Simulador Contínuo ESP8266 DHT22")
    print("=" * 50)
    print("Dispositivos configurados:")
    for device in DEVICES:
        print(f"  - {device['name']} ({device['id']}) - {device['location']}")
    
    print("\nIniciando simulação...")
    print("Pressione Ctrl+C para parar")
    print("=" * 50)
    
    try:
        while True:
            print(f"\n--- Enviando dados - {datetime.now().strftime('%H:%M:%S')} ---")
            
            for device in DEVICES:
                data = generate_realistic_data(device)
                send_data(data, device)
                time.sleep(0.5)  # Pequena pausa entre dispositivos
            
            print("Aguardando 60 segundos para próxima leitura...")
            time.sleep(60)
            
    except KeyboardInterrupt:
        print("\n\nSimulação interrompida pelo usuário")
        print("Acesse http://localhost:5005 para ver os dados no dashboard")

if __name__ == "__main__":
    main()
