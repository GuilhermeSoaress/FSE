import smbus2
import bmp280  # Importa a classe BMP280 do módulo bmp280
from time import sleep

# Cria uma instância da classe BMP280

def temp_ambiente():
    port = 1
    address = 0x77
    bus = smbus2.SMBus(port)
    # Inicializar o sensor BMP280
    sensor = bmp280.BMP280(i2c_dev=bus, i2c_addr=address)

    # Realizar a leitura da temperatura
    temperatura = sensor.get_temperature()

    return temperatura


"""
# Exemplo de uso
try:
    while True:
        temperature = temp_ambiente()
        temperature_formatada = round(temperature, 2)
        print(f"temperature ambiente: {temperature_formatada} °C")
        sleep(1)

except KeyboardInterrupt:
    print("\n\nPrograma interrompido pelo usuário.")
"""