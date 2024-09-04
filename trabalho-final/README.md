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

Ambas as ESP32 se comunicam com o ThingsBoard via MQTT, permitindo o monitoramento e controle centralizados no dashboard de um sistema chamado "Relâmpago Marquinhos", onde o usuário pode interagir com o sistema e visualizar as métricas coletadas.
