import socket
import threading
import json
import RPi.GPIO as GPIO
import uuid
import time
from datetime import datetime

GPIO.setmode(GPIO.BCM)

ENDERECO_01 = 9
ENDERECO_02 = 11
ENDERECO_03 = 15
SENSOR_DE_VAGA = 1
SINAL_DE_LOTADO_FECHADO = 14
SENSOR_DE_PASSAGEM_1 = 0
SENSOR_DE_PASSAGEM_2 = 7

GPIO.setup(ENDERECO_01, GPIO.OUT)
GPIO.setup(ENDERECO_02, GPIO.OUT)
GPIO.setup(ENDERECO_03, GPIO.OUT)
GPIO.setup(SENSOR_DE_VAGA, GPIO.IN)
GPIO.setup(SINAL_DE_LOTADO_FECHADO, GPIO.OUT)
GPIO.setup(SENSOR_DE_PASSAGEM_1, GPIO.IN)
GPIO.setup(SENSOR_DE_PASSAGEM_2, GPIO.IN)

def ler_sensor_vaga():
    return GPIO.input(SENSOR_DE_VAGA)

def acender_luz_lotado():
    GPIO.output(SINAL_DE_LOTADO_FECHADO, GPIO.HIGH)

def apagar_luz_lotado():
    GPIO.output(SINAL_DE_LOTADO_FECHADO, GPIO.LOW)

def leitura_sensor_vaga(endereco):
    GPIO.output(ENDERECO_01, (endereco & 0b001) == 0b001)
    GPIO.output(ENDERECO_02, (endereco & 0b010) == 0b010)
    GPIO.output(ENDERECO_03, (endereco & 0b100) == 0b100)
    time.sleep(0.1)
    return GPIO.input(SENSOR_DE_VAGA)

pode_entrar = True

class DistributedServer:
    def __init__(self, host='localhost', port=10712, client='client2'):
        self.host = host
        self.port = port
        self.client = client
        self.connect_to_server()

    def connect_to_server(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.sock.connect((self.host, self.port))
        except Exception as e:
            print(f"Erro ao conectar ao servidor: {e}")
            time.sleep(5)
            self.connect_to_server()
        
    def envia_servidor(self, vagas_ocupadas):
        message = [{
            "vaga_ocupada": vagas_ocupadas
        }]
        message_dict = {
            "from": self.client,
            "message": message
        }
        try:
            self.sock.send(json.dumps(message_dict).encode())
        except OSError:
            self.connect_to_server()
            self.sock.send(json.dumps(message_dict).encode())
  
    def controla_sinal(self):
        while True:
            vagas_ocupadas = []            
            for endereco in range(8):
                if leitura_sensor_vaga(endereco):
                    vagas_ocupadas.append(endereco+1)
            total_vagas_ocupadas = len(vagas_ocupadas)
            # if total_vagas_ocupadas == 8:  # alterado
            #     acender_luz_lotado()  # alterado
            time.sleep(0.5)
            self.envia_servidor(vagas_ocupadas)

    # def receive_message(self):
    #     while True:
    #         try:
    #             message = self.sock.recv(1024).decode()
    #             message_dict = json.loads(message)
    #             if message_dict['message'] == 'fecha 2 andar':
    #                 acender_luz_lotado()
    #             if message_dict['message'] == 'abre 2 andar':
    #                 apagar_luz_lotado() 
    #         except:
    #             self.connect_to_server()
    #     self.sock.close()    

    def run(self):
        thread_sinal = threading.Thread(target=self.controla_sinal)
        # thread_receive = threading.Thread(target=self.receive_message)
        # thread_receive.start()
        thread_sinal.start()

if __name__ == '__main__':
    server = DistributedServer(port=10712, client='client2')
    server.run()
