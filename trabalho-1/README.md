
# Trabalho 1 (2024-1) - Controle de Estacionamentos

Trabalho da disciplina de Fundamentos de Sistemas Embarcados (2024/1)

## Membros

- Guilherme Soares Rocha - 211039789
- João Manoel Barreto Neto - 211039519

## Objetivos

Este trabalho visa criar um sistema distribuído para o controle e monitoramento de estacionamentos comerciais. Os principais objetivos são:

- Controlar a entrada e saída de veículos
- Monitorar a ocupação de vagas individualmente e do estacionamento como um todo
- Realizar a cobrança pelo tempo de permanência dos veículos

## Descrição do Sistema

O sistema foi desenvolvido para funcionar com placas Raspberry Pi, com a seguinte arquitetura:

- **Servidor Central**: Responsável pelo controle geral e interface com o usuário
- **Servidores Distribuídos**: Controlam os sensores e atuadores de cada andar do estacionamento

### Sensores e Atuadores

#### Térreo

- **Cancela de entrada**:
  - Botão para simular a chegada de um carro
  - Sensor de presença indicando a presença do carro aguardando
  - Cancela
  - Sensor de passagem indicando que a cancela pode ser fechada
- **Sinal de lotação**:
  - LED vermelho indicando quando o estacionamento está cheio
- **Cancela de saída**:
  - Sensor de presença indicando a presença de um carro aguardando a saída
  - Cancela
  - Sensor de passagem indicando que a cancela pode ser fechada
- **Vagas**:
  - 5 vagas regulares
  - 2 vaga para idosos
  - 1 vaga para portadores de necessidades especiais (PNE)

#### Andar 1

- **Cancela de entrada**:
  - Botão para simular a chegada de um carro
  - Sensor de presença indicando a presença do carro aguardando
  - Cancela
  - Sensor de passagem indicando que a cancela pode ser fechada
- **Cancela de saída**:
  - Sensor de presença indicando a presença de um carro aguardando a saída
  - Cancela
  - Sensor de passagem indicando que a cancela pode ser fechada
- **Vagas**:
  - 5 vagas regulares
  - 2 vaga para idosos
  - 1 vaga para portadores de necessidades especiais (PNE)

#### Andar 2

- **Cancela de entrada**:
  - Botão para simular a chegada de um carro
  - Sensor de presença indicando a presença do carro aguardando
  - Cancela
  - Sensor de passagem indicando que a cancela pode ser fechada
- **Cancela de saída**:
  - Sensor de presença indicando a presença de um carro aguardando a saída
  - Cancela
  - Sensor de passagem indicando que a cancela pode ser fechada
- **Vagas**:
  - 5 vagas regulares
  - 2 vaga para idosos
  - 1 vaga para portadores de necessidades especiais (PNE)

## Linguagens e Bibliotecas

Os códigos dos servidores, tanto central, quanto distribuídos foram feitos em python, com as bibliotecas adicionais:
- RPi.GPIO
- gpiozero
