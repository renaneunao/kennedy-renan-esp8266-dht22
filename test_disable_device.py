#!/usr/bin/env python3
"""
Script para testar desabilitar um dispositivo
"""

import requests
import json

# Configurações
API_URL = "http://localhost:5005/api/devices"

def disable_device(device_id):
    """Desabilita um dispositivo"""
    print(f"Desabilitando dispositivo {device_id}")
    try:
        response = requests.put(
            f"{API_URL}/{device_id}",
            json={"is_active": False},
            headers={"Content-Type": "application/json"}
        )
        if response.status_code == 200:
            data = response.json()
            print(f"SUCESSO: {data}")
            return True
        else:
            print(f"ERRO HTTP {response.status_code}: {response.text}")
            return False
    except Exception as e:
        print(f"ERRO: {e}")
        return False

def enable_device(device_id):
    """Habilita um dispositivo"""
    print(f"Habilitando dispositivo {device_id}")
    try:
        response = requests.put(
            f"{API_URL}/{device_id}",
            json={"is_active": True},
            headers={"Content-Type": "application/json"}
        )
        if response.status_code == 200:
            data = response.json()
            print(f"SUCESSO: {data}")
            return True
        else:
            print(f"ERRO HTTP {response.status_code}: {response.text}")
            return False
    except Exception as e:
        print(f"ERRO: {e}")
        return False

def get_device_status(device_id):
    """Verifica status do dispositivo"""
    print(f"Verificando status do dispositivo {device_id}")
    try:
        response = requests.get(f"{API_URL}/{device_id}")
        if response.status_code == 200:
            data = response.json()
            device = data['data']
            print(f"Status: {'ATIVO' if device['is_active'] else 'INATIVO'}")
            print(f"Nome: {device['name']}")
            return device['is_active']
        else:
            print(f"ERRO HTTP {response.status_code}: {response.text}")
            return None
    except Exception as e:
        print(f"ERRO: {e}")
        return None

def main():
    print("Testando habilitação/desabilitação de dispositivos")
    print("=" * 60)
    
    # Teste 1: Verificar status atual
    print("1. Status atual do ESP8266_001:")
    get_device_status("ESP8266_001")
    
    # Teste 2: Desabilitar dispositivo
    print("\n2. Desabilitando dispositivo...")
    disable_device("ESP8266_001")
    
    # Teste 3: Verificar se foi desabilitado
    print("\n3. Verificando se foi desabilitado:")
    get_device_status("ESP8266_001")
    
    # Teste 4: Habilitar dispositivo
    print("\n4. Habilitando dispositivo...")
    enable_device("ESP8266_001")
    
    # Teste 5: Verificar se foi habilitado
    print("\n5. Verificando se foi habilitado:")
    get_device_status("ESP8266_001")
    
    print("\nTeste concluído!")

if __name__ == "__main__":
    main()
