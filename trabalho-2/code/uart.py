import logging

import serial
from time import sleep
import struct
import crc

uart0_filestream = serial.Serial(
    port='/dev/serial0',
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
)

if uart0_filestream == -1:
    logging.error("Não foi possível iniciar a UART.\n")
else:
    logging.info("UART inicializada!\n")

def envia_comando(comando, *args):
    msg = bytes(comando)

    for valor in args:
        if type(valor) == int:
            msg += struct.pack('<i', valor)
        elif type(valor) == float:
            msg += struct.pack('<f', valor)
        elif type(valor) == list:
            msg += bytes(valor)
        else:
            logging.warning(f"Tipo de valor não suportado: {type(valor)}")

    crc_calculado = crc.calcula_crc(msg, len(msg))
    mensagem_crc = msg + crc_calculado.to_bytes(2, 'little')
    uart0_filestream.write(mensagem_crc)

def recebe_resposta():
    if uart0_filestream.in_waiting >= 9:
        resposta = uart0_filestream.read(9)

        if len(resposta) != 9:
            logging.error("Erro de comunicação")
            return None

        crc_calculado = crc.calcula_crc(resposta, len(resposta) - 2)
        crc_recebido = struct.unpack('<H', resposta[-2:])[0]

        # logging.debug(f"Resposta recebida: {resposta}")
        # logging.debug(f"CRC calculado: {crc_calculado}")
        # logging.debug(f"CRC recebido: {crc_recebido}")

        cod = resposta[0:1]

        if crc_calculado == crc_recebido:

            if cod == b'\x03' or cod == b'\x06': # Encoder
                info = resposta[3:7]
                reg = info[-1]
                return reg
            elif cod == b'\x16': # PWM ou Temp
                info = resposta[3:7]
                temp = int.from_bytes(info, byteorder='big')
                return temp
            else: # Temp
                info = resposta[3:7]
                encoder = int.from_bytes(info, byteorder='little')
                return encoder
        else:
            # logging.error("Erro de CRC")
            return None

def envia_recebe(comando, *args):
    tentativas = 0

    uart0_filestream.flushInput()
    while True:
        envia_comando(comando, *args)
        sleep(0.05) 
        resposta = recebe_resposta()

        if resposta is not None:
            logging.debug(f"Resposta recebida: {resposta}")
            return resposta

        logging.debug(f"[{tentativas}] CRC não corresponde. Reenviando...")
        tentativas += 1 
