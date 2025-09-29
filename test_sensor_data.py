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
DEVICE_IDS = ["ESP8266_001", "ESP8266_002", "ESP8266_003"]
LOCATIONS = ["Sala de Estar", "Cozinha", "Quarto"]

def generate_sensor_data(device_id, location):
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
        "device_id": device_id,
        "timestamp": datetime.now().isoformat()
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
            print(f"✅ Dados enviados com sucesso!")
            print(f"   Dispositivo: {data['device_id']}")
            print(f"   Temperatura: {data['temperature']}°C")
            print(f"   Umidade: {data['humidity']}%")
            print(f"   Resposta: {result.get('message', 'OK')}")
            return True
        else:
            print(f"❌ Erro HTTP {response.status_code}: {response.text}")
            return False
            
    except requests.exceptions.RequestException as e:
        print(f"❌ Erro de conexão: {e}")
        return False

def main():
    """Função principal"""
    print("🌡️  Simulador de Dados ESP8266 DHT22")
    print("=" * 50)
    
    # Teste único
    print("\n📡 Enviando dados de teste...")
    data = generate_sensor_data("ESP8266_001", "Sala de Estar")
    send_sensor_data(data)
    
    # Perguntar se quer simular múltiplos dispositivos
    print("\n" + "=" * 50)
    choice = input("Quer simular múltiplos dispositivos? (s/n): ").lower()
    
    if choice == 's':
        print("\n🔄 Simulando múltiplos dispositivos...")
        print("Pressione Ctrl+C para parar")
        
        try:
            while True:
                for i, device_id in enumerate(DEVICE_IDS):
                    data = generate_sensor_data(device_id, LOCATIONS[i])
                    print(f"\n📡 Enviando dados do {device_id}...")
                    send_sensor_data(data)
                    time.sleep(1)  # Pausa entre dispositivos
                
                print(f"\n⏰ Aguardando 30 segundos para próxima rodada...")
                time.sleep(30)
                
        except KeyboardInterrupt:
            print("\n\n🛑 Simulação interrompida pelo usuário")
    
    print("\n✅ Teste concluído!")

if __name__ == "__main__":
    main()
