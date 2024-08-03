import RPi.GPIO as GPIO
import threading
import time
import struct
import serial
import smbus2
import bme280
import signal
import sys
from luma.core.interface.serial import i2c
from luma.oled.device import ssd1306
from luma.core.render import canvas
from PIL import ImageFont

# Remove warnings
GPIO.setwarnings(False)

# UART configuration
ser = serial.Serial(
    port='/dev/serial0',
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=0.5
)

# Global variables
TIME = 0.001
matricula = [6, 7, 6, 7]

# PID Controller class
class PIDController:
    def __init__(self, kp, ki, kd, setpoint=0):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.setpoint = setpoint
        self.integral = 0
        self.previous_error = 0

    def update(self, current_value):
        error = self.setpoint - current_value
        self.integral += error
        derivative = error - self.previous_error
        self.previous_error = error
        return self.kp * error + self.ki * self.integral + self.kd * derivative

# GPIO setup for two elevators
elevator1_pins = {
    "DIR1": 20,
    "DIR2": 21,
    "MOTOR_PWM": 12,
    "SENSOR_TERR": 18,
    "SENSOR_1": 23,
    "SENSOR_2": 24,
    "SENSOR_3": 25
}

elevator2_pins = {
    "DIR1": 19,
    "DIR2": 26,
    "MOTOR_PWM": 13,
    "SENSOR_TERR": 17,
    "SENSOR_1": 27,
    "SENSOR_2": 22,
    "SENSOR_3": 6
}

