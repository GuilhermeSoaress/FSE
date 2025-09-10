import socket
import threading
import json
from datetime import datetime
import uuid
import logging
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)

SINAL_DE_LOTADO_FECHADO_TERREO = 27
SINAL_DE_LOTADO_FECHADO_ANDAR1 = 8
SINAL_DE_LOTADO_FECHADO_ANDAR2 = 14

GPIO.setup(SINAL_DE_LOTADO_FECHADO_TERREO, GPIO.OUT)
GPIO.setup(SINAL_DE_LOTADO_FECHADO_ANDAR1, GPIO.OUT)
GPIO.setup(SINAL_DE_LOTADO_FECHADO_ANDAR2, GPIO.OUT)

# Configurar logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(message)s')

def exibir_titulo():
    print("\033[1;33m")  
    print("=========================================")
    print("          ESTACIONAMENTO CENTRAL         ")
    print("           'Seu carro em boas mãos'      ")
    print("=========================================")
    print("\033[0m")  

vagas_andar2 = {'B1': 0, 'B2': 0, 'B3': 0, 'B4': 0, 'B5': 0, 'B6': 0, 'B7': 0, 'B8': 0}
vagas_andar1 = {'A1': 0, 'A2': 0, 'A3': 0, 'A4': 0, 'A5': 0, 'A6': 0, 'A7': 0, 'A8': 0}
vagas_terreo = {'T1': 0, 'T2': 0, 'T3': 0, 'T4': 0, 'T5': 0, 'T6': 0, 'T7': 0, 'T8': 0}

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

andar1_bloqueado = False
andar2_bloqueado = False
controle_manual = False

class Carro:
    def __init__(self, vaga, hora_chegada):
        self.vaga = vaga
        self.hora_chegada = hora_chegada
        self.hora_saida = None

def desligar_luzes():
    GPIO.output(SINAL_DE_LOTADO_FECHADO_TERREO, GPIO.LOW)
    GPIO.output(SINAL_DE_LOTADO_FECHADO_ANDAR1, GPIO.LOW)
    GPIO.output(SINAL_DE_LOTADO_FECHADO_ANDAR2, GPIO.LOW)

