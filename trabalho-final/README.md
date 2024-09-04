# Trabalho final (2024-1) - controle de sistemas de carro

Trabalho final da disciplina de Fundamentos de Sistemas Embarcados (2024/1)

## Membros

- Guilherme Soares Rocha - 211039789
- João Manoel Barreto Neto - 211039519
- Miguel Frias - xxxx
- Yan Luca Viana de Araújo Fontenele - 211031889

## Objetivos

Este projeto visa criar um sistema embarcado para controle e monitoramento de algumas funções essenciais de um carro. Os principais objetivos são:

- Controlar e monitorar componentes eletrônicos do veículo, como faróis e iluminação interna.
- Coletar dados de sensores de luminosidade e outros sensores de controle.
- Utilizar um sistema distribuído com duas placas ESP32, cada uma controlando diferentes conjuntos de sensores e atuadores.
- Integrar o sistema com uma plataforma de monitoramento em tempo real (ThingsBoard), possibilitando a visualização dos dados e o controle remoto de componentes do carro.

## Descrição do Sistema
O sistema foi desenvolvido para funcionar com duas ESP32, que se comunicam via MQTT usando um broker. Ele utiliza sensores para monitorar condições do carro, como a luminosidade externa e interna, além de controlar os LEDs (indicando o estado dos faróis) e dispositivos PWM. A arquitetura do sistema foi organizada da seguinte forma:

- ESP32-Sensores: Responsável pela coleta de dados dos sensores de luminosidade e controle de dispositivos PWM. Ela envia os dados coletados ao ThingsBoard e interage com o dashboard.
- ESP32-GPIO: Controla os botões de entrada e LEDs de saída, responsável pelo controle das luzes do carro, como faróis e luz interna.

Ambas as ESP32 se comunicam com o ThingsBoard via MQTT, permitindo o monitoramento e controle centralizados no dashboard de um sistema chamado "Relâmpago Marquinhos", onde o usuário pode interagir com o sistema e visualizar os dados coletadas.

## Arquitetura do Projeto
A arquitetura do projeto está dividida entre hardware e software, como descrito abaixo:

#### Hardware:
- Duas ESP32. 
- Sensores: Incluindo um sensor de umidade e temperatura e um sensor de luminosidade.
- LEDs: Indicando o estado dos faróis e iluminação interna.
- Botões de entrada: Controlam manualmente as funções do veículo, como ligar/desligar os faróis.

#### Software:

- Cada ESP32 possui um arquivo main diferente, sendo responsável por diferentes conjuntos de funcionalidades.
- Comunicação MQTT: Utilizada para troca de informações entre os sensores e o ThingsBoard.
- Integração com o ThingsBoard: Dados de sensores e estado dos LEDs são monitorados e controlados via dashboard.

1. Pasta esp32_sensores
dht 11 temperatura; refletancia
Esta parte do código é responsável por:
- Configuração do MQTT: Conexão com broker, publicando e assinando tópicos para comunicação de dados.
- Sensores e atuadores:
    - Leitura de temperatura e umidade.
    - Leitura de luminosidade através de um ADC.
    - Controle de um LED via PWM, associado à "luz interna".
    - Controle de faróis, com comunicação via MQTT para refletir o estado dos faróis e luz interna.
- Processamento de métodos: Interpretação de comandos recebidos via MQTT e execução de ações, como ligar/desligar faróis e ajustar a intensidade da luz interna.
- Envio de dados via MQTT: Telemetria sobre temperatura, umidade, e luminosidade, e envio de atributos como estado dos faróis e da luz interna.
- Modo Sleep: Implementação de um botão para acordar o sistema e colocá-lo em modo light sleep quando não há atividade, visando economia de energia.

2. Pasta esp32_gpios
- Controle de farol, travas, vidros.
- Configuração de MQTT: Similar à primeira main, configurando um cliente MQTT para comunicação.
- Leitura de sensores e controle de atuadores: Configuração de um ADC para leitura de luminosidade e, provavelmente, controle de GPIOs ou LEDs.

## Passos para Execução
1. Clone o repositório do GitHub:

```bash
git clone https://github.com/FGA-FSE/trabalho-final-relampago-marquinhos.git
```

2. É recomendado que você instale a extensão ESP-IDF no Visual Studio Code. Ela auxilia no desenvolvimento e execução de projetos com ESP32.

3. Configuração do ambiente de execução:

- Abra o projeto no VSCode.
- Abra dois terminais, pois você precisará rodar duas ESP32 separadamente com arquivos 'main' diferentes.

4. Execução do projeto:

- No primeiro terminal, navegue até a pasta ESP32-Sensores:
```bash
cd esp32-sensores
```
- No segundo terminal, navegue até a pasta ESP32-GPIOS:
```bash
cd esp32-gpios
```
- Em ambos os terminais, execute o comando para fazer o build do projeto:
```bash
idf.py build
```
- Para dar o flash na placa, utilize o seguinte comando com a placa em modo boot:
```bash
idf.py flash monitor
```