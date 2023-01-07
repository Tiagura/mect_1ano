import requests

# Endereço do controlador Faucet
controller_address = "http://localhost:9302"

# Dados da regra de encaminhamento a ser adicionada
data = {
    "table_id": 0,
    "priority": 1,
    "match": {},
    "actions": ["CONTROLLER"]
}

# Envia a solicitação POST para o controlador Faucet
response = requests.post(controller_address + "/rules", json=data)

# Exibe a resposta do controlador Faucet
print(response.text)