class CentralServer:
    def __init__(self, host='localhost', port=10712):
        self.host = host
        self.port = port
        self.clients = []
        self.lock = threading.Lock()
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.bind((self.host, self.port))
        self.sock.listen(5)

    def gerar_identificador(self):
        return str(uuid.uuid4())

    def accept_connections(self):
        threading.Thread(target=self.controlar_luz).start()
        while True:
            conn, addr = self.sock.accept()
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
            global total_carros, carros, vagas_ocupadas, valorTotal, andar1_bloqueado, andar2_bloqueado
            try:
                message = conn.recv(1024).decode()
                message = json.loads(message)
                vagas_ocupadas = message['message'][0]['vaga_ocupada']
                mudanca_ocorrida = False

                if message['from'] == 'client0':
                    carros_que_sairam = list(set(carros_estacionados_anterior_terreo) - set(vagas_ocupadas))
                    vagas_ocupadas_terreo = len(vagas_ocupadas)
                    for i in carros_que_sairam:
                        vaga = f"T{i}"
                        for carro in carros_terreo:
                            if carro.vaga == vaga:
                                carro.hora_saida = datetime.now()
                                data_dif = carro.hora_saida - carro.hora_chegada
                                minutos = data_dif.total_seconds() / 60
                                preco = 0.15 * minutos
                                valorTotal += preco
                                mensagem = f"Carro na vaga {vaga} saiu às {carro.hora_saida} e o preço é: R$ {preco:.2f}. Total acumulado: R$ {valorTotal:.2f}"
                                logging.info(mensagem)
                                vagas_terreo[vaga] = 0  # Libera a vaga
                                carros_terreo.remove(carro)
                                mudanca_ocorrida = True
                                break
                    carros_estacionados_anterior_terreo = vagas_ocupadas
                    for i in vagas_ocupadas:
                        if f'T{i}' in vagas_terreo.keys():
                            if vagas_terreo[f'T{i}'] == 0:
                                carros_terreo.append(Carro(f'T{i}', datetime.now()))
                                vagas_terreo[f'T{i}'] = 1
                                mudanca_ocorrida = True
                        else:
                            logging.warning(f"A vaga T{i} não existe no térreo.")
                
                if message['from'] == 'client1' and not andar1_bloqueado:
                    carros_que_sairam = list(set(carros_estacionados_anterior_andar1) - set(vagas_ocupadas))
                    vagas_ocupadas_andar1 = len(vagas_ocupadas)
                    for i in carros_que_sairam:
                        vaga = f"A{i}"
                        for carro in carros_andar1:
                            if carro.vaga == vaga:
                                carro.hora_saida = datetime.now()
                                data_dif = carro.hora_saida - carro.hora_chegada
                                minutos = data_dif.total_seconds() / 60
                                preco = 0.15 * minutos
                                valorTotal += preco
                                mensagem = f"Carro na vaga {vaga} saiu às {carro.hora_saida} e o preço é: R$ {preco:.2f}. Total acumulado: R$ {valorTotal:.2f}"
                                logging.info(mensagem)
                                vagas_andar1[vaga] = 0  # Libera a vaga
                                carros_andar1.remove(carro)
                                mudanca_ocorrida = True
                                break
                    carros_estacionados_anterior_andar1 = vagas_ocupadas
                    for i in vagas_ocupadas:
                        if f'A{i}' in vagas_andar1.keys():
                            if vagas_andar1[f'A{i}'] == 0:
                                carros_andar1.append(Carro(f'A{i}', datetime.now()))
                                vagas_andar1[f'A{i}'] = 1
                                mudanca_ocorrida = True
                        else:
                            logging.warning(f"A vaga A{i} não existe no andar 1.")
                
                if message['from'] == 'client2' and not andar2_bloqueado:
                    carros_que_sairam = list(set(carros_estacionados_anterior_andar2) - set(vagas_ocupadas))
                    vagas_ocupadas_andar2 = len(vagas_ocupadas)
                    for i in carros_que_sairam:
                        vaga = f"B{i}"
                        for carro in carros_andar2:
                            if carro.vaga == vaga:
                                carro.hora_saida = datetime.now()
                                data_dif = carro.hora_saida - carro.hora_chegada
                                minutos = data_dif.total_seconds() / 60
                                preco = 0.15 * minutos
                                valorTotal += preco
                                mensagem = f"Carro na vaga {vaga} saiu às {carro.hora_saida} e o preço é: R$ {preco:.2f}. Total acumulado: R$ {valorTotal:.2f}"
                                logging.info(mensagem)
                                vagas_andar2[vaga] = 0  # Libera a vaga
                                carros_andar2.remove(carro)
                                mudanca_ocorrida = True
                                break
                    carros_estacionados_anterior_andar2 = vagas_ocupadas
                    for i in vagas_ocupadas:
                        if f'B{i}' in vagas_andar2.keys():
                            if vagas_andar2[f'B{i}'] == 0:
                                carros_andar2.append(Carro(f'B{i}', datetime.now()))
                                vagas_andar2[f'B{i}'] = 1
                                mudanca_ocorrida = True
                        else:
                            logging.warning(f"A vaga B{i} não existe no andar 2.")
                
                if mudanca_ocorrida:
                    self.exibir_status()

                self.controlar_acesso()
                
            except Exception as e:
                pass  # Não imprime mensagens de erro
        conn.close()

    def exibir_status(self):
        exibir_titulo()
        
        print(f"\033[1;34mPreço total: R$ {valorTotal:.2f}\033[0m")
        self.exibir_vagas(vagas_terreo, "Térreo")
        self.exibir_vagas(vagas_andar1, "Andar 1")
        self.exibir_vagas(vagas_andar2, "Andar 2")

        total_carros = sum(vagas_terreo.values()) + sum(vagas_andar1.values()) + sum(vagas_andar2.values())
        print(f"\033[1;34mQuantidade de carros por andar:\033[0m")
        print(f"\033[1;34mTérreo: {sum(vagas_terreo.values())}\033[0m")
        print(f"\033[1;34mAndar 1: {sum(vagas_andar1.values())}\033[0m")
        print(f"\033[1;34mAndar 2: {sum(vagas_andar2.values())}\033[0m")
        print(f"\033[1;34mTotal: {total_carros}\033[0m")

    def exibir_vagas(self, vagas, titulo):
        print(f"\033[1;36m{titulo}\033[0m")
        for vaga, ocupada in vagas.items():
            if ocupada:
                print(f"\033[1;31m[{vaga}] CAR\033[0m", end=" ")
            else:
                print(f"\033[1;32m[{vaga}]    \033[0m", end=" ")
        print("\n")

    def controlar_acesso(self):
        if sum(vagas_andar1.values()) == 8:
            GPIO.output(SINAL_DE_LOTADO_FECHADO_ANDAR1, GPIO.HIGH)
        if sum(vagas_andar1.values()) < 8 and controle_manual == False:
            GPIO.output(SINAL_DE_LOTADO_FECHADO_ANDAR1, GPIO.LOW)

        if sum(vagas_andar2.values()) == 8:
            GPIO.output(SINAL_DE_LOTADO_FECHADO_ANDAR2, GPIO.HIGH)
        if sum(vagas_andar2.values()) < 8 and controle_manual == False:
            GPIO.output(SINAL_DE_LOTADO_FECHADO_ANDAR2, GPIO.LOW)

        total_carros = sum(vagas_terreo.values()) + sum(vagas_andar1.values()) + sum(vagas_andar2.values())
        if total_carros == 24:
            GPIO.output(SINAL_DE_LOTADO_FECHADO_TERREO, GPIO.HIGH)
        if total_carros < 24 and controle_manual == False:
            GPIO.output(SINAL_DE_LOTADO_FECHADO_TERREO, GPIO.LOW)


    def controlar_luz(self):
        global controle_manual
        while True:
            user_input = int(input("Digite 0 para acender a luz do térreo, 1 para o primeiro andar, 2 para o segundo andar e 3 para apagar: "))
            if user_input == 0:
                GPIO.output(SINAL_DE_LOTADO_FECHADO_TERREO, GPIO.HIGH)
                controle_manual = True
            elif user_input == 1:
                GPIO.output(SINAL_DE_LOTADO_FECHADO_ANDAR1, GPIO.HIGH)
                controle_manual = True
            elif user_input == 2:
                GPIO.output(SINAL_DE_LOTADO_FECHADO_ANDAR2, GPIO.HIGH)
                controle_manual = True
            elif user_input == 3:
                desligar_luzes()
                controle_manual = False

if __name__ == "__main__":
    server = CentralServer()
    server.accept_connections()
