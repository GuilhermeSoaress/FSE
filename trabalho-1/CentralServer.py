import socket
import threading

class CentralServer:
    def __init__(self, hostname, portaTerreo, portaPrimeiroAndar, portaSegundoAndar):
        self.hostname = hostname
        self.portaTerreo = portaTerreo
        self.portaPrimeiroAndar = portaPrimeiroAndar
        self.portaSegundoandar = portaSegundoAndar
        self.socketTerreo = None
        self.socketPrimeiroAndar = None
        self.socketSegundoAndar = None

    def ConectarTerreo(self):
        self.socketTerreo = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socketTerreo.bind((self.hostname, self.portaTerreo))
        self.socketTerreo.listen(1)
        print('Aguardando conexão do Terreo...')
        conexao, endereco = self.socketTerreo.accept()
        print('Conexão estabelecida com o Terreo:', endereco)
        return conexao
    
    def ConectarPrimeiroAndar(self):
        self.socketPrimeiroAndar = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socketPrimeiroAndar.bind((self.hostname, self.portaPrimeiroAndar))
        self.socketPrimeiroAndar.listen(1)
        print('Aguardando conexão do primeiro andar...')
        conexao, endereco = self.socketPrimeiroAndar.accept()
        print('Conexão estabelecida com o primeiro andar:', endereco)
        return conexao

    def ConectarSegundoAndar(self):
        self.socketSegundoAndar = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socketSegundoAndar.bind((self.hostname, self.portaSegundoandar))
        self.socketSegundoAndar.listen(1)
        print('Aguardando conexão do segundo andar...')
        conexao, endereco = self.socketSegundoAndar.accept()
        print('Conexão estabelecida com o segundo andar:', endereco)
        return conexao

    def ReceberMsgAndar(self, conexao, andar):
        while True:
            mensagem = conexao.recv(1024).decode('utf-8')
            if not mensagem:
                print(f"Conexão com o andar {andar} encerrada.")
                break
            print(f"Mensagem do andar {andar}: {mensagem}")
            # Implemente aqui a lógica para processar a mensagem recebida

    def IniciarConexoes(self):
        processoTerreo = threading.Thread(target=self.ReceberMsgAndar, args=(self.ConectarTerreo(), 0))
        processoTerreo.start()
    
        ProcessoPrimeiroAndar = threading.Thread(target=self.ReceberMsgAndar, args=(self.ConectarPrimeiroAndar(), 1))
        ProcessoPrimeiroAndar.start()

        ProcessoSegundoAndar = threading.Thread(target=self.ReceberMsgAndar, args=(self.ConectarSegundoAndar(), 2))
        ProcessoSegundoAndar.start()

if __name__ == '__main__':
    HOSTNAME = 'localhost'
    PORTA_DO_TERREO = 10870
    PORTA_DO_PRIMEIRO_ANDAR = 10871
    PORTA_DO_SEGUNDO_ANDAR = 10872

    central = CentralServer(HOSTNAME, PORTA_DO_TERREO, PORTA_DO_PRIMEIRO_ANDAR, PORTA_DO_SEGUNDO_ANDAR)
    central.IniciarConexoes()
