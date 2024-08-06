#calculo para deteccao de erros
def calcular_crc(commands):
    crc = 0 #variavel inicializada como 0 
    for command in commands:
        crc = crc16(crc, command)
    return crc

def crc16(crc, data):
    crc ^= data & 0xFF
    for _ in range(8):
        if crc & 1:
            crc = (crc >> 1) ^ 0xA001
        else:
            crc >>= 1
    return crc
