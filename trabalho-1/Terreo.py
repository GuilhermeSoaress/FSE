import socket
import threading
import json
import RPi.GPIO as GPIO
import uuid
import time
from datetime import datetime

GPIO.setmode(GPIO.BCM)

ENDERECO_01 = 22
ENDERECO_02 = 26
ENDERECO_03 = 19
SENSOR_DE_VAGA = 18
SINAL_DE_LOTADO_FECHADO = 27
SENSOR_ABERTURA_CANCELA_ENTRADA = 23
SENSOR_FECHAMENTO_CANCELA_ENTRADA = 24
MOTOR_CANCELA_ENTRADA = 10
SENSOR_ABERTURA_CANCELA_SAIDA = 25
SENSOR_FECHAMENTO_CANCELA_SAIDA = 12
MOTOR_CANCELA_SAIDA = 17

GPIO.setup(ENDERECO_01, GPIO.OUT)
GPIO.setup(ENDERECO_02, GPIO.OUT)
GPIO.setup(ENDERECO_03, GPIO.OUT)
GPIO.setup(SENSOR_DE_VAGA, GPIO.IN)
GPIO.setup(SINAL_DE_LOTADO_FECHADO, GPIO.OUT)
GPIO.setup(SENSOR_ABERTURA_CANCELA_ENTRADA, GPIO.IN)
GPIO.setup(SENSOR_FECHAMENTO_CANCELA_ENTRADA, GPIO.IN)
GPIO.setup(MOTOR_CANCELA_ENTRADA, GPIO.OUT)
GPIO.setup(SENSOR_ABERTURA_CANCELA_SAIDA, GPIO.IN)
GPIO.setup(SENSOR_FECHAMENTO_CANCELA_SAIDA, GPIO.IN)
GPIO.setup(MOTOR_CANCELA_SAIDA, GPIO.OUT)

def gerar_identificador():
    return str(uuid.uuid4())

def abrir_cancela_entrada():
    GPIO.output(MOTOR_CANCELA_ENTRADA, GPIO.HIGH)

def fechar_cancela_entrada():
    GPIO.output(MOTOR_CANCELA_ENTRADA, GPIO.LOW)

def abrir_cancela_saida():
    GPIO.output(MOTOR_CANCELA_SAIDA, GPIO.HIGH)

def fechar_cancela_saida():    
    GPIO.output(MOTOR_CANCELA_SAIDA, GPIO.LOW)

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
    def __init__(self, host='localhost', port=10602, client='client0'):
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
            global pode_entrar
            if pode_entrar:
                if GPIO.input(SENSOR_ABERTURA_CANCELA_ENTRADA) == GPIO.HIGH:
                    abrir_cancela_entrada()
                    print("Veiculo passando")
                    if GPIO.wait_for_edge(SENSOR_FECHAMENTO_CANCELA_ENTRADA, GPIO.RISING):
                        fechar_cancela_entrada()
                        print("Cancela fechada")
            time.sleep(1.5)
            vagas_ocupadas = []            
            for endereco in range(8):
                if leitura_sensor_vaga(endereco):
                    vagas_ocupadas.append(endereco+1)
            total_vagas_ocupadas = len(vagas_ocupadas)
            time.sleep(0.3)
            self.envia_servidor(vagas_ocupadas)

            if GPIO.input(SENSOR_ABERTURA_CANCELA_SAIDA) == GPIO.HIGH:
                abrir_cancela_saida()
                print("Veiculo passando")
                if GPIO.wait_for_edge(SENSOR_FECHAMENTO_CANCELA_SAIDA, GPIO.RISING):
                    time.sleep(0.4)
                    fechar_cancela_saida()
                    print("Cancela fechada")

    def receive_message(self):
        while True:
            try:
                message = self.sock.recv(1024).decode()
                message_dict = json.loads(message)
                global pode_entrar
                if message_dict['message'] == 'fechar':
                    acender_luz_lotado()
                    pode_entrar = False
                if message_dict['message'] == 'abrir':
                    apagar_luz_lotado() 
                    pode_entrar = True   
            except:
                self.connect_to_server()

    def run(self):
        thread_sinal = threading.Thread(target=self.controla_sinal)
        thread_receive = threading.Thread(target=self.receive_message)
        thread_receive.start()
        thread_sinal.start()

if __name__ == '__main__':
    server = DistributedServer(port=10602, client='client0')
    server.run()
