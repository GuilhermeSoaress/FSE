import socket
import threading
import json
from datetime import datetime
import uuid
import time

vagas_terreo = {'T1': 0, 'T2': 0, 'T3': 0, 'T4': 0, 'T5': 0, 'T6': 0, 'T7': 0, 'T8': 0}
vagas_andar1 = {'A1': 0, 'A2': 0, 'A3': 0, 'A4': 0, 'A5': 0, 'A6': 0, 'A7': 0, 'A8': 0}
vagas_andar2 = {'B1': 0, 'B2': 0, 'B3': 0, 'B4': 0, 'B5': 0, 'B6': 0, 'B7': 0, 'B8': 0}

vagas_ocupadas_terreo = []
vagas_ocupadas_andar1 = []
vagas_ocupadas_andar2 = []

carros_estacionados_anterior_terreo = []
carros_estacionados_anterior_andar1 = []
carros_estacionados_anterior_andar2 = []

total_carros = 0
valorTotal = 0.0

carros_terreo = []
carros_andar1 = []
carros_andar2 = []

vagas_ocupadas = []

class Carro:
    def __init__(self, vaga, hora_chegada):
        self.vaga = vaga
        self.hora_chegada = hora_chegada

class CentralServer:
    def __init__(self, host='localhost', port=10602):
        self.host = host
        self.port = port
        self.clients = []
        self.lock = threading.Lock()
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.bind((self.host, self.port))
        self.sock.listen(5)
        
    def gerar_identificador():
        return str(uuid.uuid4())

    def accept_connections(self):
        while True:
            conn, addr = self.sock.accept()
            print(f"Nova conexão: {addr}")
            self.clients.append(conn)
            thread_receive = threading.Thread(target=self.receive_message, args=(conn,))
            thread_receive.start()

    estacionamento_cheio = 'False'
    def receive_message(self, conn):
        while True:
            global vagas_ocupadas_terreo, vagas_ocupadas_andar1, vagas_ocupadas_andar2
            global vagas_terreo, vagas_andar1, vagas_andar2
            global carros_terreo, carros_andar1, carros_andar2
            global carros_estacionados_anterior_terreo, carros_estacionados_anterior_andar1, carros_estacionados_anterior_andar2
            global total_carros, carros, vagas_ocupadas, valorTotal
            try:
                message = conn.recv(1024).decode()
                message = json.loads(message)
                vagas_ocupadas = message['message'][0]['vaga_ocupada']
                
                if message['from'] == 'client0':
                    carros_que_sairam = list(set(carros_estacionados_anterior_terreo) - set(vagas_ocupadas))
                    vagas_ocupadas_terreo = len(vagas_ocupadas)
                    for i in carros_que_sairam:
                        vaga = f"T{i}"
                        for carro in carros_terreo:
                            if carro.vaga == vaga:
                                data_dif = datetime.now() - carro.hora_chegada
                                minutos = data_dif.total_seconds() / 60
                                preco = 0.15 * minutos
                                valorTotal += preco
                                print(f"Carro na vaga {vaga} saiu e o preço é: R$ {preco:.2f} e o total {valorTotal:.2f}")
                                vagas_terreo[vaga] = 0  # Libera a vaga
                                carros_terreo.remove(carro)
                                break
                    carros_estacionados_anterior_terreo = vagas_ocupadas
                    for i in vagas_ocupadas:
                        if f'T{i}' in vagas_terreo.keys():
                            if vagas_terreo[f'T{i}'] == 0:
                                carros_terreo.append(Carro(f'T{i}', datetime.now()))
                                vagas_terreo[f'T{i}'] = 1
                        else:
                            print(f"A vaga T{i} não existe no térreo.")
                
                if message['from'] == 'client1':
                    carros_que_sairam = list(set(carros_estacionados_anterior_andar1) - set(vagas_ocupadas))
                    vagas_ocupadas_andar1 = len(vagas_ocupadas)
                    for i in carros_que_sairam:
                        vaga = f"A{i}"
                        for carro in carros_andar1:
                            if carro.vaga == vaga:
                                data_dif = datetime.now() - carro.hora_chegada
                                minutos = data_dif.total_seconds() / 60
                                preco = 0.15 * minutos
                                valorTotal += preco
                                print(f"Carro na vaga {vaga} saiu e o preço é: R$ {preco:.2f} e o total {valorTotal:.2f}")
                                vagas_andar1[vaga] = 0  # Libera a vaga
                                carros_andar1.remove(carro)
                                break
                    carros_estacionados_anterior_andar1 = vagas_ocupadas
                    for i in vagas_ocupadas:
                        if f'A{i}' in vagas_andar1.keys():
                            if vagas_andar1[f'A{i}'] == 0:
                                carros_andar1.append(Carro(f'A{i}', datetime.now()))
                                vagas_andar1[f'A{i}'] = 1
                        else:
                            print(f"A vaga A{i} não existe no andar 1.")
                
                if message['from'] == 'client2':
                    carros_que_sairam = list(set(carros_estacionados_anterior_andar2) - set(vagas_ocupadas))
                    vagas_ocupadas_andar2 = len(vagas_ocupadas)
                    for i in carros_que_sairam:
                        vaga = f"B{i}"
                        for carro in carros_andar2:
                            if carro.vaga == vaga:
                                data_dif = datetime.now() - carro.hora_chegada
                                minutos = data_dif.total_seconds() / 60
                                preco = 0.15 * minutos
                                valorTotal += preco
                                print(f"Carro na vaga {vaga} saiu e o preço é: R$ {preco:.2f} e o total {valorTotal:.2f}")
                                vagas_andar2[vaga] = 0  # Libera a vaga
                                carros_andar2.remove(carro)
                                break
                    carros_estacionados_anterior_andar2 = vagas_ocupadas
                    for i in vagas_ocupadas:
                        if f'B{i}' in vagas_andar2.keys():
                            if vagas_andar2[f'B{i}'] == 0:
                                carros_andar2.append(Carro(f'B{i}', datetime.now()))
                                vagas_andar2[f'B{i}'] = 1
                        else:
                            print(f"A vaga B{i} não existe no andar 2.")
                
                print(f"Preço total {valorTotal:.2f}")
                print(f"Total de carros estacionados no térreo: {vagas_ocupadas_terreo}")
                print(f"Total de carros estacionados no andar 1: {vagas_ocupadas_andar1}")
                print(f"Total de carros estacionados no andar 2: {vagas_ocupadas_andar2}")
                print(f"Estacionamento térreo: {vagas_terreo}")
                print(f"Estacionamento andar 1: {vagas_andar1}")
                print(f"Estacionamento andar 2: {vagas_andar2}")

                if (vagas_ocupadas_terreo + vagas_ocupadas_andar1 + vagas_ocupadas_andar2) == 24:
                    self.send_message('fechar')
                else:
                    self.send_message('abrir')
                
                time.sleep(0.8)
                if vagas_ocupadas_andar2 == 8:
                    self.send_message('fecha 2 andar')
                else:
                    self.send_message('abre 2 andar')
                
            except:
                break
        conn.close()

    def send_message_to_clients(self, sender, message):
        for client in self.clients:
            if client != sender:
                try:
                    client.send(json.dumps(message).encode())
                except:
                    print(f"Erro ao enviar mensagem para {client.getpeername()}")
                    self.clients.remove(client)
  
    def send_message(self, message):
        message_dict = {"from": "server", "message": message}
        self.send_message_to_clients(None, message_dict)
  
    def run(self):
        thread_accept = threading.Thread(target=self.accept_connections)
        thread_accept.start()

if __name__ == '__main__':
    server = CentralServer()
    server.run()
