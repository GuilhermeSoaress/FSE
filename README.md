# 🏎️ Fundamentos de Sistemas Embarcados (FSE - 2024.1)

Este repositório consolida os três projetos práticos desenvolvidos na disciplina de Fundamentos de Sistemas Embarcados, ministrada pelo professor Renato Coral na Universidade de Brasília (UnB) durante o primeiro semestre de 2024.

## 📚 Sobre a Disciplina

A disciplina de **Fundamentos de Sistemas Embarcados** introduz os conceitos essenciais para o desenvolvimento de sistemas embarcados eficientes e confiáveis. O foco principal está na programação com sistemas operacionais de tempo real (RTOS), onde múltiplas tarefas precisam ser executadas de forma concorrente e coordenada.

## 🎯 Principais Conceitos Aprendidos

Ao longo dos projetos, os seguintes conceitos foram estudados e implementados:

-   **Sistema Operacional de Tempo Real (RTOS):** Utilização do FreeRTOS para gerenciamento de tarefas com controle temporal determinístico.
-   **Concorrência e Paralelismo:** Criação e gerenciamento de múltiplas Tasks (Threads) executando simultaneamente.
-   **Mecanismos de Sincronização:** Uso de **Semáforos** e **Mutex** para proteger recursos compartilhados e evitar condições de corrida (_race conditions_).
-   **Comunicação entre Tarefas:** Utilização de **Filas** para a troca de mensagens e dados entre tarefas de forma segura e organizada.
-   **Internet das Coisas (IoT):** Conectividade Wi-Fi e comunicação via protocolo **MQTT** para integração com sistemas distribuídos.
-   **Sistemas Distribuídos:** Coordenação de múltiplos dispositivos trabalhando em conjunto.

## 🚀 Projetos Desenvolvidos

Cada projeto abordou um desafio diferente, aplicando os conceitos da disciplina de forma incremental.

### 1. 🅿️ Sistema de Estacionamento Inteligente
-   **Objetivo:** Desenvolver um sistema distribuído para controle e monitoramento de estacionamentos comerciais.
-   **Tecnologias:** Python, Raspberry Pi, GPIO, Threading
-   **Conceitos Aplicados:** Utilizou **Tasks** concorrentes para simular a entrada e saída de veículos em múltiplos andares. **Semáforos** foram implementados para gerenciar o acesso ao número limitado de vagas, garantindo controle de acesso seguro aos recursos compartilhados.
-   **Arquitetura:** Sistema distribuído com servidor central e servidores por andar.

### 2. ⬆️ Controle de Elevador Embarcado
-   **Objetivo:** Implementar o controle completo de um sistema de elevador com ESP32.
-   **Tecnologias:** C/C++, ESP32, FreeRTOS, Sensores, Display OLED
-   **Conceitos Aplicados:** Utilizou **Filas** para gerenciar requisições de andares de forma ordenada e eficiente. **Mutex** protegeu o estado compartilhado do elevador, enquanto **Tasks** concorrentes controlaram interface, movimento e sensoriamento.
-   **Funcionalidades:** Controle de movimento, interface com botões, display informativo, logs detalhados.

### 3. ⚡ Relâmpago Marquinhos - Sistema Automotivo IoT
-   **Objetivo:** Criar um sistema embarcado para controle e monitoramento de funções automotivas conectado à internet.
-   **Tecnologias:** C/C++, ESP32, FreeRTOS, MQTT, WiFi, ThingsBoard, Sensores (DHT22, LDR)
-   **Conceitos Aplicados:** Sistema distribuído com duas ESP32 comunicando via **MQTT**. Múltiplas **Tasks** gerenciam sensoriamento, conectividade e controle de atuadores simultaneamente. **Semáforos** sincronizam operações críticas entre dispositivos.
-   **Funcionalidades:** Controle de faróis, monitoramento ambiental, dashboard web interativo, comunicação IoT em tempo real.

## 🛠️ Tecnologias Utilizadas

### Hardware
-   **ESP32:** Microcontrolador principal para projetos embarcados
-   **Raspberry Pi:** Utilizado no sistema distribuído de estacionamento
-   **Sensores:** DHT22 (temperatura/umidade), LDR (luminosidade), BMP280 (pressão)
-   **Display:** OLED para interface visual
-   **Componentes:** LEDs, botões, resistores, jumpers

### Software
-   **Sistema Operacional:** FreeRTOS (Real-Time Operating System)
-   **Linguagens:** C/C++ (ESP32), Python (Raspberry Pi)
-   **Protocolos:** MQTT, WiFi, I2C, UART
-   **Plataformas:** ThingsBoard (dashboard IoT), ESP-IDF
-   **Ferramentas:** Git, VS Code, Arduino IDE

## 📁 Estrutura do Repositório

```
📦 FSE
├── 📁 Trabalho 1 - Estacionamento/     # Sistema distribuído com Python
├── 📁 Trabalho 2 - Elevador/           # Controle embarcado com ESP32
└── 📁 Trabalho Final - Controles do carro/  # Sistema IoT automotivo
```

## 📖 Como Explorar

Cada pasta contém seu próprio README detalhado com:
- ✅ Instruções de configuração e execução
- 🔧 Diagramas de circuito e arquitetura
- 📊 Demonstrações e resultados
- 🎥 Vídeos explicativos (quando disponível)