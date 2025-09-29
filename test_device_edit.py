#!/usr/bin/env python3
"""
Script para testar a edição de dispositivos
"""

import requests
import json

# Configurações
API_URL = "http://localhost:5005/api/devices"

def test_get_device(device_id):
    """Testa buscar dados de um dispositivo"""
    print(f"Testando GET /api/devices/{device_id}")
    try:
        response = requests.get(f"{API_URL}/{device_id}")
        if response.status_code == 200:
            data = response.json()
            print(f"✅ Sucesso: {data}")
            return data['data']
        else:
            print(f"❌ Erro HTTP {response.status_code}: {response.text}")
            return None
    except Exception as e:
        print(f"❌ Erro: {e}")
        return None

def test_update_device(device_id, update_data):
    """Testa atualizar um dispositivo"""
    print(f"Testando PUT /api/devices/{device_id}")
    try:
        response = requests.put(
            f"{API_URL}/{device_id}",
            json=update_data,
            headers={"Content-Type": "application/json"}
        )
        if response.status_code == 200:
            data = response.json()
            print(f"✅ Sucesso: {data}")
            return True
        else:
            print(f"❌ Erro HTTP {response.status_code}: {response.text}")
            return False
    except Exception as e:
        print(f"❌ Erro: {e}")
        return False

def main():
    print("Testando edição de dispositivos")
    print("=" * 50)
    
    # Teste 1: Buscar dispositivo
    device_data = test_get_device("ESP8266_001")
    if not device_data:
        print("❌ Não foi possível buscar o dispositivo")
        return
    
    # Teste 2: Atualizar dispositivo
    update_data = {
        "name": "Sensor Sala de Estar",
        "location": "Sala de Estar",
        "description": "Sensor de temperatura e umidade na sala de estar",
        "is_active": True
    }
    
    success = test_update_device("ESP8266_001", update_data)
    if success:
        print("\n✅ Teste de atualização bem-sucedido!")
        
        # Teste 3: Verificar se foi atualizado
        print("\nVerificando se a atualização foi salva...")
        updated_data = test_get_device("ESP8266_001")
        if updated_data:
            print(f"Nome: {updated_data['name']}")
            print(f"Localização: {updated_data['location']}")
            print(f"Descrição: {updated_data['description']}")
            print(f"Ativo: {updated_data['is_active']}")
    else:
        print("\n❌ Teste de atualização falhou!")

if __name__ == "__main__":
    main()
