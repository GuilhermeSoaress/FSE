import smbus2
import bmp280  # Importa a classe BMP280 do módulo bmp280
from time import sleep


def temp_ambiente():
    port = 1
    address = 0x76
    bus = smbus2.SMBus(port)
    # Inicializar o sensor BMP280
    sensor = bmp280.BMP280(i2c_dev=bus, i2c_addr=address)

    # Realizar a leitura da temperatura
    temperatura = sensor.get_temperature()

    return temperatura