import logging
import log
from threading import Thread, Event
import signal
import sys
from time import sleep

logging.getLogger('').handlers.clear()
log.config_logging()

import cmds2
import uart
import gpio2
import pid
import struct

pid_control = pid.PID()
pid_control.configura_constantes(Kp=0.1, Ki=0.05, Kd=0.01)
pid_control.atualiza_referencia(0.0)

controle = cmds2.controle
exit_execution = Event()

def finaliza_programa(sig, frame):
    logging.info("Recebido sinal para finalizar o programa.")
    controle.stop_pwm()
    gpio2.GPIO.cleanup()
    exit_execution.set()
    logging.info("Programa interrompido pelo usuário.")
    sys.exit(0)
    
try:
    # Iniciando as threads
    thread_apurar_oled = Thread(target=cmds2.apurar_oled, args=(exit_execution,))
    thread_menu_elevador = Thread(target=cmds2.menu_elevador, args=(exit_execution,))
    # thread_le_regs = Thread(target=cmds2.le_regs)

    # Configurando as threads como daemon
    thread_apurar_oled.daemon = True
    thread_menu_elevador.daemon = True
    # thread_le_regs.daemon = True

    # Configurando o tratamento de sinais para finalizar o programa
    signal.signal(signal.SIGINT, finaliza_programa)
    signal.signal(signal.SIGTERM, finaliza_programa)

    # Iniciando as threads
    thread_apurar_oled.start()
    thread_menu_elevador.start()
    # thread_le_regs.start()

    # Aguardando as threads terminarem (se necessário)
    thread_apurar_oled.join()
    thread_menu_elevador.join()
    # thread_le_regs.join()

except KeyboardInterrupt:
    logging.info("Programa interrompido pelo usuário")

finally:
    # Limpar configurações ao finalizar
    cmds2.controle.stop_pwm()
    gpio2.GPIO.cleanup()