def setup_gpio(pins):
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(pins["DIR1"], GPIO.OUT)
    GPIO.setup(pins["DIR2"], GPIO.OUT)
    GPIO.setup(pins["MOTOR_PWM"], GPIO.OUT)
    GPIO.setup(pins["SENSOR_TERR"], GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.setup(pins["SENSOR_1"], GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.setup(pins["SENSOR_2"], GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.setup(pins["SENSOR_3"], GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

setup_gpio(elevator1_pins)
setup_gpio(elevator2_pins)

pwm1 = GPIO.PWM(elevator1_pins["MOTOR_PWM"], 1000)  # 1 kHz frequency
pwm2 = GPIO.PWM(elevator2_pins["MOTOR_PWM"], 1000)  # 1 kHz frequency

pwm1.start(0)
pwm2.start(0)

def set_motor_direction(pins, up):
    if up:
        GPIO.output(pins["DIR1"], GPIO.HIGH)
        GPIO.output(pins["DIR2"], GPIO.LOW)
    else:
        GPIO.output(pins["DIR1"], GPIO.LOW)
        GPIO.output(pins["DIR2"], GPIO.HIGH)

def set_motor_speed(pwm, speed):
    pwm.ChangeDutyCycle(speed)

# I2C setup for temperature sensors
bus = smbus2.SMBus(1)
address_1 = 0x76
address_2 = 0x77

def read_temperature(address):
    calibration_params = bme280.load_calibration_params(bus, address)
    data = bme280.sample(bus, address, calibration_params)
    return data.temperature

# Display setup
serial = i2c(port=1, address=0x3C)
device = ssd1306(serial)

font = ImageFont.load_default()

def update_display(state1, floor1, temp1, state2, floor2, temp2):
    with canvas(device) as draw:
        draw.text((0, 0), f"Elevator 1 State: {state1}", font=font, fill=255)
        draw.text((0, 10), f"Elevator 1 Floor: {floor1}", font=font, fill=255)
        draw.text((0, 20), f"Elevator 1 Temp: {temp1:.2f}C", font=font, fill=255)
        draw.text((0, 30), f"Elevator 2 State: {state2}", font=font, fill=255)
        draw.text((0, 40), f"Elevator 2 Floor: {floor2}", font=font, fill=255)
        draw.text((0, 50), f"Elevator 2 Temp: {temp2:.2f}C", font=font, fill=255)

# Signal handling
def signal_handler(sig, frame):
    print("Exiting...")
    pwm1.stop()
    pwm2.stop()
    GPIO.cleanup()
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

def calcular_crc(commands):
    crc = 0
    for command in commands:
        crc = CRC16(crc, command)
    return crc

def CRC16(crc, data):
    crc ^= data & 0xFF
    for _ in range(8):
        if crc & 1:
            crc = (crc >> 1) ^ 0xA001
        else:
            crc >>= 1
    return crc

def solicitarValorEncoder(motorID):
    ser.flushInput()
    request = bytes([0x01, 0x23, 0xC1, motorID]) + bytes(matricula)
    crc16 = calcular_crc(request)
    crcBytes = struct.pack('<H', crc16)
    requestCRC = request + crcBytes
    ser.write(requestCRC)
    response = ser.read(7)
    valorEncoder = struct.unpack('<I', response[3:7])[0]
    return valorEncoder

def tratamentoSensor(pins):
    while True:
        if GPIO.event_detected(pins["SENSOR_TERR"]):
            print(f"{pins['SENSOR_TERR']} ativado")
            valorEncoder = solicitarValorEncoder(pins["MOTOR_PWM"])
            print(valorEncoder)
            
        elif GPIO.event_detected(pins["SENSOR_1"]):
            print(f"{pins['SENSOR_1']} ativado")
            valorEncoder = solicitarValorEncoder(pins["MOTOR_PWM"])
            print(valorEncoder)
            
        elif GPIO.event_detected(pins["SENSOR_2"]):
            print(f"{pins['SENSOR_2']} ativado")
            valorEncoder = solicitarValorEncoder(pins["MOTOR_PWM"])
            print(valorEncoder)
            
        elif GPIO.event_detected(pins["SENSOR_3"]):
            print(f"{pins['SENSOR_3']} ativado")
            valorEncoder = solicitarValorEncoder(pins["MOTOR_PWM"])
            print(valorEncoder)
        
        time.sleep(TIME)

def controlaElevador(pid, pwm, pins):
    while True:
        encoderAtual = solicitarValorEncoder(pins["MOTOR_PWM"])
        if encoderAtual >= 6000 and encoderAtual <= 6055:
            set_motor_direction(pins, False)
            pwm.ChangeDutyCycle(0)
            print("Parando no andar")
            break
        potenciaNecessaria = pid.update(encoderAtual)
        if potenciaNecessaria >= 0 and potenciaNecessaria <= 100:
            pwm.ChangeDutyCycle(potenciaNecessaria)
        time.sleep(0.2)

def sobeElevador(pid, pwm, pins):
    set_motor_direction(pins, True)
    controlaElevador(pid, pwm, pins)

def desceElevador(pid, pwm, pins):
    set_motor_direction(pins, False)
    controlaElevador(pid, pwm, pins)

def freiaElevador(pins):
    GPIO.output(pins["DIR1"], GPIO.HIGH)
    GPIO.output(pins["DIR2"], GPIO.HIGH)

def enviaTemp(retornoConexao):
   formattedFloat = "{:.2f}".format(get_temp_ambiente())
   message = b'\x01'  + b'\x16' + b'\xD1'
   enviaValorFloat(retornoConexao, message, matricula, formattedFloat)

def get_temp_ambiente():
    port = 1
    address = 0x76
    bus = smbus2.SMBus(port)
    sensor = bme280.BME280(i2c_dev=bus, i2c_addr=address)
    temperatura = sensor.get_temperature()
    return temperatura

def enviaValorFloat(ser, command, matricula, value):
    print(value)
    message = command + struct.pack('<f', float(value)) + bytes([int(digit) for digit in matricula])
    valorCRC = calcular_crc(message)
    message += int(valorCRC).to_bytes(2, 'little')
    print(f'FLOAT ENVIADO: {message}')
    ser.write(message)

def main():
    pid1 = PIDController(kp=0.5, ki=0.05, kd=40.0)
    pid2 = PIDController(kp=0.5, ki=0.05, kd=40.0)

    pid1.setpoint = 6000  # First floor
    pid2.setpoint = 6000  # First floor

    thread1 = threading.Thread(target=tratamentoSensor, args=(elevator1_pins,))
    thread2 = threading.Thread(target=tratamentoSensor, args=(elevator2_pins,))
    thread3 = threading.Thread(target=get_temp_ambiente)

    thread1.start()
    thread2.start()
    thread3.start()

    sobeElevador(pid1, pwm1, elevator1_pins)
    sobeElevador(pid2, pwm2, elevator2_pins)

    enviaTemp(ser)

    thread1.join()
    thread2.join()
    thread3.join()

if __name__ == "__main__":
    main()
