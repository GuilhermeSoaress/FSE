import logging

def config_logging():
    # Criação do logger
    logger = logging.getLogger('')
    logger.setLevel(logging.DEBUG)

    # Formato do log
    formatter = logging.Formatter('[%(asctime)s] [%(levelname)s] - %(message)s')

    # Adiciona um manipulador de console
    console_handler = logging.StreamHandler()
    console_handler.setFormatter(formatter)
    logger.addHandler(console_handler)

    # Adiciona um manipulador de arquivo
    file_handler = logging.FileHandler('logfile.log')
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)

